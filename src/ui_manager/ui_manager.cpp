#include "ui_manager/ui_manager.h"

#include <cstring>

#include "display_driver.h"
#include "esp_log.h"

// Include Slint generated C bindings
#include "generated/main_ui.h"

static const char *TAG = "UI_MANAGER";

// Global instance
UIManager ui_manager;

// UIManager implementation
UIManager::UIManager() 
    : main_window(nullptr), initialized(false), ssr_count(0), 
      ssr_names(nullptr), ssr_pid_enabled(nullptr),
      current_view(ViewType::CONTROL),
      ssr_callback(nullptr), dimmer_callback(nullptr),
      setpoint_callback(nullptr), pid_toggle_callback(nullptr)
{
}

void UIManager::init()
{
    ESP_LOGI(TAG, "Initializing UI manager with Slint");

    // Initialize Slint UI (frontend)
    main_window = main_window_new();
    if (main_window == nullptr) {
        ESP_LOGE(TAG, "Failed to create Slint main window");
        return;
    }

    // Register UI callbacks to connect frontend events to backend
    main_window_on_ssr_toggled(main_window, slintSSRToggled, this);
    main_window_on_dimmer_changed(main_window, slintDimmerChanged, this);
    main_window_on_temp_setpoint_changed(main_window, slintTempSetpointChanged, this);
    main_window_on_pressure_setpoint_changed(main_window, slintPressureSetpointChanged, this);
    main_window_on_ssr_setpoint_changed(main_window, slintSSRSetpointChanged, this);
    main_window_on_pid_toggled(main_window, slintPIDToggled, this);
    main_window_on_toggle_view(main_window, slintToggleView, this);
    
    initialized = true;
}

void UIManager::registerCallbacks(SSRCallback ssr_cb, 
                               DimmerCallback dimmer_cb,
                               PIDSetpointCallback setpoint_cb,
                               PIDToggleCallback pid_toggle_cb)
{
    ssr_callback = ssr_cb;
    dimmer_callback = dimmer_cb;
    setpoint_callback = setpoint_cb;
    pid_toggle_callback = pid_toggle_cb;
}

void UIManager::create(int ssr_count, const char **ssr_names, const bool *ssr_pid_enabled)
{
    ESP_LOGI(TAG, "Creating UI elements with Slint");

    // Store configuration data
    this->ssr_count = ssr_count;
    this->ssr_names = ssr_names;
    this->ssr_pid_enabled = ssr_pid_enabled;

    // Default setpoints from application
    float default_temp_setpoint = 85.0f;
    float default_pressure_setpoint = 30.0f;
    float default_ssr_setpoints[16];
    for (int i = 0; i < 16; i++) {
        default_ssr_setpoints[i] = 85.0f;
    }

    // Create data structures for the UI
    SSRData ssr_data[ssr_count];
    for (int i = 0; i < ssr_count; i++) {
        ssr_data[i].name = ssr_names[i];
        ssr_data[i].state = false;
        ssr_data[i].has_pid = ssr_pid_enabled[i];
        ssr_data[i].setpoint = default_ssr_setpoints[i];
    }

    // Initialize the UI frontend with default values
    display_slint_acquire();

    main_window_set_title(main_window, "ESP32-S3 Control Panel");
    main_window_set_pid_enabled(main_window, true);
    main_window_set_temp_setpoint(main_window, default_temp_setpoint);
    main_window_set_pressure_setpoint(main_window, default_pressure_setpoint);
    main_window_set_ssr_data(main_window, ssr_data, ssr_count);
    main_window_set_dimmer_level(main_window, 0.0f);
    main_window_set_show_control_view(main_window, true);

    // Initialize sensor data to zero
    SensorData sensor_data = {
        .temperature = 0.0f, 
        .pressure = 0.0f, 
        .flow_rate1 = 0.0f, 
        .flow_rate2 = 0.0f
    };
    main_window_set_sensor_data(main_window, &sensor_data);

    // Initialize chart data to empty arrays
    ChartData chart_data = {
        .temperature_data = nullptr,
        .pressure_data = nullptr,
        .flow_rate1_data = nullptr,
        .flow_rate2_data = nullptr
    };
    main_window_set_chart_data(main_window, &chart_data);

    display_slint_release();

    // Show UI
    main_window_show(main_window);
}

void UIManager::showControlView()
{
    ESP_LOGI(TAG, "Switching to control view");
    
    display_slint_acquire();
    main_window_set_show_control_view(main_window, true);
    display_slint_release();
    
    current_view = ViewType::CONTROL;
}

void UIManager::showPlotsView()
{
    ESP_LOGI(TAG, "Switching to plots view");
    
    display_slint_acquire();
    main_window_set_show_control_view(main_window, false);
    display_slint_release();
    
    current_view = ViewType::PLOTS;
}

void UIManager::toggleView()
{
    if (current_view == ViewType::CONTROL) {
        showPlotsView();
    } else {
        showControlView();
    }
}

void UIManager::updateSensorData(const sensor_data_t *data)
{
    if (!initialized || !data) {
        return;
    }

    display_slint_acquire();
    
    // Update sensor data in the UI
    SensorData ui_sensor_data = {
        .temperature = data->temperature,
        .pressure = data->pressure,
        .flow_rate1 = data->flow_rate1,
        .flow_rate2 = data->flow_rate2
    };
    main_window_set_sensor_data(main_window, &ui_sensor_data);
    
    // Update SSR states
    SSRData *ssr_data = main_window_get_ssr_data(main_window);
    if (ssr_data != nullptr && ssr_count > 0) {
        for (int i = 0; i < ssr_count; i++) {
            ssr_data[i].state = data->ssr_states[i];
        }
        main_window_set_ssr_data(main_window, ssr_data, ssr_count);
    }
    
    // Update dimmer level
    float dimmer_percentage = (float)data->dimmer_level / 1023.0f;
    main_window_set_dimmer_level(main_window, dimmer_percentage);
    
    display_slint_release();
}

void UIManager::updateCharts(const SensorHistory *history)
{
    if (!initialized || !history) {
        return;
    }

    display_slint_acquire();
    
    // Convert circular buffer to linear array for charts
    // This depends on how your charting library works
    // Here's a simplified example:
    float temp_data[SENSOR_HISTORY_LENGTH];
    float press_data[SENSOR_HISTORY_LENGTH];
    float flow1_data[SENSOR_HISTORY_LENGTH];
    float flow2_data[SENSOR_HISTORY_LENGTH];
    
    for (int i = 0; i < SENSOR_HISTORY_LENGTH; i++) {
        int idx = (history->write_index + i) % SENSOR_HISTORY_LENGTH;
        temp_data[i] = history->temperature[idx];
        press_data[i] = history->pressure[idx];
        flow1_data[i] = history->flow_rate1[idx];
        flow2_data[i] = history->flow_rate2[idx];
    }
    
    // Update chart data
    ChartData chart_data = {
        .temperature_data = temp_data,
        .pressure_data = press_data,
        .flow_rate1_data = flow1_data,
        .flow_rate2_data = flow2_data
    };
    main_window_set_chart_data(main_window, &chart_data);
    
    display_slint_release();
}

void UIManager::updatePIDOutputs(float pressure_output, const bool *ssr_states)
{
    if (!initialized) {
        return;
    }

    display_slint_acquire();
    
    // Update dimmer level (for pressure PID)
    float dimmer_percentage = pressure_output / 1023.0f;
    main_window_set_dimmer_level(main_window, dimmer_percentage);
    
    // Update SSR states
    SSRData *ssr_data = main_window_get_ssr_data(main_window);
    if (ssr_data != nullptr && ssr_count > 0) {
        for (int i = 0; i < ssr_count; i++) {
            ssr_data[i].state = ssr_states[i];
        }
        main_window_set_ssr_data(main_window, ssr_data, ssr_count);
    }
    
    display_slint_release();
}

void UIManager::updatePIDSetpoints(float pressure_setpoint, const float *ssr_setpoints)
{
    if (!initialized) {
        return;
    }

    display_slint_acquire();
    
    // Update pressure setpoint
    main_window_set_pressure_setpoint(main_window, pressure_setpoint);
    
    // Update SSR setpoints
    SSRData *ssr_data = main_window_get_ssr_data(main_window);
    if (ssr_data != nullptr && ssr_count > 0) {
        for (int i = 0; i < ssr_count; i++) {
            if (ssr_pid_enabled[i]) {
                ssr_data[i].setpoint = ssr_setpoints[i];
            }
        }
        main_window_set_ssr_data(main_window, ssr_data, ssr_count);
    }
    
    display_slint_release();
}

// Static event handlers
void UIManager::slintSSRToggled(void *context, int index, bool state)
{
    UIManager *ui = static_cast<UIManager*>(context);
    if (ui && ui->ssr_callback) {
        ui->ssr_callback(index, state);
    }
}

void UIManager::slintDimmerChanged(void *context, float level)
{
    UIManager *ui = static_cast<UIManager*>(context);
    if (ui && ui->dimmer_callback) {
        ui->dimmer_callback((uint32_t)(level * 1023.0f));
    }
}

void UIManager::slintTempSetpointChanged(void *context, float setpoint)
{
    UIManager *ui = static_cast<UIManager*>(context);
    
    // Update UI state
    display_slint_acquire();
    main_window_set_temp_setpoint(ui->main_window, setpoint);
    display_slint_release();

    // Forward to application logic (backend)
    if (ui && ui->setpoint_callback) {
        ui->setpoint_callback(0, setpoint);
    }

    ESP_LOGI(TAG, "Temperature setpoint changed to %.1f °C", setpoint);
}

void UIManager::slintPressureSetpointChanged(void *context, float setpoint)
{
    UIManager *ui = static_cast<UIManager*>(context);
    
    // Update UI state
    display_slint_acquire();
    main_window_set_pressure_setpoint(ui->main_window, setpoint);
    display_slint_release();

    // Forward to application logic (backend)
    if (ui && ui->setpoint_callback) {
        ui->setpoint_callback(-1, setpoint);
    }

    ESP_LOGI(TAG, "Pressure setpoint changed to %.1f PSI", setpoint);
}

void UIManager::slintSSRSetpointChanged(void *context, int index, float setpoint)
{
    UIManager *ui = static_cast<UIManager*>(context);
    
    // Update the UI state
    if (ui && index >= 0 && index < ui->ssr_count && ui->ssr_pid_enabled[index]) {
        display_slint_acquire();
        SSRData *ssr_data = main_window_get_ssr_data(ui->main_window);
        if (ssr_data != nullptr && index < main_window_get_ssr_data_size(ui->main_window)) {
            ssr_data[index].setpoint = setpoint;
            main_window_set_ssr_data(
                ui->main_window, ssr_data, main_window_get_ssr_data_size(ui->main_window));
        }
        display_slint_release();

        // Forward to application logic (backend)
        if (ui->setpoint_callback) {
            ui->setpoint_callback(index, setpoint);
        }

        ESP_LOGI(TAG, "SSR%d setpoint changed to %.1f °C", index + 1, setpoint);
    }
}

void UIManager::slintPIDToggled(void *context, bool enabled)
{
    UIManager *ui = static_cast<UIManager*>(context);
    
    // Update UI state
    display_slint_acquire();
    main_window_set_pid_enabled(ui->main_window, enabled);
    display_slint_release();

    // Forward to application logic (backend)
    if (ui && ui->pid_toggle_callback) {
        ui->pid_toggle_callback(enabled);
    }

    ESP_LOGI(TAG, "PID controllers %s", enabled ? "enabled" : "disabled and reset");
}

void UIManager::slintToggleView(void *context)
{
    UIManager *ui = static_cast<UIManager*>(context);
    if (ui) {
        ui->toggleView();
    }
}

// C compatibility wrappers
extern "C" {

void ui_manager_init(void)
{
    ui_manager.init();
}

void ui_manager_register_callbacks(SSRCallback ssr_cb,
                                DimmerCallback dimmer_cb,
                                PIDSetpointCallback setpoint_cb,
                                PIDToggleCallback pid_toggle_cb)
{
    ui_manager.registerCallbacks(ssr_cb, dimmer_cb, setpoint_cb, pid_toggle_cb);
}

void ui_create(int ssr_count, const char **ssr_names, const bool *ssr_pid_enabled)
{
    ui_manager.create(ssr_count, ssr_names, ssr_pid_enabled);
}

void ui_show_control_view(void)
{
    ui_manager.showControlView();
}

void ui_show_plots_view(void)
{
    ui_manager.showPlotsView();
}

void ui_toggle_view(void)
{
    ui_manager.toggleView();
}

void ui_update_sensor_data(const sensor_data_t *data)
{
    ui_manager.updateSensorData(data);
}

void ui_update_charts(const SensorHistory *history)
{
    ui_manager.updateCharts(history);
}

void ui_update_pid_outputs(float pressure_output, const bool *ssr_states)
{
    ui_manager.updatePIDOutputs(pressure_output, ssr_states);
}

void ui_update_pid_setpoints(float pressure_setpoint, const float *ssr_setpoints)
{
    ui_manager.updatePIDSetpoints(pressure_setpoint, ssr_setpoints);
}

} // extern "C" 