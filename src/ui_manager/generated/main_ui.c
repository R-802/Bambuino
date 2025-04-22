#include "main_ui.h"

#include <stdlib.h>
#include <string.h>

#include "esp_log.h"

static const char *TAG = "SLINT_UI";

// Define the MainWindow struct
struct MainWindow {
    const char *title;
    SensorData sensor_data;
    ChartData chart_data;
    SSRData *ssr_data;
    size_t ssr_data_size;
    bool pid_enabled;
    float temp_setpoint;
    float pressure_setpoint;
    float dimmer_level;
    bool show_control_view;

    // Callbacks
    ssr_toggled_callback_t ssr_toggled_cb;
    void *ssr_toggled_ctx;
    dimmer_changed_callback_t dimmer_changed_cb;
    void *dimmer_changed_ctx;
    temp_setpoint_changed_callback_t temp_setpoint_changed_cb;
    void *temp_setpoint_changed_ctx;
    pressure_setpoint_changed_callback_t pressure_setpoint_changed_cb;
    void *pressure_setpoint_changed_ctx;
    ssr_setpoint_changed_callback_t ssr_setpoint_changed_cb;
    void *ssr_setpoint_changed_ctx;
    pid_toggled_callback_t pid_toggled_cb;
    void *pid_toggled_ctx;
    toggle_view_callback_t toggle_view_cb;
    void *toggle_view_ctx;
};

// Create a new MainWindow
MainWindow *main_window_new(void)
{
    MainWindow *window = calloc(1, sizeof(MainWindow));
    if (!window) {
        ESP_LOGE(TAG, "Failed to allocate memory for MainWindow");
        return NULL;
    }

    // Initialize default values
    window->title             = "ESP32-S3 Control Panel";
    window->pid_enabled       = true;
    window->temp_setpoint     = 85.0f;
    window->pressure_setpoint = 30.0f;
    window->dimmer_level      = 0.0f;
    window->show_control_view = true;

    ESP_LOGI(TAG, "Created new MainWindow");
    return window;
}

// Show the window
void main_window_show(MainWindow *window)
{
    if (!window)
        return;
    ESP_LOGI(TAG, "Showing MainWindow with title: %s", window->title);
}

// Property setters
void main_window_set_title(MainWindow *window, const char *title)
{
    if (!window)
        return;
    window->title = title;
}

void main_window_set_sensor_data(MainWindow *window, const SensorData *data)
{
    if (!window || !data)
        return;
    memcpy(&window->sensor_data, data, sizeof(SensorData));
}

void main_window_set_chart_data(MainWindow *window, const ChartData *data)
{
    if (!window || !data)
        return;

    // We just store the pointers, data management is the responsibility of the caller
    window->chart_data = *data;
}

void main_window_set_ssr_data(MainWindow *window, const SSRData *data, size_t count)
{
    if (!window || !data || count == 0)
        return;

    // Free previous data if any
    if (window->ssr_data) {
        free(window->ssr_data);
    }

    // Allocate and copy new data
    window->ssr_data = calloc(count, sizeof(SSRData));
    if (!window->ssr_data) {
        ESP_LOGE(TAG, "Failed to allocate memory for SSR data");
        return;
    }

    memcpy(window->ssr_data, data, count * sizeof(SSRData));
    window->ssr_data_size = count;
}

void main_window_set_pid_enabled(MainWindow *window, bool enabled)
{
    if (!window)
        return;
    window->pid_enabled = enabled;
}

void main_window_set_temp_setpoint(MainWindow *window, float setpoint)
{
    if (!window)
        return;
    window->temp_setpoint = setpoint;
}

void main_window_set_pressure_setpoint(MainWindow *window, float setpoint)
{
    if (!window)
        return;
    window->pressure_setpoint = setpoint;
}

void main_window_set_dimmer_level(MainWindow *window, float level)
{
    if (!window)
        return;
    window->dimmer_level = level;
}

void main_window_set_show_control_view(MainWindow *window, bool show)
{
    if (!window)
        return;
    window->show_control_view = show;
}

// Property getters
SSRData *main_window_get_ssr_data(MainWindow *window)
{
    if (!window)
        return NULL;
    return window->ssr_data;
}

size_t main_window_get_ssr_data_size(MainWindow *window)
{
    if (!window)
        return 0;
    return window->ssr_data_size;
}

// Callback registration
void main_window_on_ssr_toggled(MainWindow *window, ssr_toggled_callback_t callback, void *context)
{
    if (!window)
        return;
    window->ssr_toggled_cb  = callback;
    window->ssr_toggled_ctx = context;
}

void main_window_on_dimmer_changed(MainWindow *window,
                                   dimmer_changed_callback_t callback,
                                   void *context)
{
    if (!window)
        return;
    window->dimmer_changed_cb  = callback;
    window->dimmer_changed_ctx = context;
}

void main_window_on_temp_setpoint_changed(MainWindow *window,
                                          temp_setpoint_changed_callback_t callback,
                                          void *context)
{
    if (!window)
        return;
    window->temp_setpoint_changed_cb  = callback;
    window->temp_setpoint_changed_ctx = context;
}

void main_window_on_pressure_setpoint_changed(MainWindow *window,
                                              pressure_setpoint_changed_callback_t callback,
                                              void *context)
{
    if (!window)
        return;
    window->pressure_setpoint_changed_cb  = callback;
    window->pressure_setpoint_changed_ctx = context;
}

void main_window_on_ssr_setpoint_changed(MainWindow *window,
                                         ssr_setpoint_changed_callback_t callback,
                                         void *context)
{
    if (!window)
        return;
    window->ssr_setpoint_changed_cb  = callback;
    window->ssr_setpoint_changed_ctx = context;
}

void main_window_on_pid_toggled(MainWindow *window, pid_toggled_callback_t callback, void *context)
{
    if (!window)
        return;
    window->pid_toggled_cb  = callback;
    window->pid_toggled_ctx = context;
}

void main_window_on_toggle_view(MainWindow *window, toggle_view_callback_t callback, void *context)
{
    if (!window)
        return;
    window->toggle_view_cb  = callback;
    window->toggle_view_ctx = context;
}