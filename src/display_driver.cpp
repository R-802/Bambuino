#include "display_driver.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "esp_psram.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

// -------------------------------------------------------------
// DISPLAY HARDWARE LAYER
// -------------------------------------------------------------

#include "tft_config.h"

static const char *TAG = "display";

/* Display size configuration */
#define DISPLAY_WIDTH TFT_WIDTH
#define DISPLAY_HEIGHT TFT_HEIGHT

/* SPI bus configuration */
#define LCD_HOST SPI2_HOST
#define TOUCH_HOST SPI3_HOST

/* Display hardware globals */
static esp_lcd_panel_io_handle_t io_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;
static SemaphoreHandle_t slint_mutex       = NULL;

/* Touch related variables */
static spi_device_handle_t touch_spi                         = NULL;
static bool (*touch_event_handler)(uint16_t *x, uint16_t *y) = NULL;

/* Function prototypes */
static bool touch_get_xy(uint16_t *x, uint16_t *y);
static void slint_task(void *pvParameter);

// -------------------------------------------------------------
// SLINT UI RENDERER LAYER
// -------------------------------------------------------------

// Define a concrete struct for the renderer to avoid incomplete type errors
struct slint_renderer_t {
    void *panel_handle;  // Store the panel for rendering
    int width;
    int height;
    void *framebuffer;  // We would have a framebuffer in a real implementation
};

// Slint renderer implementation
static struct slint_renderer_t *slint_renderer = NULL;

// Global instance
DisplayDriver display;

// C++ implementation of DisplayDriver
DisplayDriver::DisplayDriver() : panel_handle(nullptr), width(0), height(0), initialized(false)
{
}

esp_err_t DisplayDriver::init()
{
    ESP_LOGI(TAG, "Initializing SPI OLED display for Slint");

    // Check for and initialize PSRAM
    if (esp_psram_is_initialized()) {
        ESP_LOGI(TAG, "PSRAM is initialized, size: %d bytes", esp_psram_get_size());
    }
    else {
        ESP_LOGW(TAG, "PSRAM not initialized or not available");
    }

    // STEP 1: Initialize SPI bus
    spi_bus_config_t buscfg = {
        .miso_io_num   = TFT_MISO,
        .mosi_io_num   = TFT_MOSI,
        .sclk_io_num   = TFT_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        /* For a 5-inch 800x480 display, we need a larger transfer size */
        .max_transfer_sz =
            DISPLAY_WIDTH * 200 * sizeof(uint16_t),  // Increased for full-frame rendering
    };

    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // STEP 2: Initialize LCD panel
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num       = TFT_DC,
        .cs_gpio_num       = TFT_CS,
        .pclk_hz           = SPI_FREQUENCY,
        .lcd_cmd_bits      = 8,
        .lcd_param_bits    = 8,
        .spi_mode          = 0,
        .trans_queue_depth = 20,  // Increased queue depth for larger display
    };

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = TFT_RST,
        .rgb_endian     = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_spi(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    // STEP 3: Configure panel settings
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, INVERT_COLORS));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, false));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, TFT_ROTATION == 1 || TFT_ROTATION == 3));

    uint8_t contrast_cmd[] = {0x81, 0xCF};  // Example command for SSD1306
    esp_lcd_panel_io_tx_param(io_handle, 0x00, contrast_cmd, 2);

    // STEP 4: Configure backlight
    gpio_config_t pwr_gpio_config = {.mode = GPIO_MODE_OUTPUT, .pin_bit_mask = 1ULL << TFT_BL};
    ESP_ERROR_CHECK(gpio_config(&pwr_gpio_config));
    gpio_set_level(TFT_BL, 1);

    // STEP 5: Initialize touch controller
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = TOUCH_SPI_FREQUENCY,
        .mode           = 0,
        .spics_io_num   = TOUCH_CS,
        .queue_size     = 7,
    };

    spi_bus_add_device(TOUCH_HOST, &devcfg, &touch_spi);

    uint8_t touch_init_cmd             = 0x38;
    spi_transaction_t touch_init_trans = {
        .length    = 8,
        .tx_buffer = &touch_init_cmd,
    };
    spi_device_polling_transmit(touch_spi, &touch_init_trans);

    // STEP 6: Initialize Slint renderer
    slint_init_with_custom_renderer(DISPLAY_WIDTH, DISPLAY_HEIGHT, panel_handle);

    // STEP 7: Create mutex for UI access
    slint_mutex = xSemaphoreCreateMutex();
    if (!slint_mutex) {
        ESP_LOGE(TAG, "Failed to create Slint mutex");
        return ESP_FAIL;
    }

    // STEP 8: Start Slint task
    xTaskCreate(slint_task, "slint", 8192, NULL, 5, NULL);

    this->initialized  = true;
    this->panel_handle = panel_handle;
    this->width        = DISPLAY_WIDTH;
    this->height       = DISPLAY_HEIGHT;

    // Register touch handler
    slint_register_touch_event_handler(touch_get_xy);

    ESP_LOGI(TAG, "Display initialization complete");
    return ESP_OK;
}

void DisplayDriver::acquireUIMutex()
{
    if (slint_mutex) {
        xSemaphoreTake(slint_mutex, portMAX_DELAY);
    }
}

void DisplayDriver::releaseUIMutex()
{
    if (slint_mutex) {
        xSemaphoreGive(slint_mutex);
    }
}

void DisplayDriver::tick()
{
    slint_tick();
}

// Implementation of touch_get_xy
static bool touch_get_xy(uint16_t *x, uint16_t *y)
{
    if (!touch_spi) {
        return false;
    }

    // Read touch controller registers to determine if screen is touched
    // This is a simplified example and would need to be adapted for your specific touch controller
    uint8_t cmd     = 0xD0;  // Example command to read touch status
    uint8_t data[4] = {0};   // Buffer for touch data

    spi_transaction_t trans = {
        .length    = 8,
        .tx_buffer = &cmd,
        .rx_buffer = data,
    };

    spi_device_polling_transmit(touch_spi, &trans);

    // Check if touch is active (implementation depends on touch controller)
    bool touched = (data[0] & 0x80) != 0;
    if (touched && x && y) {
        // Read X/Y coordinates (implementation depends on touch controller)
        // This is a simplified example
        *x = ((data[1] << 8) | data[2]) & 0x0FFF;
        *y = ((data[2] << 8) | data[3]) & 0x0FFF;

        // Normalize coordinates to display
        *x = (*x * DISPLAY_WIDTH) / 4096;
        *y = (*y * DISPLAY_HEIGHT) / 4096;
    }

    return touched;
}

// Implementation of Slint rendering task
static void slint_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Slint rendering task started");

    while (1) {
        // Acquire mutex to safely access Slint UI
        xSemaphoreTake(slint_mutex, portMAX_DELAY);

        // Process a frame in Slint
        slint_tick();

        // Release mutex
        xSemaphoreGive(slint_mutex);

        // Frame rate control - target 60 FPS
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}

// Implementation of Slint initialization
void slint_init_with_custom_renderer(int width, int height, void *panel)
{
    ESP_LOGI(TAG, "Initializing Slint renderer with width=%d, height=%d", width, height);

    // Allocate and initialize renderer structure
    slint_renderer = (struct slint_renderer_t *)malloc(sizeof(struct slint_renderer_t));
    if (!slint_renderer) {
        ESP_LOGE(TAG, "Failed to allocate memory for Slint renderer");
        return;
    }

    // Store parameters
    slint_renderer->panel_handle = panel;
    slint_renderer->width        = width;
    slint_renderer->height       = height;
    slint_renderer->framebuffer = NULL;  // We would allocate a framebuffer in a real implementation
}

// Register touch event handler
void slint_register_touch_event_handler(bool (*handler)(uint16_t *x, uint16_t *y))
{
    touch_event_handler = handler;
}

// Process Slint events and update UI
void slint_tick(void)
{
    // Check for touch events and process them
    uint16_t x, y;
    bool touched = touch_get_xy(&x, &y);

    // Process touch events if handler is registered
    if (touched && touch_event_handler) {
        touch_event_handler(&x, &y);
    }

    // Here you would normally call Slint to render a frame
    // For now, we'll just simulate a rendering operation
    if (panel_handle) {
        // In a full implementation, this would render the Slint UI to the display
        // Currently just a placeholder for the rendering loop
        static uint32_t frame_counter = 0;
        frame_counter++;

        if (frame_counter % 100 == 0) {
            ESP_LOGD(TAG, "Rendered frame %u", frame_counter);
        }
    }
}

// C compatibility functions
extern "C" {

esp_err_t display_init(void)
{
    return display.init();
}

void display_slint_acquire(void)
{
    display.acquireUIMutex();
}

void display_slint_release(void)
{
    display.releaseUIMutex();
}

}  // extern "C"