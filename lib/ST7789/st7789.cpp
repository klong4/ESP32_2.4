#include "st7789.h"
#include <Arduino.h>
#include <SPI.h>

static st7789_config_t display_config;
static SPIClass *spi_obj = &SPI;

// ST7789VI commands
#define ST7789_SWRESET   0x01  // Software Reset
#define ST7789_SLPOUT    0x11  // Sleep Out
#define ST7789_NORON     0x13  // Normal Display Mode On
#define ST7789_INVOFF    0x20  // Display Inversion Off
#define ST7789_INVON     0x21  // Display Inversion On
#define ST7789_DISPOFF   0x28  // Display Off
#define ST7789_DISPON    0x29  // Display On
#define ST7789_CASET     0x2A  // Column Address Set
#define ST7789_RASET     0x2B  // Row Address Set (same as PASET)
#define ST7789_RAMWR     0x2C  // Memory Write
#define ST7789_MADCTL    0x36  // Memory Data Access Control
#define ST7789_COLMOD    0x3A  // Interface Pixel Format
#define ST7789_SLPIN     0x10  // Sleep In
#define ST7789_PORCTRL   0xB2  // Porch Control
#define ST7789_GCTRL     0xB7  // Gate Control
#define ST7789_VCOMS     0xBB  // VCOM Setting
#define ST7789_LCMCTRL   0xC0  // LCM Control
#define ST7789_VDVVRHEN  0xC2  // VDV and VRH Command Enable
#define ST7789_VRHS      0xC3  // VRH Set
#define ST7789_VDVS      0xC4  // VDV Set
#define ST7789_FRCTRL2   0xC6  // Frame Rate Control in Normal Mode
#define ST7789_PWCTRL1   0xD0  // Power Control 1
#define ST7789_PVGAMCTRL 0xE0  // Positive Voltage Gamma Control
#define ST7789_NVGAMCTRL 0xE1  // Negative Voltage Gamma Control

static void st7789_send_cmd(uint8_t cmd) {
    digitalWrite(display_config.pin_dc, LOW); // Command mode
    digitalWrite(display_config.pin_cs, LOW);
    spi_obj->transfer(cmd);
    digitalWrite(display_config.pin_cs, HIGH);
}

static void st7789_send_data(const uint8_t *data, size_t len) {
    if (len == 0) return;
    digitalWrite(display_config.pin_dc, HIGH); // Data mode
    digitalWrite(display_config.pin_cs, LOW);
    spi_obj->transfer((void*)data, len);
    digitalWrite(display_config.pin_cs, HIGH);
}

static void st7789_send_u8(uint8_t data) {
    st7789_send_data(&data, 1);
}

static void st7789_send_u16(uint16_t data) {
    uint8_t buf[2];
    buf[0] = (data >> 8) & 0xFF;
    buf[1] = data & 0xFF;
    st7789_send_data(buf, 2);
}

bool st7789_init(const st7789_config_t *config) {
    if (!config) return false;
    
    display_config = *config;
    
    // Configure pins
    pinMode(config->pin_dc, OUTPUT);
    pinMode(config->pin_cs, OUTPUT);
    digitalWrite(config->pin_cs, HIGH); // Deselect
    
    if (config->pin_rst >= 0) {
        pinMode(config->pin_rst, OUTPUT);
    }
    
    if (config->pin_bl >= 0) {
        pinMode(config->pin_bl, OUTPUT);
        digitalWrite(config->pin_bl, LOW); // Turn off initially
    }
    
    // Initialize SPI
    spi_obj->begin();
    spi_obj->beginTransaction(SPISettings(config->spi_clock_mhz * 1000000, MSBFIRST, SPI_MODE0));
    
    // Hardware reset
    if (config->pin_rst >= 0) {
        digitalWrite(config->pin_rst, HIGH);
        delay(10);
        digitalWrite(config->pin_rst, LOW);
        delay(20);
        digitalWrite(config->pin_rst, HIGH);
        delay(150);
    } else {
        delay(150);
    }
    
    // ST7789VI initialization sequence
    st7789_send_cmd(ST7789_SWRESET);
    delay(150);
    
    st7789_send_cmd(ST7789_SLPOUT);
    delay(120);
    
    // Memory Data Access Control - Portrait mode
    st7789_send_cmd(ST7789_MADCTL);
    st7789_send_u8(0x00); // RGB order, Portrait
    
    // Interface Pixel Format - 16-bit RGB565
    st7789_send_cmd(ST7789_COLMOD);
    st7789_send_u8(0x05); // 16-bit/pixel
    
    // Porch Control
    st7789_send_cmd(ST7789_PORCTRL);
    uint8_t porch[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
    st7789_send_data(porch, 5);
    
    // Gate Control
    st7789_send_cmd(ST7789_GCTRL);
    st7789_send_u8(0x35);
    
    // VCOM Setting
    st7789_send_cmd(ST7789_VCOMS);
    st7789_send_u8(0x19);
    
    // LCM Control
    st7789_send_cmd(ST7789_LCMCTRL);
    st7789_send_u8(0x2C);
    
    // VDV and VRH Command Enable
    st7789_send_cmd(ST7789_VDVVRHEN);
    st7789_send_u8(0x01);
    
    // VRH Set
    st7789_send_cmd(ST7789_VRHS);
    st7789_send_u8(0x12);
    
    // VDV Set
    st7789_send_cmd(ST7789_VDVS);
    st7789_send_u8(0x20);
    
    // Frame Rate Control - 60Hz
    st7789_send_cmd(ST7789_FRCTRL2);
    st7789_send_u8(0x0F);
    
    // Power Control 1
    st7789_send_cmd(ST7789_PWCTRL1);
    uint8_t pwctrl[] = {0xA4, 0xA1};
    st7789_send_data(pwctrl, 2);
    
    // Positive Voltage Gamma Control
    st7789_send_cmd(ST7789_PVGAMCTRL);
    uint8_t pvgam[] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
    st7789_send_data(pvgam, 14);
    
    // Negative Voltage Gamma Control
    st7789_send_cmd(ST7789_NVGAMCTRL);
    uint8_t nvgam[] = {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};
    st7789_send_data(nvgam, 14);
    
    // Display Inversion On
    st7789_send_cmd(ST7789_INVON);
    
    // Normal Display Mode On
    st7789_send_cmd(ST7789_NORON);
    delay(10);
    
    // Display On
    st7789_send_cmd(ST7789_DISPON);
    delay(120);
    
    // Turn on backlight
    if (config->pin_bl >= 0) {
        digitalWrite(config->pin_bl, HIGH);
    }
    
    return true;
}

void st7789_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // Column address set
    st7789_send_cmd(ST7789_CASET);
    st7789_send_u16(x0);
    st7789_send_u16(x1);
    
    // Row address set
    st7789_send_cmd(ST7789_RASET);
    st7789_send_u16(y0);
    st7789_send_u16(y1);
    
    // Write to RAM
    st7789_send_cmd(ST7789_RAMWR);
}

void st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;
    
    st7789_set_addr_window(x, y, x, y);
    st7789_send_u16(color);
}

void st7789_fill_screen(uint16_t color) {
    st7789_fill_rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

void st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;
    if (x + w > ST7789_WIDTH) w = ST7789_WIDTH - x;
    if (y + h > ST7789_HEIGHT) h = ST7789_HEIGHT - y;
    
    st7789_set_addr_window(x, y, x + w - 1, y + h - 1);
    
    uint8_t color_hi = (color >> 8) & 0xFF;
    uint8_t color_lo = color & 0xFF;
    
    digitalWrite(display_config.pin_dc, HIGH); // Data mode
    digitalWrite(display_config.pin_cs, LOW);
    
    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        spi_obj->transfer(color_hi);
        spi_obj->transfer(color_lo);
    }
    
    digitalWrite(display_config.pin_cs, HIGH);
}

void st7789_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    // Top
    st7789_fill_rect(x, y, w, 1, color);
    // Bottom
    st7789_fill_rect(x, y + h - 1, w, 1, color);
    // Left
    st7789_fill_rect(x, y, 1, h, color);
    // Right
    st7789_fill_rect(x + w - 1, y, 1, h, color);
}

void st7789_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    
    while (true) {
        st7789_draw_pixel(x0, y0, color);
        
        if (x0 == x1 && y0 == y1) break;
        
        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void st7789_write_color(uint16_t color) {
    st7789_send_u16(color);
}

void st7789_write_colors(const uint16_t* colors, uint32_t length) {
    digitalWrite(display_config.pin_dc, HIGH); // Data mode
    digitalWrite(display_config.pin_cs, LOW);
    
    for (uint32_t i = 0; i < length; i++) {
        spi_obj->transfer((colors[i] >> 8) & 0xFF);
        spi_obj->transfer(colors[i] & 0xFF);
    }
    
    digitalWrite(display_config.pin_cs, HIGH);
}

void st7789_set_backlight(uint8_t brightness) {
    if (display_config.pin_bl >= 0) {
        analogWrite(display_config.pin_bl, brightness);
    }
}

void st7789_sleep(void) {
    st7789_send_cmd(ST7789_SLPIN);
    delay(120);
}

void st7789_wake(void) {
    st7789_send_cmd(ST7789_SLPOUT);
    delay(120);
}
