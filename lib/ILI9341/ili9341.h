#ifndef ILI9341_H
#define ILI9341_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ILI9341 SPI Display with I2C Touchscreen - Rolodex Interface
 * Board: ESP32-S3
 * Display: ER-TFTM024-3 (ILI9341 240x320)
 * Touch: Capacitive Touch Panel (I2C - FT6236/FT6206)
 * Interface: 4-wire SPI (J2,J3,J4,J5 Short and J1,J6,J7,J8 Open)
 * 
 * ========================================
 * ER-TFTM024-3 TO ESP32-S3 PINOUT
 * ========================================
 * 
 * DISPLAY MODULE CONNECTIONS (4-WIRE SPI):
 * ----------------------------------------
 * Pin 1  (VSS)          → GND
 * Pin 2  (VDD)          → 3.3V or 5V (Use 3.3V for ESP32-S3)
 * Pin 3-20 (DB0-DB17)   → Not used (parallel interface only)
 * Pin 21 (/RESET_NC)    → Not connected (Has onboard RC reset)
 * Pin 22 (TE)           → Not connected (Tearing effect, optional)
 * Pin 23 (LCD_/CS)      → GPIO 15 (ESP32-S3) - Chip Select
 * Pin 24 (D/C SCL)      → GPIO 6 (ESP32-S3) - SPI Clock (4-wire SPI mode)
 * Pin 25 (/WR D/C)      → GPIO 12 (ESP32-S3) - Data/Command Select (4-wire SPI mode)
 * Pin 26 (/RD)          → Not connected (Parallel interface only)
 * Pin 27 (LCD_SDI)      → GPIO 11 (ESP32-S3 MOSI) - Serial Data In
 * Pin 28 (LCD_SDO)      → GPIO 13 (ESP32-S3 MISO) - Serial Data Out (optional)
 * Pin 29 (BL_ON/OFF)    → GPIO 4 (ESP32-S3) - Backlight Control
 * Pin 30 (CTP_SCL)      → GPIO 9 (ESP32-S3) - I2C Clock for Capacitive Touch
 * Pin 31 (CTP_SDA)      → GPIO 8 (ESP32-S3) - I2C Data for Capacitive Touch
 * Pin 32 (SDO)          → Not connected (RTP/SD/Flash serial output)
 * Pin 33 (SCL)          → Not connected (Used for SD/Flash, not LCD SPI)
 * Pin 34 (SDI)          → Not connected (RTP/SD/Flash serial input)
 * Pin 35 (SD_/CS)       → Not connected (SD card chip select)
 * Pin 36 (FONT_/CS)     → Not connected (Font chip select)
 * Pin 37 (FLASH_/CS)    → Not connected (Flash chip select)
 * Pin 38 (FLASH_/WP)    → Not connected (Flash write protect)
 * Pin 39 (CTP_INT)      → GPIO 3 (ESP32-S3) - Touch Interrupt (optional, J9 Short)
 * Pin 40 (VSS)          → GND
 * 
 * SUMMARY - ESP32-S3 GPIO ASSIGNMENTS (Ordered by DevKit Pin Layout):
 * --------------------------------------------------------------------
 * 3V3               → VDD (Pin 2) - 3.3V Power to Display
 * EN                → Not used
 * GPIO 4            → BL_ON/OFF (Pin 29) - Backlight Control
 * GPIO 5            → Not used
 * GPIO 6            → D/C(SCL) (Pin 24) - SPI Clock (4-wire SPI mode)
 * GPIO 7            → Not used
 * GPIO 15           → LCD_/CS (Pin 23) - Display Chip Select
 * GPIO 16           → Not used
 * GPIO 8            → CTP_SDA (Pin 31) - I2C Touch Data
 * GPIO 3            → CTP_INT (Pin 39) - Touch Interrupt (optional)
 * GPIO 46           → Not used
 * GPIO 9            → CTP_SCL (Pin 30) - I2C Touch Clock
 * GPIO 10           → Not used
 * GPIO 11           → LCD_SDI (Pin 27) - SPI MOSI
 * GPIO 12           → /WR(D/C) (Pin 25) - Data/Command (4-wire SPI mode)
 * GPIO 13           → LCD_SDO (Pin 28) - SPI MISO (optional)
 * GPIO 14           → Not used
 * 5V0               → Not used (using 3.3V)
 * GND               → VSS (Pins 1, 40) - Ground
 * 
 * POWER CONNECTIONS:
 * ------------------
 * VDD (Pin 2): 3.3V (JP open for 5V or JP short for 3.3V - use 3.3V)
 * GND (Pins 1, 40): Ground
 * 
 * JUMPER SETTINGS ON ER-TFTM024-3:
 * ---------------------------------
 * JP: Open (5V VDD) or Short (3.3V VDD) - Use 3.3V for ESP32-S3
 * J2, J3, J4, J5: Short (4-wire SPI mode)
 * J1, J6, J7, J8: Open (4-wire SPI mode)
 * J9, J11, J12, J13: Short (Capacitive Touch Panel)
 * J10: Open
 * 
 * NOTES:
 * - Module configured for 4-wire SPI interface
 * - Capacitive touch uses I2C (not resistive touch SPI)
 * - Backlight uses GPIO 4 with PWM for brightness control (0-255)
 * - Touch interrupt (CTP_INT) is optional but recommended
 * - All signals are 3.3V logic level
 */

// Display dimensions (Landscape mode)
#define ILI9341_WIDTH  320
#define ILI9341_HEIGHT 240

// RGB565 Color definitions
#define ILI9341_BLACK   0x0000
#define ILI9341_WHITE   0xFFFF
#define ILI9341_RED     0xF800
#define ILI9341_GREEN   0x07E0
#define ILI9341_BLUE     0x001F
#define ILI9341_CYAN     0x07FF
#define ILI9341_MAGENTA  0xF81F
#define ILI9341_YELLOW   0xFFE0
#define ILI9341_ORANGE   0xFD20
#define ILI9341_GRAY     0x8410
#define ILI9341_DARKGREY 0x4208
#define ILI9341_NAVY     0x000F
#define ILI9341_LIGHTGRAY 0xF7DE

// Pin configuration structure
typedef struct {
    int pin_mosi;
    int pin_miso;
    int pin_sclk;
    int pin_cs;
    int pin_dc;
    int pin_rst;
    int pin_bl;
    int spi_host;      // SPI2_HOST or SPI3_HOST
    int spi_clock_mhz; // SPI clock speed in MHz (e.g., 40)
} ili9341_config_t;

/**
 * @brief Initialize ILI9341 display
 * @param config Pin and SPI configuration
 * @return true on success, false on failure
 */
bool ili9341_init(const ili9341_config_t *config);

/**
 * @brief Set address window for subsequent pixel writes
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 */
void ili9341_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief Draw a single pixel
 * @param x X coordinate
 * @param y Y coordinate
 * @param color RGB565 color
 */
void ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief Fill entire screen with a color
 * @param color RGB565 color
 */
void ili9341_fill_screen(uint16_t color);

/**
 * @brief Fill a rectangle with a color
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color RGB565 color
 */
void ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief Draw a rectangle outline
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color RGB565 color
 */
void ili9341_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief Draw a line
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 * @param color RGB565 color
 */
void ili9341_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

/**
 * @brief Write a single RGB565 color to display (address window must be set first)
 * @param color RGB565 color
 */
void ili9341_write_color(uint16_t color);

/**
 * @brief Write multiple colors to display (bulk transfer)
 */
void ili9341_write_colors(const uint16_t* colors, uint32_t length);

/**
 * @brief Set backlight brightness (0-255)
 * @param brightness Brightness percentage
 */
void ili9341_set_backlight(uint8_t brightness);

/**
 * @brief Enter sleep mode
 */
void ili9341_sleep(void);

/**
 * @brief Exit sleep mode
 */
void ili9341_wake(void);

#ifdef __cplusplus
}
#endif

#endif // ILI9341_H
