#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include "esp_err.h"

/**
 * @brief Display driver class for handling LCD and UI
 */
class DisplayDriver {
private:
    void* panel_handle;
    int width;
    int height;
    bool initialized;
    
    // Private methods for internal implementation
    static bool touchEventHandler(uint16_t* x, uint16_t* y);

public:
    /**
     * @brief Constructor
     */
    DisplayDriver();
    
    /**
     * @brief Initialize display hardware and Slint UI renderer
     *
     * This function initializes the display hardware (LCD panel and touch),
     * as well as the Slint UI renderer that will be used to render the UI.
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t init();
    
    /**
     * @brief Acquire Slint UI mutex
     * 
     * Call this before performing UI operations from tasks other than the 
     * Slint rendering task. This ensures thread safety in the UI layer.
     */
    void acquireUIMutex();
    
    /**
     * @brief Release Slint UI mutex
     * 
     * Call this after performing UI operations from tasks other than the
     * Slint rendering task. This ensures thread safety in the UI layer.
     */
    void releaseUIMutex();
    
    /**
     * @brief Process a frame of the UI
     * 
     * This should be called in the main rendering loop to update the UI
     */
    void tick();
};

// Global instance
extern DisplayDriver display;

// C compatibility functions
#ifdef __cplusplus
extern "C" {
#endif

esp_err_t display_init(void);
void display_slint_acquire(void);
void display_slint_release(void);

// Slint function prototypes - implemented in display_driver.cpp
// These are internal to the display driver, but declared here to make
// them available to the Slint bindings
void slint_init_with_custom_renderer(int width, int height, void* panel_handle);
void slint_register_touch_event_handler(bool (*handler)(uint16_t* x, uint16_t* y));
void slint_tick(void);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_DRIVER_H */ 