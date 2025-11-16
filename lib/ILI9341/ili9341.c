#include "ili9341.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "ILI9341";

static spi_device_handle_t spi_handle = NULL;
static ili9341_config_t display_config;

// ILI9341 commands
#define ILI9341_SWRESET   0x01
#define ILI9341_SLPOUT    0x11
#define ILI9341_DISPOFF   0x28
#define ILI9341_DISPON    0x29
#define ILI9341_CASET     0x2A
#define ILI9341_PASET     0x2B
#define ILI9341_RAMWR     0x2C
#define ILI9341_MADCTL    0x36
#define ILI9341_PIXFMT    0x3A
#define ILI9341_SLPIN     0x10

static inline void gpio_set(int pin, int level) {
    gpio_set_level(pin, level);
}

static void ili9341_send_cmd(uint8_t cmd) {
    gpio_set(display_config.pin_dc, 0); // Command mode
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
        .flags = SPI_TRANS_USE_TXDATA
    };
    t.tx_data[0] = cmd;
    spi_device_polling_transmit(spi_handle, &t);
}

static void ili9341_send_data(const uint8_t *data, size_t len) {
    if (len == 0) return;
    gpio_set(display_config.pin_dc, 1); // Data mode
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data
    };
    if (len <= 4) {
        t.flags = SPI_TRANS_USE_TXDATA;
        memcpy(t.tx_data, data, len);
    }
    spi_device_polling_transmit(spi_handle, &t);
}

static void ili9341_send_u8(uint8_t data) {
    ili9341_send_data(&data, 1);
}

bool ili9341_init(const ili9341_config_t *config) {
    if (!config) return false;
    
    memcpy(&display_config, config, sizeof(ili9341_config_t));
    
    // Configure DC pin (always required)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << config->pin_dc),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    // Add RST pin if configured
    if (config->pin_rst >= 0) {
        io_conf.pin_bit_mask |= (1ULL << config->pin_rst);
    }
    
    // Add backlight pin if configured
    if (config->pin_bl >= 0) {
        io_conf.pin_bit_mask |= (1ULL << config->pin_bl);
    }
    
    gpio_config(&io_conf);
    
    // Turn backlight off initially
    if (config->pin_bl >= 0) {
        gpio_set(config->pin_bl, 0);
    }
    
    // Initialize SPI bus
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = config->pin_mosi,
        .miso_io_num = config->pin_miso,
        .sclk_io_num = config->pin_sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = ILI9341_WIDTH * ILI9341_HEIGHT * 2
    };
    
    esp_err_t ret = spi_bus_initialize(config->spi_host, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    // Add device to SPI bus
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = config->spi_clock_mhz * 1000000,
        .mode = 0,
        .spics_io_num = config->pin_cs,
        .queue_size = 16,  // Increased queue for better pipelining
        .flags = SPI_DEVICE_NO_DUMMY,  // No dummy bits for faster transfers
        .pre_cb = NULL
    };
    
    ret = spi_bus_add_device(config->spi_host, &dev_cfg, &spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI device add failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    // Hardware reset (if RST pin is configured)
    if (config->pin_rst >= 0) {
        gpio_set(config->pin_rst, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
        gpio_set(config->pin_rst, 1);
        vTaskDelay(pdMS_TO_TICKS(120));
    } else {
        // If no RST pin, wait for power-on reset to complete
        vTaskDelay(pdMS_TO_TICKS(150));
    }
    
    // Initialization sequence (based on ER-TFTM024-3 4-wire SPI example)
    ili9341_send_cmd(ILI9341_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Power control A
    ili9341_send_cmd(0xCF);
    uint8_t cf[] = {0x00, 0xC3, 0x30};
    ili9341_send_data(cf, 3);
    
    // Power control B
    ili9341_send_cmd(0xED);
    uint8_t ed[] = {0x64, 0x03, 0x12, 0x81};
    ili9341_send_data(ed, 4);
    
    // Driver timing control A
    ili9341_send_cmd(0xE8);
    uint8_t e8[] = {0x85, 0x10, 0x79};
    ili9341_send_data(e8, 3);
    
    // Driver timing control B
    ili9341_send_cmd(0xCB);
    uint8_t cb[] = {0x39, 0x2C, 0x00, 0x34, 0x02};
    ili9341_send_data(cb, 5);
    
    // Power on sequence control
    ili9341_send_cmd(0xF7);
    ili9341_send_u8(0x20);
    
    // Pump ratio control
    ili9341_send_cmd(0xEA);
    uint8_t ea[] = {0x00, 0x00};
    ili9341_send_data(ea, 2);
    
    // Power Control 1
    ili9341_send_cmd(0xC0);
    ili9341_send_u8(0x22);
    
    // Power Control 2
    ili9341_send_cmd(0xC1);
    ili9341_send_u8(0x11);
    
    // VCOM Control 1
    ili9341_send_cmd(0xC5);
    uint8_t c5[] = {0x3D, 0x20};
    ili9341_send_data(c5, 2);
    
    // VCOM Control 2
    ili9341_send_cmd(0xC7);
    ili9341_send_u8(0xAA);
    
    // Memory Access Control (rotation/orientation)
    ili9341_send_cmd(ILI9341_MADCTL);
    ili9341_send_u8(0xC0); // MY=1, MX=1, MV=0, ML=0, BGR=0, MH=0 - Landscape normal with RGB
    
    // Pixel Format Set (16-bit/pixel)
    ili9341_send_cmd(ILI9341_PIXFMT);
    ili9341_send_u8(0x55);
    
    // Frame Rate Control
    ili9341_send_cmd(0xB1);
    uint8_t b1[] = {0x00, 0x13};
    ili9341_send_data(b1, 2);
    
    // Display Function Control
    ili9341_send_cmd(0xB6);
    uint8_t b6[] = {0x0A, 0xA2};
    ili9341_send_data(b6, 2);
    
    // Interface Control
    ili9341_send_cmd(0xF6);
    uint8_t f6[] = {0x01, 0x30};
    ili9341_send_data(f6, 2);
    
    // Disable 3Gamma Function
    ili9341_send_cmd(0xF2);
    ili9341_send_u8(0x00);
    
    // Gamma curve selected
    ili9341_send_cmd(0x26);
    ili9341_send_u8(0x01);
    
    // Positive Gamma Correction
    ili9341_send_cmd(0xE0);
    uint8_t e0[] = {0x0F, 0x3F, 0x2F, 0x0C, 0x10, 0x0A, 0x53, 0xD5,
                    0x40, 0x0A, 0x13, 0x03, 0x08, 0x03, 0x00};
    ili9341_send_data(e0, 15);
    
    // Negative Gamma Correction
    ili9341_send_cmd(0xE1);
    uint8_t e1[] = {0x00, 0x00, 0x10, 0x03, 0x0F, 0x05, 0x2C, 0xA2,
                    0x3F, 0x05, 0x0E, 0x0C, 0x37, 0x3C, 0x0F};
    ili9341_send_data(e1, 15);
    
    // Sleep Out (already sent above, but here again as per original sequence)
    ili9341_send_cmd(ILI9341_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Display Invert OFF (normal colors)
    ili9341_send_cmd(0x20);  // ILI9341_INVOFF
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Display ON
    ili9341_send_cmd(ILI9341_DISPON);
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Turn on backlight (if configured)
    if (config->pin_bl >= 0) {
        gpio_set(config->pin_bl, 1);
        ESP_LOGI(TAG, "Backlight enabled on GPIO %d", config->pin_bl);
    }
    
    ESP_LOGI(TAG, "ILI9341 initialized successfully");
    return true;
}

void ili9341_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data[4];
    
    // Column Address Set
    ili9341_send_cmd(ILI9341_CASET);
    data[0] = x0 >> 8;
    data[1] = x0 & 0xFF;
    data[2] = x1 >> 8;
    data[3] = x1 & 0xFF;
    ili9341_send_data(data, 4);
    
    // Page Address Set
    ili9341_send_cmd(ILI9341_PASET);
    data[0] = y0 >> 8;
    data[1] = y0 & 0xFF;
    data[2] = y1 >> 8;
    data[3] = y1 & 0xFF;
    ili9341_send_data(data, 4);
    
    // Memory Write
    ili9341_send_cmd(ILI9341_RAMWR);
}

void ili9341_write_color(uint16_t color) {
    uint8_t data[2] = {color >> 8, color & 0xFF};
    ili9341_send_data(data, 2);
}

// Fast batch write for display flush - writes raw buffer directly
void ili9341_write_pixels(const uint16_t* pixels, uint32_t length) {
    if (length == 0) return;
    
    gpio_set(display_config.pin_dc, 1); // Data mode
    
    // Use larger chunks with DMA for better throughput
    const uint32_t MAX_CHUNK = 16384; // 16384 pixels * 2 bytes = 32KB per transfer
    uint32_t remaining = length;
    const uint16_t* ptr = pixels;
    
    while (remaining > 0) {
        uint32_t chunk = (remaining > MAX_CHUNK) ? MAX_CHUNK : remaining;
        
        spi_transaction_t t = {
            .flags = 0,
            .length = chunk * 16,  // bits
            .tx_buffer = ptr,
            .rx_buffer = NULL
        };
        
        // Use queued transmit for better pipelining
        spi_device_queue_trans(spi_handle, &t, portMAX_DELAY);
        
        // Get result immediately (still allows DMA to work)
        spi_transaction_t* rtrans;
        spi_device_get_trans_result(spi_handle, &rtrans, portMAX_DELAY);
        
        ptr += chunk;
        remaining -= chunk;
    }
}

void ili9341_write_colors(const uint16_t* colors, uint32_t length) {
    if (length == 0) return;
    
    gpio_set(display_config.pin_dc, 1); // Data mode
    
    // Use structure with designated initializers to ensure zero-init
    spi_transaction_t t = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = length * 16,  // 16 bits per pixel
        .rxlength = 0,
        .user = NULL,
        .tx_buffer = colors,
        .rx_buffer = NULL
    };
    
    spi_device_polling_transmit(spi_handle, &t);
}

void ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT) return;
    
    ili9341_set_addr_window(x, y, x, y);
    ili9341_write_color(color);
}

void ili9341_fill_screen(uint16_t color) {
    ili9341_fill_rect(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

void ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT) return;
    if (x + w > ILI9341_WIDTH) w = ILI9341_WIDTH - x;
    if (y + h > ILI9341_HEIGHT) h = ILI9341_HEIGHT - y;
    
    ili9341_set_addr_window(x, y, x + w - 1, y + h - 1);
    
    // Prepare line buffer
    uint8_t line_buf[ILI9341_WIDTH * 2];
    for (int i = 0; i < w; i++) {
        line_buf[i * 2] = color >> 8;
        line_buf[i * 2 + 1] = color & 0xFF;
    }
    
    gpio_set(display_config.pin_dc, 1); // Data mode
    
    // Send each line
    for (int line = 0; line < h; line++) {
        spi_transaction_t t = {
            .length = w * 16,
            .tx_buffer = line_buf
        };
        spi_device_polling_transmit(spi_handle, &t);
    }
}

void ili9341_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    // Draw four lines for the rectangle outline
    // Top line
    for (uint16_t i = 0; i < w; i++) {
        ili9341_draw_pixel(x + i, y, color);
    }
    // Bottom line
    for (uint16_t i = 0; i < w; i++) {
        ili9341_draw_pixel(x + i, y + h - 1, color);
    }
    // Left line
    for (uint16_t i = 1; i < h - 1; i++) {
        ili9341_draw_pixel(x, y + i, color);
    }
    // Right line
    for (uint16_t i = 1; i < h - 1; i++) {
        ili9341_draw_pixel(x + w - 1, y + i, color);
    }
}

void ili9341_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    while (1) {
        ili9341_draw_pixel(x0, y0, color);
        
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

void ili9341_set_backlight(uint8_t brightness) {
    if (display_config.pin_bl < 0) return; // No backlight pin configured
    
    if (brightness > 100) brightness = 100;
    
    if (brightness == 0) {
        gpio_set(display_config.pin_bl, 0);
    } else if (brightness == 100) {
        gpio_set(display_config.pin_bl, 1);
    } else {
        // TODO: Implement PWM control for variable brightness
        gpio_set(display_config.pin_bl, 1);
    }
}

void ili9341_sleep(void) {
    ili9341_send_cmd(ILI9341_DISPOFF);
    vTaskDelay(pdMS_TO_TICKS(20));
    ili9341_send_cmd(ILI9341_SLPIN);
    vTaskDelay(pdMS_TO_TICKS(120));
    if (display_config.pin_bl >= 0) {
        gpio_set(display_config.pin_bl, 0);
    }
}

void ili9341_wake(void) {
    if (display_config.pin_bl >= 0) {
        gpio_set(display_config.pin_bl, 1);
    }
    ili9341_send_cmd(ILI9341_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));
    ili9341_send_cmd(ILI9341_DISPON);
    vTaskDelay(pdMS_TO_TICKS(20));
}
