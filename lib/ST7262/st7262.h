#ifndef ST7262_H
#define ST7262_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ST7262 24-bit Parallel RGB Display Driver
 * Board: Teensy 4.0
 * Display: 800x480 5-inch TFT
 * Interface: 24-bit Parallel RGB
 */

// Display dimensions
#define ST7262_WIDTH  800
#define ST7262_HEIGHT 480

// RGB888 Color definitions (24-bit)
#define ST7262_BLACK   0x000000
#define ST7262_WHITE   0xFFFFFF
#define ST7262_RED     0xFF0000
#define ST7262_GREEN   0x00FF00
#define ST7262_BLUE    0x0000FF
#define ST7262_CYAN    0x00FFFF
#define ST7262_MAGENTA 0xFF00FF
#define ST7262_YELLOW  0xFFFF00
#define ST7262_ORANGE  0xFF8000
#define ST7262_GRAY    0x808080
#define ST7262_DARKGREY 0x404040
#define ST7262_NAVY     0x000080
#define ST7262_LIGHTGRAY 0xC0C0C0

// Pin configuration structure for 24-bit parallel RGB
typedef struct {
    // RGB Data pins (24-bit)
    int pin_r[8];  // Red data pins (R0-R7)
    int pin_g[8];  // Green data pins (G0-G7)
    int pin_b[8];  // Blue data pins (B0-B7)
    
    // Control pins
    int pin_de;    // Data Enable
    int pin_vsync; // Vertical Sync
    int pin_hsync; // Horizontal Sync
    int pin_pclk;  // Pixel Clock
    int pin_rst;   // Reset (optional)
    int pin_bl;    // Backlight (optional)
    
    // Timing configuration
    uint32_t pixel_clock_hz; // Pixel clock frequency
} st7262_config_t;

/**
 * @brief Initialize ST7262 display
 * @param config Pin and timing configuration
 * @return true on success, false on failure
 */
bool st7262_init(const st7262_config_t *config);

/**
 * @brief Set address window for subsequent pixel writes
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 */
void st7262_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief Draw a single pixel
 * @param x X coordinate
 * @param y Y coordinate
 * @param color RGB888 color (24-bit)
 */
void st7262_draw_pixel(uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief Fill entire screen with a color
 * @param color RGB888 color (24-bit)
 */
void st7262_fill_screen(uint32_t color);

/**
 * @brief Fill a rectangle with a color
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color RGB888 color (24-bit)
 */
void st7262_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);

/**
 * @brief Write a single RGB888 color to display (address window must be set first)
 * @param color RGB888 color (24-bit)
 */
void st7262_write_color(uint32_t color);

/**
 * @brief Control backlight
 * @param enable true to turn on, false to turn off
 */
void st7262_backlight(bool enable);

#ifdef __cplusplus
}
#endif

#endif // ST7262_H
