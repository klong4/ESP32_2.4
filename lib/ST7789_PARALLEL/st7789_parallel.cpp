#include "st7789_parallel.h"
#include <Arduino.h>

static st7789_parallel_config_t display_config;

// Write data to 8-bit parallel bus
static void st7789_parallel_write_byte(uint8_t data) {
    // Set data pins - pin_data[0]=DB0 (LSB), pin_data[7]=DB7 (MSB)
    for (int i = 0; i < 8; i++) {
        digitalWrite(display_config.pin_data[i], (data >> i) & 0x01);
    }
    
    // Pulse WR low (write on falling edge) - faster timing
    digitalWrite(display_config.pin_wr, LOW);
    __asm__ volatile("nop\n nop\n nop\n nop\n");  // ~40ns delay
    digitalWrite(display_config.pin_wr, HIGH);
    __asm__ volatile("nop\n nop\n");  // ~20ns delay
}

// Write data to 16-bit parallel bus
static void st7789_parallel_write_word(uint16_t data) {
    // Set data pins (16-bit mode)
    for (int i = 0; i < 16; i++) {
        digitalWrite(display_config.pin_data[i], (data >> i) & 0x01);
    }
    
    // Pulse WR low (write on falling edge) - faster timing
    digitalWrite(display_config.pin_wr, LOW);
    __asm__ volatile("nop\n nop\n nop\n nop\n");  // ~40ns delay
    digitalWrite(display_config.pin_wr, HIGH);
    __asm__ volatile("nop\n nop\n");  // ~20ns delay
}

static void st7789_parallel_send_cmd(uint8_t cmd) {
    digitalWrite(display_config.pin_dc, LOW);  // Command mode
    digitalWrite(display_config.pin_cs, LOW);  // Select chip
    st7789_parallel_write_byte(cmd);
    digitalWrite(display_config.pin_cs, HIGH); // Deselect
}

static void st7789_parallel_send_data(const uint8_t *data, size_t len) {
    if (len == 0) return;
    
    digitalWrite(display_config.pin_dc, HIGH); // Data mode
    digitalWrite(display_config.pin_cs, LOW);  // Select chip
    
    for (size_t i = 0; i < len; i++) {
        st7789_parallel_write_byte(data[i]);
    }
    
    digitalWrite(display_config.pin_cs, HIGH); // Deselect
}

static void st7789_parallel_send_u8(uint8_t data) {
    st7789_parallel_send_data(&data, 1);
}

static void st7789_parallel_write_cmd_u8(uint8_t cmd, uint8_t data) {
    st7789_parallel_send_cmd(cmd);
    st7789_parallel_send_u8(data);
}

bool st7789_parallel_init(const st7789_parallel_config_t *config) {
    if (!config) return false;
    
    display_config = *config;
    
    int num_data_pins = config->use_16bit ? 16 : 8;
    
    // Configure interface mode pins (IM0, IM2)
    // Mode testing showed all combinations work - using IM0=0, IM2=0 (16-bit mode)
    if (config->pin_im0 >= 0) {
        pinMode(config->pin_im0, OUTPUT);
        digitalWrite(config->pin_im0, LOW);  // IM0 = 0
    }
    if (config->pin_im2 >= 0) {
        pinMode(config->pin_im2, OUTPUT);
        digitalWrite(config->pin_im2, LOW);  // IM2 = 0
    }
    
    // Configure data pins
    for (int i = 0; i < num_data_pins; i++) {
        pinMode(config->pin_data[i], OUTPUT);
        digitalWrite(config->pin_data[i], LOW);
    }
    
    // Configure control pins
    pinMode(config->pin_dc, OUTPUT);
    pinMode(config->pin_cs, OUTPUT);
    pinMode(config->pin_wr, OUTPUT);
    pinMode(config->pin_rd, OUTPUT);
    
    digitalWrite(config->pin_cs, HIGH);  // Deselect
    digitalWrite(config->pin_dc, HIGH);  // Data mode default
    digitalWrite(config->pin_wr, HIGH);  // Write inactive
    digitalWrite(config->pin_rd, HIGH);  // Read inactive
    
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
    
    Serial.println("[ST7789] Starting initialization sequence...");
    
    // ST7789VI initialization sequence
    st7789_parallel_send_cmd(0x28); // Display OFF
    delay(10);
    
    st7789_parallel_send_cmd(0x11); // Exit sleep mode
    delay(120);
    
    // Memory Data Access Control - Portrait mode
    st7789_parallel_write_cmd_u8(0x36, 0x88); // MADCTL: 0x88 per NHD sample
    
    // Interface Pixel Format - RGB565 (16-bit for 16-bit 8080 interface)
    st7789_parallel_write_cmd_u8(0x3A, 0x55); // COLMOD - RGB565 (5-6-5 format, 1 byte per pixel in 16-bit interface)
    
    // Porch settings
    st7789_parallel_send_cmd(0xB2); // Porch Setting
    uint8_t porch[5] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
    st7789_parallel_send_data(porch, 5);
    
    // Gate Control
    st7789_parallel_write_cmd_u8(0xB7, 0x35); // GCTRL
    
    // VCOM Setting
    st7789_parallel_write_cmd_u8(0xBB, 0x2B); // VCOMS: 0x2B per Newhaven sample
    
    // LCM Control
    st7789_parallel_write_cmd_u8(0xC0, 0x2C); // LCMCTRL
    
    // VDV and VRH Command Enable
    st7789_parallel_send_cmd(0xC2); // VDVVRHEN
    uint8_t vdv_vrh[2] = {0x01, 0xFF};
    st7789_parallel_send_data(vdv_vrh, 2);
    
    // VRH Set
    st7789_parallel_write_cmd_u8(0xC3, 0x11); // VRHS: 0x11 per Newhaven sample
    
    // VDV Set
    st7789_parallel_write_cmd_u8(0xC4, 0x20); // VDVS
    
    // Frame Rate Control
    st7789_parallel_write_cmd_u8(0xC6, 0x0F); // FRCTRL2: 60Hz
    
    // Power Control 1
    st7789_parallel_send_cmd(0xD0); // PWCTRL1
    uint8_t pwr[2] = {0xA4, 0xA1};
    st7789_parallel_send_data(pwr, 2);
    
    // Positive Voltage Gamma (Newhaven sample values)
    st7789_parallel_send_cmd(0xE0); // PVGAMCTRL
    uint8_t pvgamma[14] = {0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19};
    st7789_parallel_send_data(pvgamma, 14);
    
    // Negative Voltage Gamma (Newhaven sample values)
    st7789_parallel_send_cmd(0xE1); // NVGAMCTRL
    uint8_t nvgamma[14] = {0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D, 0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19};
    st7789_parallel_send_data(nvgamma, 14);
    
    // Set address window to full screen
    st7789_parallel_send_cmd(0x2A); // X address set
    uint8_t x_addr[4] = {0x00, 0x00, 0x00, 0xEF};
    st7789_parallel_send_data(x_addr, 4);
    
    st7789_parallel_send_cmd(0x2B); // Y address set
    uint8_t y_addr[4] = {0x00, 0x00, 0x01, 0x3F};
    st7789_parallel_send_data(y_addr, 4);
    
    delay(10);
    
    // Inversion ON
    st7789_parallel_send_cmd(0x21); // INVON
    
    delay(10);
    
    Serial.println("[ST7789] Initialization complete");
    
    return true;
}

void st7789_parallel_display_on(void) {
    st7789_parallel_send_cmd(0x29); // Display ON
    delay(10);
}

void st7789_parallel_backlight(bool enable) {
    if (display_config.pin_bl >= 0) {
        digitalWrite(display_config.pin_bl, enable ? HIGH : LOW);
    }
}

void st7789_parallel_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    st7789_parallel_send_cmd(0x2A); // Column Address Set
    uint8_t col_data[4] = {
        (uint8_t)((x0 >> 8) & 0xFF), (uint8_t)(x0 & 0xFF),
        (uint8_t)((x1 >> 8) & 0xFF), (uint8_t)(x1 & 0xFF)
    };
    st7789_parallel_send_data(col_data, 4);
    
    st7789_parallel_send_cmd(0x2B); // Row Address Set
    uint8_t row_data[4] = {
        (uint8_t)((y0 >> 8) & 0xFF), (uint8_t)(y0 & 0xFF),
        (uint8_t)((y1 >> 8) & 0xFF), (uint8_t)(y1 & 0xFF)
    };
    st7789_parallel_send_data(row_data, 4);
    
    st7789_parallel_send_cmd(0x2C); // Memory Write
}

void st7789_parallel_write_color(uint16_t color) {
    if (display_config.use_16bit) {
        // 16-bit mode: write as single word
        digitalWrite(display_config.pin_dc, HIGH);
        digitalWrite(display_config.pin_cs, LOW);
        st7789_parallel_write_word(color);
        digitalWrite(display_config.pin_cs, HIGH);
    } else {
        // 8-bit mode: write as two bytes
        uint8_t data[2] = {(uint8_t)((color >> 8) & 0xFF), (uint8_t)(color & 0xFF)};
        st7789_parallel_send_data(data, 2);
    }
}

void st7789_parallel_write_pixels(const uint16_t* pixels, uint32_t length) {
    if (length == 0) return;
    
    digitalWrite(display_config.pin_dc, HIGH);
    digitalWrite(display_config.pin_cs, LOW);
    
    for (uint32_t i = 0; i < length; i++) {
        if (display_config.use_16bit) {
            st7789_parallel_write_word(pixels[i]);
        } else {
            // 8-bit mode RGB565: Send as 2 bytes (HIGH byte, LOW byte)
            st7789_parallel_write_byte((pixels[i] >> 8) & 0xFF);  // HIGH byte first
            st7789_parallel_write_byte(pixels[i] & 0xFF);         // LOW byte second
        }
    }
    
    digitalWrite(display_config.pin_cs, HIGH);
}

void st7789_parallel_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= ST7789_PARALLEL_WIDTH || y >= ST7789_PARALLEL_HEIGHT) return;
    
    st7789_parallel_set_addr_window(x, y, x, y);
    st7789_parallel_write_color(color);
}

void st7789_parallel_fill_screen(uint16_t color) {
    st7789_parallel_fill_rect(0, 0, ST7789_PARALLEL_WIDTH, ST7789_PARALLEL_HEIGHT, color);
}

void st7789_parallel_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= ST7789_PARALLEL_WIDTH || y >= ST7789_PARALLEL_HEIGHT) return;
    
    if (x + w > ST7789_PARALLEL_WIDTH) w = ST7789_PARALLEL_WIDTH - x;
    if (y + h > ST7789_PARALLEL_HEIGHT) h = ST7789_PARALLEL_HEIGHT - y;
    
    st7789_parallel_set_addr_window(x, y, x + w - 1, y + h - 1);
    
    digitalWrite(display_config.pin_dc, HIGH);
    digitalWrite(display_config.pin_cs, LOW);
    
    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        if (display_config.use_16bit) {
            st7789_parallel_write_word(color);
        } else {
            // RGB565: 2 bytes (HIGH byte, LOW byte)
            st7789_parallel_write_byte((color >> 8) & 0xFF);
            st7789_parallel_write_byte(color & 0xFF);
        }
    }
    
    digitalWrite(display_config.pin_cs, HIGH);
}
