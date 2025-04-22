#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <cstdbool>
#include "driver/spi_master.h"

// Number of data points to store for graphs
#define SENSOR_HISTORY_LENGTH 120 // 2 minutes of data at 1Hz

// Sensor data structure to hold all readings
typedef struct {
    float temperature;
    float pressure;
    float flow_rate1;
    float flow_rate2;
    bool ssr_states[4];
    uint32_t dimmer_level;
    float ssr_pwm[4];  // PWM values for each SSR (0.0-1.0)
} sensor_data_t;

// Historical sensor data for plotting
class SensorHistory {
public:
    float temperature[SENSOR_HISTORY_LENGTH];
    float pressure[SENSOR_HISTORY_LENGTH];
    float flow_rate1[SENSOR_HISTORY_LENGTH];
    float flow_rate2[SENSOR_HISTORY_LENGTH];
    int write_index;
    uint32_t last_update_time;
    uint32_t update_interval_ms; // Interval between data points

    SensorHistory();
    void reset();
};

// Sensor manager class
class SensorManager {
private:
    spi_device_handle_t max6675_spi;
    bool initialized;
    SensorHistory sensor_history;
    
public:
    SensorManager();
    
    /**
     * @brief Initialize the sensor manager
     *
     * @param max6675_spi SPI device handle for MAX6675 temperature sensor
     * @return ESP_OK on success
     */
    esp_err_t init(spi_device_handle_t max6675_spi);
    
    /**
     * @brief Read temperature from MAX6675 thermocouple
     *
     * @return Temperature in degrees Celsius, or -1.0 on error
     */
    float readTemperature();
    
    /**
     * @brief Read pressure from ADC
     *
     * @return Pressure in PSI
     */
    float readPressure();
    
    /**
     * @brief Calculate flow rates from pulse counts
     *
     * @param flow1 Pointer to store flow rate 1 (mL/min)
     * @param flow2 Pointer to store flow rate 2 (mL/min)
     */
    void calculateFlowRates(float* flow1, float* flow2);
    
    /**
     * @brief Read all sensor values and update the sensor data structure
     *
     * @param data Pointer to sensor data structure
     */
    void readAll(sensor_data_t* data);
    
    /**
     * @brief Update sensor history with current sensor data
     *
     * @param data Current sensor data
     * @param current_time Current time in milliseconds
     */
    void updateHistory(const sensor_data_t* data, uint32_t current_time);
    
    /**
     * @brief Get sensor history data for plotting
     *
     * @return Pointer to sensor history structure
     */
    const SensorHistory* getHistory() const;
    
    /**
     * @brief Set the update interval for sensor history
     *
     * @param interval_ms Interval in milliseconds
     */
    void setHistoryInterval(uint32_t interval_ms);
    
    /**
     * @brief Get the update interval for sensor history
     *
     * @return Update interval in milliseconds
     */
    uint32_t getHistoryInterval() const;
};

// Global instance
extern SensorManager sensor_manager;

// C compatibility functions
#ifdef __cplusplus
extern "C" {
#endif

esp_err_t sensor_manager_init(spi_device_handle_t max6675_spi);
float sensor_read_temperature(void);
float sensor_read_pressure(void);
void sensor_calculate_flow_rates(float* flow1, float* flow2);
void sensor_read_all(sensor_data_t* data);
void sensor_update_history(const sensor_data_t* data, uint32_t current_time);
const SensorHistory* sensor_get_history(void);
void sensor_set_history_interval(uint32_t interval_ms);
uint32_t sensor_get_history_interval(void);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_MANAGER_H */ 