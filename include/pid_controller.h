#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <cstdbool>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

/**
 * @brief PID controller class definition
 */
class PIDController {
private:
    // PID parameters
    float kp;           // Proportional gain
    float ki;           // Integral gain 
    float kd;           // Derivative gain

    // Control limits
    float min_output;   // Minimum output value
    float max_output;   // Maximum output value

    // Control variables
    float setpoint;     // Desired control value
    float input;        // Current process value
    float output;       // Controller output

    // State variables
    float error_sum;    // Integrated error 
    float last_error;   // Previous error for derivative calc
    float last_input;   // Previous input for derivative calc
    bool initialized;   // Whether controller has received initial values

    // Time tracking
    uint32_t sample_time_ms; // Control loop interval in ms
    uint32_t last_time;      // Time of last computation

public:
    /**
     * @brief Initialize a PID controller
     *
     * @param kp Proportional gain
     * @param ki Integral gain
     * @param kd Derivative gain
     * @param min_output Minimum output value
     * @param max_output Maximum output value
     * @param sample_time_ms Control loop interval in ms
     */
    PIDController(float kp, float ki, float kd,
        float min_output, float max_output, uint32_t sample_time_ms);

    /**
     * @brief Set the PID setpoint
     *
     * @param setpoint New setpoint value
     */
    void setSetpoint(float setpoint);

    /**
     * @brief Set the PID tuning parameters
     *
     * @param kp Proportional gain
     * @param ki Integral gain
     * @param kd Derivative gain
     */
    void setTunings(float kp, float ki, float kd);

    /**
     * @brief Set the PID output limits
     *
     * @param min Minimum output value
     * @param max Maximum output value
     */
    void setLimits(float min, float max);

    /**
     * @brief Set the PID sample time
     *
     * @param sample_time_ms Sample time in milliseconds
     */
    void setSampleTime(uint32_t sample_time_ms);

    /**
     * @brief Reset the PID controller
     */
    void reset();

    /**
     * @brief Compute a new PID output value
     *
     * @param input Current process value
     * @param current_time Current time in milliseconds
     * @return Computed control output
     */
    float compute(float input, uint32_t current_time);

    /**
     * @brief Get current PID error (setpoint - input)
     *
     * @return Current error value
     */
    float getError() const;
    
    /**
     * @brief Get current output value
     *
     * @return Current output value
     */
    float getOutput() const { return output; }
    
    /**
     * @brief Get current setpoint
     *
     * @return Current setpoint value
     */
    float getSetpoint() const { return setpoint; }
};

// For backward compatibility with C code
typedef PIDController pid_controller_t;

// C-compatible wrapper functions for backward compatibility
#ifdef __cplusplus
extern "C" {
#endif

void pid_init(pid_controller_t* pid, float kp, float ki, float kd,
    float min_output, float max_output, uint32_t sample_time_ms);
void pid_set_setpoint(pid_controller_t* pid, float setpoint);
void pid_set_tunings(pid_controller_t* pid, float kp, float ki, float kd);
void pid_set_limits(pid_controller_t* pid, float min, float max);
void pid_set_sample_time(pid_controller_t* pid, uint32_t sample_time_ms);
void pid_reset(pid_controller_t* pid);
float pid_compute(pid_controller_t* pid, float input, uint32_t current_time);
float pid_get_error(pid_controller_t* pid);

#ifdef __cplusplus
}
#endif

#endif /* PID_CONTROLLER_H */ 