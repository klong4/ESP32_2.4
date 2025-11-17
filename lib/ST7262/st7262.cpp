#include "st7262.h"
#include <Arduino.h>

static st7262_config_t display_config;

// Write a 24-bit pixel to parallel bus
static void st7262_write_pixel_parallel(uint32_t color) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    
    // Set RGB data pins
    for (int i = 0; i < 8; i++) {
        digitalWrite(display_config.pin_r[i], (r >> i) & 0x01);
        digitalWrite(display_config.pin_g[i], (g >> i) & 0x01);
        digitalWrite(display_config.pin_b[i], (b >> i) & 0x01);
    }
    
    // Pulse pixel clock
    digitalWrite(display_config.pin_pclk, HIGH);
    delayMicroseconds(1);
    digitalWrite(display_config.pin_pclk, LOW);
    delayMicroseconds(1);
}

bool st7262_init(const st7262_config_t *config) {
    if (!config) return false;
    
    display_config = *config;
    
    // Configure RGB data pins as outputs
    for (int i = 0; i < 8; i++) {
        pinMode(config->pin_r[i], OUTPUT);
        pinMode(config->pin_g[i], OUTPUT);
        pinMode(config->pin_b[i], OUTPUT);
        digitalWrite(config->pin_r[i], LOW);
        digitalWrite(config->pin_g[i], LOW);
        digitalWrite(config->pin_b[i], LOW);
    }
    
    // Configure control pins
    pinMode(config->pin_de, OUTPUT);
    pinMode(config->pin_vsync, OUTPUT);
    pinMode(config->pin_hsync, OUTPUT);
    pinMode(config->pin_pclk, OUTPUT);
    
    digitalWrite(config->pin_de, LOW);
    digitalWrite(config->pin_vsync, HIGH);
    digitalWrite(config->pin_hsync, HIGH);
    digitalWrite(config->pin_pclk, LOW);
    
    if (config->pin_rst >= 0) {
        pinMode(config->pin_rst, OUTPUT);
        digitalWrite(config->pin_rst, LOW);
        delay(100);
        digitalWrite(config->pin_rst, HIGH);
        delay(100);
    }
    
    if (config->pin_bl >= 0) {
        pinMode(config->pin_bl, OUTPUT);
        digitalWrite(config->pin_bl, HIGH); // Turn on backlight
    }
    
    delay(120);
    
    return true;
}

void st7262_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // For parallel RGB interface, addressing is typically handled by
    // the display controller automatically via HSYNC/VSYNC
    // This is a placeholder - actual implementation depends on controller
}

void st7262_draw_pixel(uint16_t x, uint16_t y, uint32_t color) {
    if (x >= ST7262_WIDTH || y >= ST7262_HEIGHT) return;
    
    // For direct pixel access, you'd need to calculate position and write
    // This is simplified - actual implementation needs proper sync signals
    st7262_write_pixel_parallel(color);
}

void st7262_fill_screen(uint32_t color) {
    st7262_fill_rect(0, 0, ST7262_WIDTH, ST7262_HEIGHT, color);
}

void st7262_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
    if (x >= ST7262_WIDTH || y >= ST7262_HEIGHT) return;
    
    // Clip to screen bounds
    if (x + w > ST7262_WIDTH) w = ST7262_WIDTH - x;
    if (y + h > ST7262_HEIGHT) h = ST7262_HEIGHT - y;
    
    digitalWrite(display_config.pin_de, HIGH); // Enable data
    
    for (uint16_t row = 0; row < h; row++) {
        // HSYNC pulse at start of line
        digitalWrite(display_config.pin_hsync, LOW);
        delayMicroseconds(2);
        digitalWrite(display_config.pin_hsync, HIGH);
        delayMicroseconds(2);
        
        for (uint16_t col = 0; col < w; col++) {
            st7262_write_pixel_parallel(color);
        }
        
        if (row == 0) {
            // VSYNC pulse at start of frame
            digitalWrite(display_config.pin_vsync, LOW);
            delayMicroseconds(2);
            digitalWrite(display_config.pin_vsync, HIGH);
            delayMicroseconds(2);
        }
    }
    
    digitalWrite(display_config.pin_de, LOW); // Disable data
}

void st7262_write_color(uint32_t color) {
    st7262_write_pixel_parallel(color);
}

void st7262_backlight(bool enable) {
    if (display_config.pin_bl >= 0) {
        digitalWrite(display_config.pin_bl, enable ? HIGH : LOW);
    }
}
