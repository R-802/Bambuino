#ifndef HARDWARE_CONTROL_H
#define HARDWARE_CONTROL_H

#ifdef __cplusplus
#include <cstddef>  // For std::size_t
#else
#include <stdbool.h> // For bool in C context  
#endif

#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "esp_log.h"

// Constants and definitions
#define ADC_PRESSURE_CHANNEL ADC1_CHANNEL_0  // Pressure transducer ADC channel
#define DIMMER_PIN GPIO_NUM_12               // AC Dimmer control pin

// SSR pins - multiple relays
#define SSR_COUNT 4            // Number of SSR relays
#define SSR_PIN_1 GPIO_NUM_13  // Heater SSR
#define SSR_PIN_2 GPIO_NUM_16  // Additional SSR 2
#define SSR_PIN_3 GPIO_NUM_17  // Additional SSR 3
#define SSR_PIN_4 GPIO_NUM_18  // Additional SSR 4

// MAX6675 thermocouple interface pins
#define MAX6675_CS_PIN GPIO_NUM_10
#define MAX6675_SCK_PIN GPIO_NUM_11
#define MAX6675_MISO_PIN GPIO_NUM_9
#define MAX6675_MOSI_PIN -1  // Not used for MAX6675

// Flow meter pins (interrupts)
#define FLOW_METER1_PIN GPIO_NUM_14
#define FLOW_METER2_PIN GPIO_NUM_15

// C++ class to handle hardware control
class HardwareControl {
private:
    // Internal state
    bool initialized;
    uint32_t dimmer_level;
    bool ssr_states[SSR_COUNT];
    spi_device_handle_t max6675_spi;

public:
    // Public static constants
    static const uint8_t SSR_PINS[SSR_COUNT];
    static const char* SSR_NAMES[SSR_COUNT];
    static uint32_t flow_meter1_count;
    static uint32_t flow_meter2_count;

    // Constructor
    HardwareControl();
    
    // Destructor
    ~HardwareControl();

    /**
     * @brief Initialize all hardware components
     * 
     * @return ESP_OK on success, or error code
     */
    esp_err_t init();

    /**
     * @brief Initialize the pressure sensor ADC
     */
    void initPressureSensor();

    /**
     * @brief Initialize the dimmer using LEDC for PWM
     */
    void initDimmer();

    /**
     * @brief Initialize SPI for MAX6675 K-type thermocouple
     */
    void initMax6675();

    /**
     * @brief Initialize flow meters with interrupt handlers
     */
    void initFlowMeters();

    /**
     * @brief Initialize the Solid State Relays
     */
    void initSSR();

    /**
     * @brief Set the dimmer level
     *
     * @param level PWM level (0-1023)
     */
    void setDimmer(uint32_t level);

    /**
     * @brief Set the state of a specific SSR
     *
     * @param index SSR index (0-3)
     * @param state SSR state (true for ON, false for OFF)
     */
    void setSSRState(int index, bool state);

    /**
     * @brief Set PWM value for an SSR (for PWM-capable relays)
     *
     * @param index SSR index (0-3)
     * @param pwm PWM value (0.0 to 1.0)
     */
    void setSSRPWM(int index, float pwm);

    /**
     * @brief Set the state of all SSRs
     *
     * @param state SSR state (true for ON, false for OFF)
     */
    void setAllSSR(bool state);

    /**
     * @brief Get MAX6675 SPI handle
     *
     * @return SPI device handle for MAX6675
     */
    spi_device_handle_t getMax6675Handle() const { return max6675_spi; }
};

// Global instance
extern HardwareControl hw;

// C compatibility functions
#ifdef __cplusplus
extern "C" {
#endif

esp_err_t hw_init(void);
void hw_init_pressure_sensor(void);
void hw_init_dimmer(void);
void hw_init_max6675(spi_device_handle_t* spi_handle);
void hw_init_flow_meters(void);
void hw_init_ssr(void);
void hw_set_dimmer(uint32_t level);
void hw_set_ssr_state(int index, bool state);
void hw_set_ssr_pwm(int index, float pwm);
void hw_set_all_ssr(bool state);
void hardware_control_init(void);

#ifdef __cplusplus
}
#endif

#endif /* HARDWARE_CONTROL_H */ 