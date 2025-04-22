#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <cstdbool>
#include "sensor_manager/sensor_manager.h"

// Forward declarations
struct MainWindow;

// Chart types
enum class ChartType {
    TEMPERATURE,
    PRESSURE,
    FLOW_RATE_1,
    FLOW_RATE_2,
    COUNT  // Total number of chart types
};

// Callback types
using SSRCallback = void (*)(int index, bool state);
using DimmerCallback = void (*)(uint32_t level);
using PIDSetpointCallback = void (*)(int index, float setpoint);
using PIDToggleCallback = void (*)(bool enabled);

// UI Manager class
class UIManager {
private:
    MainWindow* main_window;
    bool initialized;
    int ssr_count;
    const char** ssr_names;
    const bool* ssr_pid_enabled;
    
    // Current view state
    enum class ViewType { 
        CONTROL, 
        PLOTS 
    };
    ViewType current_view;
    
    // Callback handlers
    SSRCallback ssr_callback;
    DimmerCallback dimmer_callback;
    PIDSetpointCallback setpoint_callback;
    PIDToggleCallback pid_toggle_callback;
    
    // Private methods for event handling
    static void slintSSRToggled(void* context, int index, bool state);
    static void slintDimmerChanged(void* context, float level);
    static void slintTempSetpointChanged(void* context, float setpoint);
    static void slintPressureSetpointChanged(void* context, float setpoint);
    static void slintSSRSetpointChanged(void* context, int index, float setpoint);
    static void slintPIDToggled(void* context, bool enabled);
    static void slintToggleView(void* context);
    
public:
    UIManager();
    
    /**
     * @brief Initialize the UI manager
     */
    void init();
    
    /**
     * @brief Register UI callbacks
     *
     * @param ssr_cb Callback for SSR toggle events
     * @param dimmer_cb Callback for dimmer change events
     * @param setpoint_cb Callback for setpoint change events
     * @param pid_toggle_cb Callback for PID toggle events
     */
    void registerCallbacks(SSRCallback ssr_cb, 
                          DimmerCallback dimmer_cb,
                          PIDSetpointCallback setpoint_cb,
                          PIDToggleCallback pid_toggle_cb);
    
    /**
     * @brief Create the UI elements
     *
     * @param ssr_count Number of SSRs to display controls for
     * @param ssr_names Array of SSR names
     * @param ssr_pid_enabled Array indicating which SSRs have PID control enabled
     */
    void create(int ssr_count, const char** ssr_names, const bool* ssr_pid_enabled);
    
    /**
     * @brief Switch to main control view
     */
    void showControlView();
    
    /**
     * @brief Switch to plots view
     */
    void showPlotsView();
    
    /**
     * @brief Toggle between main view and plots view
     */
    void toggleView();
    
    /**
     * @brief Update the UI with sensor data
     *
     * @param data Sensor data to display
     */
    void updateSensorData(const sensor_data_t* data);
    
    /**
     * @brief Update charts with sensor history data
     *
     * @param history Sensor history data to plot
     */
    void updateCharts(const SensorHistory* history);
    
    /**
     * @brief Update UI elements to reflect PID output states
     *
     * @param pressure_output Current pressure PID output
     * @param ssr_states Array of current SSR states
     */
    void updatePIDOutputs(float pressure_output, const bool* ssr_states);
    
    /**
     * @brief Update UI elements to reflect PID setpoints
     *
     * @param pressure_setpoint Pressure PID setpoint
     * @param ssr_setpoints Array of SSR PID setpoints
     */
    void updatePIDSetpoints(float pressure_setpoint, const float* ssr_setpoints);
};

// Global instance
extern UIManager ui_manager;

// C compatibility functions
#ifdef __cplusplus
extern "C" {
#endif

void ui_manager_init(void);
void ui_manager_register_callbacks(SSRCallback ssr_cb,
                                 DimmerCallback dimmer_cb,
                                 PIDSetpointCallback setpoint_cb,
                                 PIDToggleCallback pid_toggle_cb);
void ui_create(int ssr_count, const char** ssr_names, const bool* ssr_pid_enabled);
void ui_show_control_view(void);
void ui_show_plots_view(void);
void ui_toggle_view(void);
void ui_update_sensor_data(const sensor_data_t* data);
void ui_update_charts(const SensorHistory* history);
void ui_update_pid_outputs(float pressure_output, const bool* ssr_states);
void ui_update_pid_setpoints(float pressure_setpoint, const float* ssr_setpoints);

#ifdef __cplusplus
}
#endif

#endif /* UI_MANAGER_H */ 