#include "pid_controller.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "esp_log.h"

static const char* TAG = "pid";

// C++ implementation
PIDController::PIDController(float kp,
                             float ki,
                             float kd,
                             float min_output,
                             float max_output,
                             uint32_t sample_time_ms)
{
    this->kp = kp;
    this->ki = ki;
    this->kd = kd;

    this->min_output = min_output;
    this->max_output = max_output;

    this->setpoint = 0.0f;
    this->input    = 0.0f;
    this->output   = 0.0f;

    this->error_sum   = 0.0f;
    this->last_error  = 0.0f;
    this->last_input  = 0.0f;
    this->initialized = false;

    this->sample_time_ms = sample_time_ms;
    this->last_time      = 0;

    ESP_LOGI(TAG, "PID controller initialized with kp=%.2f, ki=%.2f, kd=%.2f", kp, ki, kd);
}

void PIDController::setSetpoint(float setpoint)
{
    this->setpoint = setpoint;
    ESP_LOGD(TAG, "PID setpoint set to %.2f", setpoint);
}

void PIDController::setTunings(float kp, float ki, float kd)
{
    // Ensure positive gains
    if (kp < 0 || ki < 0 || kd < 0) {
        ESP_LOGW(TAG, "Negative PID gains not allowed");
        return;
    }

    this->kp = kp;
    this->ki = ki;
    this->kd = kd;

    ESP_LOGI(TAG, "PID tunings updated: kp=%.2f, ki=%.2f, kd=%.2f", kp, ki, kd);
}

void PIDController::setLimits(float min, float max)
{
    if (min >= max) {
        ESP_LOGW(TAG, "Invalid limits: min must be less than max");
        return;
    }

    this->min_output = min;
    this->max_output = max;

    // Constrain current output to new limits
    if (this->output > max) {
        this->output = max;
    }
    else if (this->output < min) {
        this->output = min;
    }

    ESP_LOGI(TAG, "PID limits set to [%.2f, %.2f]", min, max);
}

void PIDController::setSampleTime(uint32_t sample_time_ms)
{
    if (sample_time_ms <= 0) {
        ESP_LOGW(TAG, "Sample time must be positive");
        return;
    }

    // Adjust integral and derivative terms to account for time change
    float ratio = (float)sample_time_ms / (float)this->sample_time_ms;
    this->ki *= ratio;
    this->kd /= ratio;

    this->sample_time_ms = sample_time_ms;

    ESP_LOGI(TAG, "PID sample time set to %u ms", sample_time_ms);
}

void PIDController::reset()
{
    this->error_sum   = 0.0f;
    this->last_error  = 0.0f;
    this->last_input  = 0.0f;
    this->initialized = false;
    this->output      = 0.0f;

    ESP_LOGI(TAG, "PID controller reset");
}

float PIDController::compute(float input, uint32_t current_time)
{
    // Check if enough time has passed
    uint32_t time_diff = current_time - this->last_time;
    if (time_diff < this->sample_time_ms && this->initialized) {
        return this->output;  // Not enough time has passed
    }

    // Store current input
    this->input = input;

    // Calculate error terms
    float error = this->setpoint - input;

    // Calculate P term
    float p_term = this->kp * error;

    // Calculate I term
    this->error_sum += error * time_diff / 1000.0f;  // Time in seconds

    // Anti-windup: constrain error sum to produce output within limits
    float i_term = this->ki * this->error_sum;
    if (i_term > this->max_output) {
        i_term          = this->max_output;
        this->error_sum = i_term / this->ki;
    }
    else if (i_term < this->min_output) {
        i_term          = this->min_output;
        this->error_sum = i_term / this->ki;
    }

    // Calculate D term (on process variable change, not error)
    float d_term = 0.0f;
    if (this->initialized) {
        float d_input = (input - this->last_input) * 1000.0f / time_diff;  // Change rate per second
        d_term        = -this->kd * d_input;  // Negative because input rising = error falling
    }

    // Calculate output
    float output = p_term + i_term + d_term;

    // Apply output constraints
    if (output > this->max_output) {
        output = this->max_output;
    }
    else if (output < this->min_output) {
        output = this->min_output;
    }

    // Store state for next iteration
    this->last_input  = input;
    this->last_error  = error;
    this->last_time   = current_time;
    this->output      = output;
    this->initialized = true;

    ESP_LOGD(TAG,
             "PID: SP=%.2f, PV=%.2f, err=%.2f, P=%.2f, I=%.2f, D=%.2f, out=%.2f",
             this->setpoint,
             input,
             error,
             p_term,
             i_term,
             d_term,
             output);

    return output;
}

float PIDController::getError() const
{
    return this->setpoint - this->input;
}

// C wrapper functions for backward compatibility
extern "C" {

void pid_init(pid_controller_t* pid,
              float kp,
              float ki,
              float kd,
              float min_output,
              float max_output,
              uint32_t sample_time_ms)
{
    // Use placement new to initialize the C++ object in the memory provided
    new (pid) PIDController(kp, ki, kd, min_output, max_output, sample_time_ms);
}

void pid_set_setpoint(pid_controller_t* pid, float setpoint)
{
    pid->setSetpoint(setpoint);
}

void pid_set_tunings(pid_controller_t* pid, float kp, float ki, float kd)
{
    pid->setTunings(kp, ki, kd);
}

void pid_set_limits(pid_controller_t* pid, float min, float max)
{
    pid->setLimits(min, max);
}

void pid_set_sample_time(pid_controller_t* pid, uint32_t sample_time_ms)
{
    pid->setSampleTime(sample_time_ms);
}

void pid_reset(pid_controller_t* pid)
{
    pid->reset();
}

float pid_compute(pid_controller_t* pid, float input, uint32_t current_time)
{
    return pid->compute(input, current_time);
}

float pid_get_error(pid_controller_t* pid)
{
    return pid->getError();
}

}  // extern "C"