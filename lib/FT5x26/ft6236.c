#include "ft6236.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "FT6236";

static ft6236_config_t touch_config;
static bool initialized = false;

// FT6236 Register addresses
#define FT6236_REG_MODE         0x00
#define FT6236_REG_GEST_ID      0x01
#define FT6236_REG_TD_STATUS    0x02
#define FT6236_REG_TOUCH1_XH    0x03
#define FT6236_REG_TOUCH1_XL    0x04
#define FT6236_REG_TOUCH1_YH    0x05
#define FT6236_REG_TOUCH1_YL    0x06
#define FT6236_REG_TOUCH2_XH    0x09
#define FT6236_REG_TOUCH2_XL    0x0A
#define FT6236_REG_TOUCH2_YH    0x0B
#define FT6236_REG_TOUCH2_YL    0x0C

// Touch events
#define FT6236_EVENT_PRESS_DOWN   0
#define FT6236_EVENT_LIFT_UP      1
#define FT6236_EVENT_CONTACT      2
#define FT6236_EVENT_NO_EVENT     3

static esp_err_t ft6236_i2c_read(uint8_t reg, uint8_t *data, size_t len) {
    if (!initialized || !data || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    // Start + Write device address + register address
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (FT6236_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    
    // Repeated start + Read data
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (FT6236_ADDR << 1) | I2C_MASTER_READ, true);
    
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(touch_config.i2c_port, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

static esp_err_t ft6236_i2c_write(uint8_t reg, uint8_t data) {
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (FT6236_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(touch_config.i2c_port, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

bool ft6236_init(const ft6236_config_t *config) {
    if (!config) {
        ESP_LOGE(TAG, "Invalid configuration");
        return false;
    }
    
    memcpy(&touch_config, config, sizeof(ft6236_config_t));
    
    // Configure I2C
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = config->pin_sda,
        .scl_io_num = config->pin_scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = config->i2c_freq
    };
    
    esp_err_t ret = i2c_param_config(config->i2c_port, &i2c_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C param config failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    ret = i2c_driver_install(config->i2c_port, I2C_MODE_MASTER, 0, 0, 0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
        return false;
    }
    
    // Configure interrupt pin if provided
    if (config->pin_int >= 0) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << config->pin_int),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_NEGEDGE
        };
        gpio_config(&io_conf);
    }
    
    initialized = true;
    
    // Small delay for chip to be ready
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Verify communication by reading chip ID or status
    uint8_t test_read;
    ret = ft6236_i2c_read(FT6236_REG_TD_STATUS, &test_read, 1);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "FT6236 communication test warning: %s", esp_err_to_name(ret));
        // Don't fail - some chips may not respond until first touch
    }
    
    ESP_LOGI(TAG, "FT6236 initialized successfully");
    return true;
}

bool ft6236_read_touch(ft6236_touch_t *touch_data) {
    if (!initialized || !touch_data) {
        return false;
    }
    
    memset(touch_data, 0, sizeof(ft6236_touch_t));
    
    // Read 13 bytes starting from register 0x00
    uint8_t buf[13];
    esp_err_t ret = ft6236_i2c_read(FT6236_REG_MODE, buf, 13);
    if (ret != ESP_OK) {
        return false;
    }
    
    // Extract gesture ID
    touch_data->gesture = buf[FT6236_REG_GEST_ID];
    
    // Extract number of touch points
    uint8_t touch_count = buf[FT6236_REG_TD_STATUS] & 0x0F;
    if (touch_count > FT6236_MAX_TOUCHES) {
        touch_count = FT6236_MAX_TOUCHES;
    }
    touch_data->touch_count = touch_count;
    
    if (touch_count == 0) {
        return false; // No touch detected
    }
    
    // Parse first touch point
    if (touch_count >= 1) {
        touch_data->points[0].event = (buf[FT6236_REG_TOUCH1_XH] >> 6) & 0x03;
        touch_data->points[0].x = ((buf[FT6236_REG_TOUCH1_XH] & 0x0F) << 8) | buf[FT6236_REG_TOUCH1_XL];
        touch_data->points[0].y = ((buf[FT6236_REG_TOUCH1_YH] & 0x0F) << 8) | buf[FT6236_REG_TOUCH1_YL];
        touch_data->points[0].id = (buf[FT6236_REG_TOUCH1_YH] >> 4) & 0x0F;
    }
    
    // Parse second touch point
    if (touch_count >= 2) {
        touch_data->points[1].event = (buf[FT6236_REG_TOUCH2_XH] >> 6) & 0x03;
        touch_data->points[1].x = ((buf[FT6236_REG_TOUCH2_XH] & 0x0F) << 8) | buf[FT6236_REG_TOUCH2_XL];
        touch_data->points[1].y = ((buf[FT6236_REG_TOUCH2_YH] & 0x0F) << 8) | buf[FT6236_REG_TOUCH2_YL];
        touch_data->points[1].id = (buf[FT6236_REG_TOUCH2_YH] >> 4) & 0x0F;
    }
    
    return true;
}

bool ft6236_is_touched(void) {
    if (!initialized) {
        return false;
    }
    
    uint8_t status;
    esp_err_t ret = ft6236_i2c_read(FT6236_REG_TD_STATUS, &status, 1);
    if (ret != ESP_OK) {
        return false;
    }
    
    uint8_t touch_count = status & 0x0F;
    return (touch_count > 0);
}

bool ft6236_read_register(uint8_t reg, uint8_t *data) {
    if (!data) {
        return false;
    }
    
    esp_err_t ret = ft6236_i2c_read(reg, data, 1);
    return (ret == ESP_OK);
}

bool ft6236_write_register(uint8_t reg, uint8_t data) {
    esp_err_t ret = ft6236_i2c_write(reg, data);
    return (ret == ESP_OK);
}
