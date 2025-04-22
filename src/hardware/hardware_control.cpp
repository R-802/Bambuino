#include "hardware/hardware_control.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

static const char *TAG = "HW_CONTROL";

// Static member initialization
const uint8_t HardwareControl::SSR_PINS[SSR_COUNT] = {SSR_PIN_1, SSR_PIN_2, SSR_PIN_3, SSR_PIN_4};
const char *HardwareControl::SSR_NAMES[SSR_COUNT] = {"Heater", "Valve 1", "Valve 2", "Aux"};
uint32_t HardwareControl::flow_meter1_count = 0;
uint32_t HardwareControl::flow_meter2_count = 0;

// Global instance
HardwareControl hw;

// Flow meter pulse counting (ISR)
void IRAM_ATTR flow_meter1_isr(void *arg)
{
    HardwareControl::flow_meter1_count++;
}

void IRAM_ATTR flow_meter2_isr(void *arg)
{
    HardwareControl::flow_meter2_count++;
}

// HardwareControl implementation
HardwareControl::HardwareControl() 
    : initialized(false), dimmer_level(0), max6675_spi(nullptr)
{
    memset(ssr_states, 0, sizeof(ssr_states));
}

HardwareControl::~HardwareControl()
{
    // Properly clean up resources
    if (max6675_spi != nullptr) {
        spi_bus_remove_device(max6675_spi);
        spi_bus_free(SPI2_HOST);
    }
}

esp_err_t HardwareControl::init()
{
    ESP_LOGI(TAG, "Initializing hardware control module");

    // Initialize GPIO ISR service for flow meters - must be done before any other ISR initialization
    gpio_install_isr_service(0);

    // Initialize all hardware components
    initPressureSensor();
    initDimmer();
    initMax6675();
    initFlowMeters();
    initSSR();

    initialized = true;
    return ESP_OK;
}

void HardwareControl::initPressureSensor()
{
    ESP_LOGI(TAG, "Initializing pressure sensor ADC");
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_PRESSURE_CHANNEL, ADC_ATTEN_DB_11);
}

void HardwareControl::initDimmer()
{
    ESP_LOGI(TAG, "Initializing AC dimmer");

    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = DIMMER_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);
}

void HardwareControl::initMax6675()
{
    ESP_LOGI(TAG, "Initializing MAX6675 thermocouple");

    spi_bus_config_t bus_config = {
        .miso_io_num = MAX6675_MISO_PIN,
        .mosi_io_num = MAX6675_MOSI_PIN,
        .sclk_io_num = MAX6675_SCK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };

    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = 1000000,  // 1 MHz
        .mode = 0,                  // SPI mode 0
        .spics_io_num = MAX6675_CS_PIN,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_config, &max6675_spi));
}

void HardwareControl::initFlowMeters()
{
    ESP_LOGI(TAG, "Initializing flow meters");

    // Configure GPIO for flow meter pulse counting
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << FLOW_METER1_PIN) | (1ULL << FLOW_METER2_PIN),
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&io_conf);

    // Hook ISR handlers for flow meters
    gpio_isr_handler_add(FLOW_METER1_PIN, flow_meter1_isr, NULL);
    gpio_isr_handler_add(FLOW_METER2_PIN, flow_meter2_isr, NULL);
}

void HardwareControl::initSSR()
{
    ESP_LOGI(TAG, "Initializing Solid State Relays");

    // Initialize SSR pin bit mask
    uint64_t pin_mask = 0;
    for (int i = 0; i < SSR_COUNT; i++) {
        pin_mask |= (1ULL << SSR_PINS[i]);
    }

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = pin_mask,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&io_conf);

    // Initially turn off all SSRs
    for (int i = 0; i < SSR_COUNT; i++) {
        gpio_set_level(SSR_PINS[i], 0);
        ssr_states[i] = false;
    }
}

void HardwareControl::setDimmer(uint32_t level)
{
    ESP_LOGI(TAG, "Setting dimmer level to %u", level);
    dimmer_level = level;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, level);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void HardwareControl::setSSRState(int index, bool state)
{
    if (index >= 0 && index < SSR_COUNT) {
        ESP_LOGI(TAG, "Setting SSR %s to %s", SSR_NAMES[index], state ? "ON" : "OFF");
        gpio_set_level(SSR_PINS[index], state ? 1 : 0);
        ssr_states[index] = state;
    }
}

void HardwareControl::setSSRPWM(int index, float pwm)
{
    if (index >= 0 && index < SSR_COUNT) {
        ESP_LOGI(TAG, "Setting SSR %s PWM to %.2f", SSR_NAMES[index], pwm);
        
        // For now, just use simple on/off based on PWM threshold
        // In a real implementation, you would use hardware PWM or software PWM
        setSSRState(index, pwm > 0.0f);
    }
}

void HardwareControl::setAllSSR(bool state)
{
    ESP_LOGI(TAG, "Setting all SSRs to %s", state ? "ON" : "OFF");
    for (int i = 0; i < SSR_COUNT; i++) {
        setSSRState(i, state);
    }
}

// C compatibility wrappers
extern "C" {

esp_err_t hw_init(void) 
{
    return hw.init();
}

void hw_init_pressure_sensor(void) 
{
    hw.initPressureSensor();
}

void hw_init_dimmer(void) 
{
    hw.initDimmer();
}

void hw_init_max6675(spi_device_handle_t* spi_handle) 
{
    hw.initMax6675();
    if (spi_handle) {
        *spi_handle = hw.getMax6675Handle();
    }
}

void hw_init_flow_meters(void) 
{
    hw.initFlowMeters();
}

void hw_init_ssr(void) 
{
    hw.initSSR();
}

void hw_set_dimmer(uint32_t level) 
{
    hw.setDimmer(level);
}

void hw_set_ssr_state(int index, bool state) 
{
    hw.setSSRState(index, state);
}

void hw_set_ssr_pwm(int index, float pwm) 
{
    hw.setSSRPWM(index, pwm);
}

void hw_set_all_ssr(bool state) 
{
    hw.setAllSSR(state);
}

void hardware_control_init(void) 
{
    hw.init();
}

} // extern "C" 