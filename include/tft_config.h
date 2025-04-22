#ifndef TFT_CONFIG_H
#define TFT_CONFIG_H

// Display controller type - changed to a common OLED controller
// Replace with your specific OLED controller if different
#define SSD1963_DRIVER

// Higher resolution for a 5-inch display
#define TFT_WIDTH  800
#define TFT_HEIGHT 480

// Pin definitions for your ESP32-S3 and display connection
// SPI pins - adjust based on your wiring
#define TFT_MISO 13
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10  // Chip select control pin
#define TFT_DC   9   // Data/Command control pin
#define TFT_RST  14  // Reset pin
#define TFT_BL   15  // Backlight control pin (might not be needed for OLED)

// Touch screen configuration
#define TOUCH_SPI_FREQUENCY 2500000  // Touch SPI frequency in Hz
#define SPI_FREQUENCY       60000000 // Display SPI frequency in Hz - increased for larger display
#define SPI_READ_FREQUENCY  20000000 // Display SPI read frequency in Hz

// Rotation of the display (0-3)
#define TFT_ROTATION 0

// Color depth (16 bits for OLED, could be 16 or 24 depending on display)
#define COLOR_DEPTH 16

// OLED specific settings
#define OLED_DISPLAY
#define INVERT_COLORS 0
#define ENABLE_EXTENDED_SETTINGS 1

// Font rendering and other optimizations
#define SMOOTH_FONT
#define LOAD_GLCD   // Standard ASCII 8x5 font
#define LOAD_FONT2  // Small 16 pixel high font
#define LOAD_FONT4  // Medium 26 pixel high font
#define LOAD_FONT6  // Large 48 pixel high font
#define LOAD_FONT7  // Medium 16 pixel high 7-segment font
#define LOAD_FONT8  // Large 75 pixel high 7-segment font
#define LOAD_GFXFF  // Freefont support

// Color definitions
#define TFT_BLACK       0x0000
#define TFT_NAVY        0x000F
#define TFT_DARKGREEN   0x03E0
#define TFT_DARKCYAN    0x03EF
#define TFT_MAROON      0x7800
#define TFT_PURPLE      0x780F
#define TFT_OLIVE       0x7BE0
#define TFT_LIGHTGREY   0xD69A
#define TFT_DARKGREY    0x7BEF
#define TFT_BLUE        0x001F
#define TFT_GREEN       0x07E0
#define TFT_CYAN        0x07FF
#define TFT_RED         0xF800
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_WHITE       0xFFFF
#define TFT_ORANGE      0xFDA0
#define TFT_GREENYELLOW 0xB7E0
#define TFT_PINK        0xFE19

#endif /* TFT_CONFIG_H */ 