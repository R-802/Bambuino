#ifndef MAIN_UI_H
#define MAIN_UI_H

#include <stdbool.h>
#include <stddef.h>

// Define structs that match the Slint UI file
typedef struct {
    float temperature;
    float pressure;
    float flow_rate1;
    float flow_rate2;
} SensorData;

typedef struct {
    float* temperature_data;
    float* pressure_data;
    float* flow_rate1_data;
    float* flow_rate2_data;
} ChartData;

typedef struct {
    const char* name;
    bool state;
    bool has_pid;
    float setpoint;
} SSRData;

// Forward declaration of opaque MainWindow type
typedef struct MainWindow MainWindow;

// MainWindow constructor
MainWindow* main_window_new(void);

// Showing the window
void main_window_show(MainWindow* window);

// Property setters
void main_window_set_title(MainWindow* window, const char* title);
void main_window_set_sensor_data(MainWindow* window, const SensorData* data);
void main_window_set_chart_data(MainWindow* window, const ChartData* data);
void main_window_set_ssr_data(MainWindow* window, const SSRData* data, size_t count);
void main_window_set_pid_enabled(MainWindow* window, bool enabled);
void main_window_set_temp_setpoint(MainWindow* window, float setpoint);
void main_window_set_pressure_setpoint(MainWindow* window, float setpoint);
void main_window_set_dimmer_level(MainWindow* window, float level);
void main_window_set_show_control_view(MainWindow* window, bool show);

// Property getters
SSRData* main_window_get_ssr_data(MainWindow* window);
size_t main_window_get_ssr_data_size(MainWindow* window);

// Callback types
typedef void (*ssr_toggled_callback_t)(void* context, int index, bool state);
typedef void (*dimmer_changed_callback_t)(void* context, float level);
typedef void (*temp_setpoint_changed_callback_t)(void* context, float setpoint);
typedef void (*pressure_setpoint_changed_callback_t)(void* context, float setpoint);
typedef void (*ssr_setpoint_changed_callback_t)(void* context, int index, float setpoint);
typedef void (*pid_toggled_callback_t)(void* context, bool enabled);
typedef void (*toggle_view_callback_t)(void* context);

// Callback registration
void main_window_on_ssr_toggled(MainWindow* window, ssr_toggled_callback_t callback, void* context);
void main_window_on_dimmer_changed(MainWindow* window, dimmer_changed_callback_t callback, void* context);
void main_window_on_temp_setpoint_changed(MainWindow* window, temp_setpoint_changed_callback_t callback, void* context);
void main_window_on_pressure_setpoint_changed(MainWindow* window, pressure_setpoint_changed_callback_t callback, void* context);
void main_window_on_ssr_setpoint_changed(MainWindow* window, ssr_setpoint_changed_callback_t callback, void* context);
void main_window_on_pid_toggled(MainWindow* window, pid_toggled_callback_t callback, void* context);
void main_window_on_toggle_view(MainWindow* window, toggle_view_callback_t callback, void* context);

#endif /* MAIN_UI_H */ 