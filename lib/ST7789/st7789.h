#ifndef ST7789_H
#define ST7789_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ST7789VI SPI Display with I2C Touchscreen
 * Board: Teensy 4.0
 * Display: ER-TFTM024-3 (ST7789VI 240x320)
 * Touch: Capacitive Touch Panel (I2C - FT5x26)
 * Interface: 3-wire SPI (IM0=1, IM2=1)
 * 
 * ========================================
 * ER-TFTM024-3 TO TEENSY 4.0 PINOUT
 * ========================================
 * 
 * DISPLAY MODULE CONNECTIONS (3-WIRE SPI):
 * ----------------------------------------
 * TFT CONNECTIONS:
 * Pin 1  (GND)          → GND
 * Pin 6  (SDO)          → Not connected (MISO - optional for read)
 * Pin 7  (VDD)          → 3.3V Power Supply
 * Pin 8  (VDDI)         → 3.3V Logic Supply
 * Pin 9  (SDA)          → Teensy Pin 11 (MOSI) - Serial Data In
 * Pin 10 (CSX)          → Teensy Pin 10 (CS) - Chip Select (Active Low)
 * Pin 11 (DCX)          → Teensy Pin 9 - Data/Command Select
 * Pin 12 (WRX)          → Teensy Pin 13 (SCK) - SPI Clock (Active Low Write)
 * Pin 13 (RDX)          → 3.3V (Not used - tie high)
 * Pin 14-29 (DB0-DB15)  → Not connected (Parallel mode only)
 * Pin 30 (RESX)         → Teensy Pin 8 - Reset (Active Low, optional)
 * Pin 31 (IM0)          → 3.3V (3-wire SPI mode)
 * Pin 32 (IM2)          → 3.3V (3-wire SPI mode)
 * Pin 33 (GND)          → GND
 * Pin 34-37 (LED-K1-4)  → GND (Backlight Cathode)
 * Pin 38 (LED-A)        → Teensy Pin 6 (PWM) - Backlight Anode via resistor
 * Pin 39 (GND)          → GND
 * Pin 40 (TE)           → Not connected (Tearing Effect - optional)
 *
 * CTP (CAPACITIVE TOUCH) CONNECTIONS:
 * Pin 1  (VDD)          → 3.3V
 * Pin 2  (VSS)          → GND
 * Pin 3  (SCL)          → Teensy Pin 19 (SCL0) - I2C Clock
 * Pin 4  (SDA)          → Teensy Pin 18 (SDA0) - I2C Data
 * Pin 5  (/INT)         → Teensy Pin 7 - Touch Interrupt (optional)
 * Pin 6  (/RESET)       → Teensy Pin 8 - Touch Reset (shared with display)
 * 
 * SUMMARY - TEENSY 4.0 PIN ASSIGNMENTS:
 * --------------------------------------
 * Pin 6  → LED-A (Backlight PWM)
 * Pin 7  → CTP_INT (Touch Interrupt)
 * Pin 8  → RESX (Display & Touch Reset)
 * Pin 9  → DCX (Data/Command Select)
 * Pin 10 → CSX (SPI Chip Select)
 * Pin 11 → SDA (SPI MOSI)
 * Pin 13 → WRX (SPI Clock SCK)
 * Pin 18 → CTP_SDA (I2C Data)
 * Pin 19 → CTP_SCL (I2C Clock)
 * 3.3V → VDD, VDDI, IM0, IM2, RDX
 * GND → GND, LED-K
 * 
 * INTERFACE MODE SELECTION (3-WIRE SPI):
 * ---------------------------------------
 * IM0 = 1 (3.3V)
 * IM2 = 1 (3.3V)
 * 
 * NOTES:
 * - 3-wire SPI mode: WRX=SCK, DCX=D/C, SDA=MOSI
 * - Backlight LED-A requires current limiting resistor (100-150Ω for ~20mA)
 * - Touch uses separate I2C interface (FT5x26)
 * - All signals are 3.3V logic level
 * - RESX can be shared between display and touch controller
 * - RDX tied high since read operations not used
 */

// Display dimensions (Portrait mode)
#define ST7789_WIDTH  240
#define ST7789_HEIGHT 320

// SPI Communication modes
#define ST7789_SPI_MODE_HW_4WIRE  0  // Hardware SPI with DC pin (default)
#define ST7789_SPI_MODE_HW_3WIRE  1  // Hardware SPI with D/C as first bit
#define ST7789_SPI_MODE_BITBANG   2  // Bit-banged SPI

// RGB565 Color definitions
#define ST7789_BLACK   0x0000
#define ST7789_WHITE   0xFFFF
#define ST7789_RED     0xF800
#define ST7789_GREEN   0x07E0
#define ST7789_BLUE     0x001F
#define ST7789_CYAN     0x07FF
#define ST7789_MAGENTA  0xF81F
#define ST7789_YELLOW   0xFFE0
#define ST7789_ORANGE   0xFD20
#define ST7789_GRAY     0x8410
#define ST7789_DARKGREY 0x4208
#define ST7789_NAVY     0x000F
#define ST7789_LIGHTGRAY 0xF7DE

// Pin configuration structure
typedef struct {
    int pin_mosi;      // SDA - MOSI (Pin 9 on display)
    int pin_miso;      // SDO - MISO (Pin 6 on display, optional)
    int pin_sclk;      // WRX - SCK (Pin 12 on display)
    int pin_cs;        // CSX - Chip Select (Pin 10 on display)
    int pin_dc;        // DCX - Data/Command (Pin 11 on display)
    int pin_rst;       // RESX - Reset (Pin 30 on display, optional)
    int pin_bl;        // LED-A - Backlight (Pin 38 on display)
    int pin_im0;       // IM0 - Interface mode bit 0 (Pin 31 on display)
    int pin_im2;       // IM2 - Interface mode bit 2 (Pin 32 on display)
    int spi_clock_mhz; // SPI clock speed in MHz (e.g., 30 for Teensy 4.0)
} st7789_config_t;

/**
 * @brief Initialize ST7789VI display
 * @param config Pin and SPI configuration
 * @return true on success, false on failure
 */
bool st7789_init(const st7789_config_t *config);

/**
 * @brief Set address window for subsequent pixel writes
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 */
void st7789_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief Draw a single pixel
 * @param x X coordinate
 * @param y Y coordinate
 * @param color RGB565 color
 */
void st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief Fill entire screen with a color
 * @param color RGB565 color
 */
void st7789_fill_screen(uint16_t color);

/**
 * @brief Fill a rectangle with a color
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color RGB565 color
 */
void st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief Draw a rectangle outline
 * @param x X coordinate
 * @param y Y coordinate
 * @param w Width
 * @param h Height
 * @param color RGB565 color
 */
void st7789_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief Draw a line
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 * @param color RGB565 color
 */
void st7789_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

/**
 * @brief Write a single RGB565 color to display (address window must be set first)
 * @param color RGB565 color
 */
void st7789_write_color(uint16_t color);

/**
 * @brief Write multiple colors to display (bulk transfer)
 */
void st7789_write_colors(const uint16_t* colors, uint32_t length);

/**
 * @brief Set backlight brightness (0-255)
 * @param brightness Brightness percentage
 */
void st7789_set_backlight(uint8_t brightness);

/**
 * @brief Enter sleep mode
 */
void st7789_sleep(void);

/**
 * @brief Exit sleep mode
 */
void st7789_wake(void);

/**
 * @brief Turn display on
 */
void st7789_display_on(void);

/**
 * @brief Set SPI settings for testing different speeds and modes
 * @param speed SPI clock speed in Hz
 * @param mode SPI mode (SPI_MODE0, SPI_MODE1, SPI_MODE2, or SPI_MODE3)
 */
void st7789_set_spi_settings(uint32_t speed, uint8_t mode);

/**
 * @brief Set SPI communication mode
 * @param mode ST7789_SPI_MODE_HW_4WIRE, ST7789_SPI_MODE_HW_3WIRE, or ST7789_SPI_MODE_BITBANG
 */
void st7789_set_spi_mode(uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif // ST7789_H
