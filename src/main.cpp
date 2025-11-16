#include <Arduino.h>
#include "st7789.h"

// Pin configuration - ER-TFTM024-3 to Teensy 4.0 (3-wire SPI + I2C Touch)
// 3-Wire SPI Mode: IM0=1, IM2=1

// TFT LCD (ST7789VI) - 3-Wire SPI
#define TFT_DC      9   // Pin 9  → Pin 11 DCX (Data/Command Select)
#define TFT_CS      10  // Pin 10 → Pin 10 CSX (Chip Select)
#define TFT_MOSI    11  // Pin 11 → Pin 9 SDA (MOSI - Serial Data In)
#define TFT_SCLK    13  // Pin 13 → Pin 12 WRX (SCK - SPI Clock)
#define TFT_MISO    12  // Pin 12 → Pin 6 SDO (MISO - optional, for read)
#define TFT_RST     8   // Pin 8  → Pin 30 RESX (Reset, optional)
#define TFT_BL      6   // Pin 6  → Pin 38 LED-A (Backlight PWM)

// CTP Touch (FT5x26) - I2C
#define TOUCH_SDA   18  // Pin 18 (SDA0) → CTP Pin 4 SDA (I2C Data)
#define TOUCH_SCL   19  // Pin 19 (SCL0) → CTP Pin 3 SCL (I2C Clock)
#define TOUCH_INT   7   // Pin 7  → CTP Pin 5 /INT (Touch Interrupt, optional)

// SD Card - Shared SPI bus
#define SD_CS       1   // Pin 1 â†’ SD Card Chip Select

// NOTE: Interface Mode Selection for 3-Wire SPI
// IM0 (Pin 31) = 3.3V (HIGH)
// IM2 (Pin 32) = 3.3V (HIGH)
// RDX (Pin 13) = 3.3V (HIGH - read not used)

void setup() {
    // Wait 3 seconds for serial monitor to connect
    Serial.begin(115200);
    delay(3000);
    
    Serial.println("");
    Serial.println("========================================");
    Serial.println("Teensy 4.0 ST7789VI + FT5x26 Touch Demo");
    Serial.println("========================================");
    Serial.println("");
    
    // Initialize display FIRST (3-wire SPI mode)
    st7789_config_t display_config = {
        .pin_mosi = TFT_MOSI,      // Pin 11 â†’ SDA (Pin 9)
        .pin_miso = TFT_MISO,      // Pin 12 â†’ SDO (Pin 6, optional)
        .pin_sclk = TFT_SCLK,      // Pin 13 â†’ WRX/SCK (Pin 12)
        .pin_cs = TFT_CS,          // Pin 10 â†’ CSX (Pin 10)
        .pin_dc = TFT_DC,          // Pin 9 â†’ DCX (Pin 11)
        .pin_rst = TFT_RST,        // Pin 8 â†’ RESX (Pin 30)
        .pin_bl = TFT_BL,          // Pin 6 â†’ LED-A (Pin 38)
        .spi_clock_mhz = 30        // 30MHz SPI clock for Teensy 4.0
    };
    
    if (!st7789_init(&display_config)) {
        Serial.println("Display initialization failed!");
        return;
    }
    
    Serial.println("Display initialized successfully");
    
    // Test display with a simple fill
    st7789_fill_screen(ST7789_BLUE);
    delay(1000);
    st7789_fill_screen(ST7789_GREEN);  
    delay(1000);
    st7789_fill_screen(ST7789_RED);
    delay(1000);
    
    Serial.println("Display test complete - ready for LVGL integration");
}

void loop() {
    // Main application loop will go here
    delay(100);
}
