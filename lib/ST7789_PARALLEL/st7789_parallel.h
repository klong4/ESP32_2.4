#ifndef ST7789_PARALLEL_H
#define ST7789_PARALLEL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ST7789VI 8/16-bit Parallel Display Driver
 * Board: Teensy 4.0
 * Display: ST7789VI 240x320
 * Interface: 8-bit or 16-bit Parallel
 */

// Display dimensions
#define ST7789_PARALLEL_WIDTH  240
#define ST7789_PARALLEL_HEIGHT 320

// RGB565 Color definitions
#define ST7789_PARALLEL_BLACK   0x0000
#define ST7789_PARALLEL_WHITE   0xFFFF
#define ST7789_PARALLEL_RED     0xF800
#define ST7789_PARALLEL_GREEN   0x07E0
#define ST7789_PARALLEL_BLUE    0x001F
#define ST7789_PARALLEL_CYAN    0x07FF
#define ST7789_PARALLEL_MAGENTA 0xF81F
#define ST7789_PARALLEL_YELLOW  0xFFE0
#define ST7789_PARALLEL_ORANGE  0xFD20
#define ST7789_PARALLEL_GRAY    0x8410

// Pin configuration structure
typedef struct {
    int pin_data[16];  // Data pins DB0-DB15 (use first 8 for 8-bit mode)
    int pin_dc;        // Data/Command
    int pin_cs;        // Chip Select
    int pin_wr;        // Write strobe
    int pin_rd;        // Read strobe
    int pin_rst;       // Reset (optional)
    int pin_bl;        // Backlight (optional)
    int pin_im0;       // IM0 - Interface mode bit 0
    int pin_im2;       // IM2 - Interface mode bit 2
    bool use_16bit;    // true = 16-bit mode, false = 8-bit mode
} st7789_parallel_config_t;

/**
 * @brief Initialize ST7789 in parallel mode
 * @param config Pin configuration
 * @return true on success, false on failure
 */
bool st7789_parallel_init(const st7789_parallel_config_t *config);

/**
 * @brief Turn display on
 */
void st7789_parallel_display_on(void);

/**
 * @brief Control backlight
 * @param enable true to turn on, false to turn off
 */
void st7789_parallel_backlight(bool enable);

/**
 * @brief Set address window for subsequent pixel writes
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 */
void st7789_parallel_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief Draw a single pixel
 * @param x X coordinate
 * @param y Y coordinate
 * @param color RGB565 color
 */
void st7789_parallel_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief Fill entire screen with a color
 * @param color RGB565 color
 */
void st7789_parallel_fill_screen(uint16_t color);

/**
 * @brief Fill a rectangle with a color
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color RGB565 color
 */
void st7789_parallel_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief Write a single RGB565 color
 * @param color RGB565 color
 */
void st7789_parallel_write_color(uint16_t color);

/**
 * @brief Write multiple RGB565 pixels
 * @param pixels Pointer to pixel array
 * @param length Number of pixels to write
 */
void st7789_parallel_write_pixels(const uint16_t* pixels, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif // ST7789_PARALLEL_H
