#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#ifdef CONFIG_BT_ENABLED
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "esp_bt.h"
#include "esp_bt_device.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gap_bt_api.h"
#endif

#include "display_driver.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "pid_controller.h"

// Include our new modules
#include "hardware/hardware_control.h"
#include "sensor_manager/sensor_manager.h"
#include "ui_manager/ui_manager.h"

// Tag for logging
static const char *TAG = "MAIN";

// WiFi credentials
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// PID control parameters - Pressure
#define PRESSURE_KP 2.0f                 // Proportional gain
#define PRESSURE_KI 0.5f                 // Integral gain
#define PRESSURE_KD 0.1f                 // Derivative gain
#define PRESSURE_SAMPLE_TIME 100         // PID update interval (ms)
#define PRESSURE_MIN_OUTPUT 0.0f         // Minimum output (0%)
#define PRESSURE_MAX_OUTPUT 1023.0f      // Maximum output (100% dimmer)
#define PRESSURE_DEFAULT_SETPOINT 30.0f  // Default pressure setpoint (PSI)

// SSR PID control parameters - Only the first SSR (heater) uses PID by default
#define SSR_PID_ENABLED {true, false, false, false}            // Which SSRs use PID control
#define SSR_PID_KP {5.0f, 5.0f, 5.0f, 5.0f}                    // Proportional gains
#define SSR_PID_KI {0.1f, 0.1f, 0.1f, 0.1f}                    // Integral gains
#define SSR_PID_KD {1.0f, 1.0f, 1.0f, 1.0f}                    // Derivative gains
#define SSR_PID_SAMPLE_TIME {1000, 1000, 1000, 1000}           // PID update intervals (ms)
#define SSR_PID_DEFAULT_SETPOINT {85.0f, 85.0f, 85.0f, 85.0f}  // Default setpoints

// Global variables
static EventGroupHandle_t wifi_event_group;
static spi_device_handle_t max6675_spi;

// PID controllers
static PIDController pressure_pid(PRESSURE_KP,
                                  PRESSURE_KI,
                                  PRESSURE_KD,
                                  PRESSURE_MIN_OUTPUT,
                                  PRESSURE_MAX_OUTPUT,
                                  PRESSURE_SAMPLE_TIME);

// Array of PID controllers
static PIDController ssr_pid[SSR_COUNT] = {
    PIDController(SSR_PID_KP[0], SSR_PID_KI[0], SSR_PID_KD[0], 0.0f, 1.0f, SSR_PID_SAMPLE_TIME[0]),
    PIDController(SSR_PID_KP[1], SSR_PID_KI[1], SSR_PID_KD[1], 0.0f, 1.0f, SSR_PID_SAMPLE_TIME[1]),
    PIDController(SSR_PID_KP[2], SSR_PID_KI[2], SSR_PID_KD[2], 0.0f, 1.0f, SSR_PID_SAMPLE_TIME[2]),
    PIDController(SSR_PID_KP[3], SSR_PID_KI[3], SSR_PID_KD[3], 0.0f, 1.0f, SSR_PID_SAMPLE_TIME[3])};

static bool ssr_pid_enabled[SSR_COUNT] = SSR_PID_ENABLED;  // Which SSRs use PID
static bool pid_enabled                = true;

// Sensor data
static sensor_data_t sensor_data = {0};

// Event group bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

// WiFi event handler
static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying WiFi connection...");
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// Initialize WiFi in station mode
static void init_wifi(void)
{
    ESP_LOGI(TAG, "Initializing WiFi");

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta =
            {
                .ssid               = WIFI_SSID,
                .password           = WIFI_PASSWORD,
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                .pmf_cfg            = {.capable = true, .required = false},
            },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi initialization complete");
}

#ifdef CONFIG_BT_ENABLED
// Bluetooth event callback
static void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_BT_GAP_AUTH_CMPL_EVT:
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG,
                         "Bluetooth authentication completed successfully: %s",
                         param->auth_cmpl.device_name);
            }
            else {
                ESP_LOGE(TAG, "Bluetooth authentication failed: %d", param->auth_cmpl.stat);
            }
            break;

        default:
            break;
    }
}
#endif

// Initialize Bluetooth
static void init_bluetooth(void)
{
#ifdef CONFIG_BT_ENABLED
    ESP_LOGI(TAG, "Initializing Bluetooth");

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_bt_gap_register_callback(bt_app_gap_cb));

    /* Set discoverable and connectable mode */
    ESP_ERROR_CHECK(esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE));

    /* Set the device name */
    ESP_ERROR_CHECK(esp_bt_dev_set_device_name("ESP32-S3-CONTROLLER"));

    ESP_LOGI(TAG, "Bluetooth initialization complete");
#else
    ESP_LOGW(TAG, "Bluetooth not enabled in sdkconfig");
#endif
}

// Initialize PID controllers
static void init_pid_controllers(void)
{
    ESP_LOGI(TAG, "Initializing PID controllers");

    // Set pressure PID setpoint
    pressure_pid.setSetpoint(PRESSURE_DEFAULT_SETPOINT);

    // Initialize SSR PID controllers setpoints
    float setpoints[SSR_COUNT] = SSR_PID_DEFAULT_SETPOINT;

    for (int i = 0; i < SSR_COUNT; i++) {
        ssr_pid[i].setSetpoint(setpoints[i]);
        ESP_LOGI(TAG, "SSR%d PID initialized, setpoint=%.1f", i + 1, setpoints[i]);
    }
}

// UI Callback handlers
static void on_ssr_toggled(int index, bool state)
{
    if (!pid_enabled) {
        hw_set_ssr_state(index, state);
        sensor_data.ssr_states[index] = state;
    }
}

static void on_dimmer_changed(uint32_t level)
{
    if (!pid_enabled) {
        hw_set_dimmer(level);
        sensor_data.dimmer_level = level;
    }
}

static void on_pid_setpoint_changed(int index, float setpoint)
{
    if (index < 0) {
        // Special case: pressure setpoint
        pressure_pid.setSetpoint(setpoint);
    }
    else if (index < SSR_COUNT && ssr_pid_enabled[index]) {
        // SSR setpoint
        ssr_pid[index].setSetpoint(setpoint);
    }
}

static void on_pid_toggled(bool enabled)
{
    pid_enabled = enabled;

    // If PID is disabled, reset controllers to avoid integration windup
    if (!pid_enabled) {
        pressure_pid.reset();
        for (int i = 0; i < SSR_COUNT; i++) {
            if (ssr_pid_enabled[i]) {
                ssr_pid[i].reset();
            }
        }
    }
}

// Sensor reading task
static void sensor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Sensor task starting");

    // Initialize sensor data structure
    memset(&sensor_data, 0, sizeof(sensor_data_t));

    // Loop forever reading sensors
    while (1) {
        // Read sensors
        sensor_manager_read_all(&sensor_data);

        // Update UI with sensor data
        display_slint_acquire();
        ui_manager_update_sensor_data(&sensor_data);
        display_slint_release();

        // Run PID controllers if enabled
        if (pid_enabled) {
            uint32_t current_time = esp_timer_get_time() / 1000;  // Convert to ms

            // Update pressure PID
            float pressure_output = pressure_pid.compute(sensor_data.pressure, current_time);
            hw_set_dimmer((uint32_t)pressure_output);
            sensor_data.dimmer_level = (uint32_t)pressure_output;

            // Update SSR PIDs
            for (int i = 0; i < SSR_COUNT; i++) {
                if (ssr_pid_enabled[i]) {
                    // Select correct input value based on SSR purpose
                    // Assuming SSR0 = heater, others can have different inputs
                    float input_value = 0.0f;
                    switch (i) {
                        case 0:  // Heater
                            input_value = sensor_data.temperature;
                            break;
                        case 1:  // Pump
                            input_value = sensor_data.flow_rate;
                            break;
                        default:
                            input_value = 0.0f;  // Default
                    }

                    // Compute PID output
                    float output = ssr_pid[i].compute(input_value, current_time);

                    // Apply PWM value (0.0-1.0) to SSR - hardware_control API handles PWM
                    hw_set_ssr_pwm(i, output);

                    // Update state for UI
                    sensor_data.ssr_states[i] = (output > 0.0f);
                    sensor_data.ssr_pwm[i]    = output;
                }
            }
        }

        // Delay for sensor task
        vTaskDelay(pdMS_TO_TICKS(50));  // 20Hz sensor update rate
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-S3 Pump Controller starting");

    // Initialize NVS (needed for WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize components
    ESP_ERROR_CHECK(display_init());         // Initialize display and UI
    ESP_ERROR_CHECK(hw_init());              // Initialize hardware control
    ESP_ERROR_CHECK(sensor_manager_init());  // Initialize sensor manager

    // Initialize communication
    init_wifi();       // Initialize WiFi
    init_bluetooth();  // Initialize Bluetooth

    // Initialize UI manager with callback handlers
    ui_manager_init();
    ui_manager_register_callbacks(
        on_ssr_toggled, on_dimmer_changed, on_pid_setpoint_changed, on_pid_toggled);

    // Initialize PID controllers
    init_pid_controllers();

    // Create sensor reading task (highest priority)
    xTaskCreate(sensor_task, "sensor", 4096, NULL, configMAX_PRIORITIES - 1, NULL);

    ESP_LOGI(TAG, "Initialization complete");
}
}