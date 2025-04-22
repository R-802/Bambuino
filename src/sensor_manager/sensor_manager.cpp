#include "sensor_manager/sensor_manager.h"

#include <cstring>
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "hardware/hardware_control.h"

// Forward declarations of ISR functions defined in hardware_control.cpp
extern "C" void IRAM_ATTR flow_meter1_isr(void* arg);
extern "C" void IRAM_ATTR flow_meter2_isr(void* arg);

static const char* TAG = "SENSOR_MGR";

// Global instance
SensorManager sensor_manager;

// SensorHistory implementation
SensorHistory::SensorHistory()
    : write_index(0), last_update_time(0), update_interval_ms(1000)
{
    reset();
}

void SensorHistory::reset()
{
    for (int i = 0; i < SENSOR_HISTORY_LENGTH; i++) {
        temperature[i] = 0.0f;
        pressure[i] = 0.0f;
        flow_rate1[i] = 0.0f;
        flow_rate2[i] = 0.0f;
    }
    write_index = 0;
    last_update_time = 0;
}

// SensorManager implementation
SensorManager::SensorManager()
    : max6675_spi(nullptr), initialized(false)
{
}

esp_err_t SensorManager::init(spi_device_handle_t spi_handle)
{
    ESP_LOGI(TAG, "Initializing sensor manager");
    max6675_spi = spi_handle;
    initialized = true;
    return ESP_OK;
}

float SensorManager::readTemperature()
{
    uint16_t data = 0;
    spi_transaction_t t = {
        .length = 16,
        .rx_buffer = &data,
        .flags = SPI_TRANS_USE_RXDATA,
    };

    if (spi_device_polling_transmit(max6675_spi, &t) != ESP_OK) {
        ESP_LOGE(TAG, "Error reading MAX6675");
        return -1.0;
    }

    // Convert SPI data to temperature (MAX6675 format)
    data = (t.rx_data[0] << 8) | t.rx_data[1];

    // Check if thermocouple is open
    if (data & 0x04) {
        ESP_LOGE(TAG, "Thermocouple is open/disconnected");
        return -1.0;
    }

    // Extract temperature (12 bits, bit 3-14)
    data >>= 3;

    // MAX6675 LSB = 0.25 degrees Celsius
    return data * 0.25;
}

float SensorManager::readPressure()
{
    // Read raw ADC value
    int raw_value = adc1_get_raw(ADC_PRESSURE_CHANNEL);

    // Convert to pressure - this needs calibration based on your specific sensor
    // This is a simple linear conversion example - adjust based on your sensor
    // Assuming 0-5V sensor with 0-100 PSI range
    float voltage = (raw_value / 4095.0) * 3.3;  // ESP32 ADC voltage reference (3.3V)
    float pressure = (voltage / 3.3) * 100.0;     // Convert to PSI based on 0-100 PSI range

    return pressure;
}

void SensorManager::calculateFlowRates(float* flow1, float* flow2)
{
    // These conversion factors depend on your specific flow meters
    // Typically it's pulses per liter, so adjust as needed
    const float FLOW_FACTOR1 = 5.5;  // Example: 5.5 pulses per mL
    const float FLOW_FACTOR2 = 5.5;  // Example: 5.5 pulses per mL

    // Disable interrupt temporarily
    gpio_isr_handler_remove(FLOW_METER1_PIN);
    gpio_isr_handler_remove(FLOW_METER2_PIN);

    // Read pulse counts
    uint32_t count1 = HardwareControl::flow_meter1_count;
    uint32_t count2 = HardwareControl::flow_meter2_count;

    // Reset counters
    HardwareControl::flow_meter1_count = 0;
    HardwareControl::flow_meter2_count = 0;

    // Re-enable interrupt
    gpio_isr_handler_add(FLOW_METER1_PIN, flow_meter1_isr, NULL);
    gpio_isr_handler_add(FLOW_METER2_PIN, flow_meter2_isr, NULL);

    // Calculate flow rates (mL/min) assuming measurements over 1 second
    *flow1 = (count1 / FLOW_FACTOR1) * 60.0;
    *flow2 = (count2 / FLOW_FACTOR2) * 60.0;
}

void SensorManager::readAll(sensor_data_t* data)
{
    if (data == NULL) {
        ESP_LOGE(TAG, "Invalid sensor data pointer");
        return;
    }

    // Read temperature
    data->temperature = readTemperature();

    // Read pressure
    data->pressure = readPressure();

    // Calculate flow rates
    calculateFlowRates(&data->flow_rate1, &data->flow_rate2);

    // Log sensor readings
    ESP_LOGI(TAG, "Temperature: %.2f°C", data->temperature);
    ESP_LOGI(TAG, "Pressure: %.2f PSI", data->pressure);
    ESP_LOGI(TAG, "Flow rate 1: %.2f mL/min", data->flow_rate1);
    ESP_LOGI(TAG, "Flow rate 2: %.2f mL/min", data->flow_rate2);
}

void SensorManager::updateHistory(const sensor_data_t* data, uint32_t current_time)
{
    if (data == NULL) {
        return;
    }

    // Check if it's time to record a new data point
    if (current_time - sensor_history.last_update_time >= sensor_history.update_interval_ms) {
        // Store current values in history arrays
        sensor_history.temperature[sensor_history.write_index] = data->temperature;
        sensor_history.pressure[sensor_history.write_index] = data->pressure;
        sensor_history.flow_rate1[sensor_history.write_index] = data->flow_rate1;
        sensor_history.flow_rate2[sensor_history.write_index] = data->flow_rate2;

        // Move to next index, wrapping around if needed
        sensor_history.write_index = (sensor_history.write_index + 1) % SENSOR_HISTORY_LENGTH;

        // Update timestamp
        sensor_history.last_update_time = current_time;

        ESP_LOGD(TAG, "Sensor history updated, index=%d", sensor_history.write_index);
    }
}

const SensorHistory* SensorManager::getHistory() const
{
    return &sensor_history;
}

void SensorManager::setHistoryInterval(uint32_t interval_ms)
{
    if (interval_ms > 0) {
        sensor_history.update_interval_ms = interval_ms;
        ESP_LOGI(TAG, "Sensor history interval set to %u ms", interval_ms);
    }
}

uint32_t SensorManager::getHistoryInterval() const
{
    return sensor_history.update_interval_ms;
}

// C compatibility wrappers
extern "C" {

esp_err_t sensor_manager_init(spi_device_handle_t max6675_spi)
{
    return sensor_manager.init(max6675_spi);
}

float sensor_read_temperature(void)
{
    return sensor_manager.readTemperature();
}

float sensor_read_pressure(void)
{
    return sensor_manager.readPressure();
}

void sensor_calculate_flow_rates(float* flow1, float* flow2)
{
    sensor_manager.calculateFlowRates(flow1, flow2);
}

void sensor_read_all(sensor_data_t* data)
{
    sensor_manager.readAll(data);
}

void sensor_update_history(const sensor_data_t* data, uint32_t current_time)
{
    sensor_manager.updateHistory(data, current_time);
}

const SensorHistory* sensor_get_history(void)
{
    return sensor_manager.getHistory();
}

void sensor_set_history_interval(uint32_t interval_ms)
{
    sensor_manager.setHistoryInterval(interval_ms);
}

uint32_t sensor_get_history_interval(void)
{
    return sensor_manager.getHistoryInterval();
}

} // extern "C" 