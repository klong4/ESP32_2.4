#include "ft6236.h"
#include <Wire.h>
#include <Arduino.h>

static ft5x26_config_t touch_config;
static bool initialized = false;

// FT5x26 Register addresses
#define FT5X26_REG_MODE         0x00
#define FT5X26_REG_GEST_ID      0x01
#define FT5X26_REG_TD_STATUS    0x02
#define FT5X26_REG_TOUCH1_XH    0x03
#define FT5X26_REG_TOUCH1_XL    0x04
#define FT5X26_REG_TOUCH1_YH    0x05
#define FT5X26_REG_TOUCH1_YL    0x06
#define FT5X26_REG_TOUCH2_XH    0x09
#define FT5X26_REG_TOUCH2_XL    0x0A
#define FT5X26_REG_TOUCH2_YH    0x0B
#define FT5X26_REG_TOUCH2_YL    0x0C

// Touch events
#define FT5X26_EVENT_PRESS_DOWN   0
#define FT5X26_EVENT_LIFT_UP      1
#define FT5X26_EVENT_CONTACT      2
#define FT5X26_EVENT_NO_EVENT     3

static bool ft5x26_i2c_read(uint8_t reg, uint8_t *data, size_t len) {
    if (!initialized || !data || len == 0) {
        return false;
    }
    
    Wire.beginTransmission(FT5X26_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) { // false = send restart, don't release bus
        return false;
    }
    
    size_t received = Wire.requestFrom((uint8_t)FT5X26_ADDR, (uint8_t)len);
    if (received != len) {
        return false;
    }
    
    for (size_t i = 0; i < len; i++) {
        data[i] = Wire.read();
    }
    
    return true;
}

static bool ft5x26_i2c_write(uint8_t reg, uint8_t data) {
    if (!initialized) {
        return false;
    }
    
    Wire.beginTransmission(FT5X26_ADDR);
    Wire.write(reg);
    Wire.write(data);
    return (Wire.endTransmission() == 0);
}

bool ft5x26_init(const ft5x26_config_t *config) {
    if (!config) {
        return false;
    }
    
    memcpy(&touch_config, config, sizeof(ft5x26_config_t));
    
    // Initialize I2C with Wire library (uses pins 18=SDA, 19=SCL on Teensy 4.0)
    Wire.begin();
    Wire.setClock(400000); // 400kHz fast mode
    
    // Configure reset pin if provided
    if (config->pin_rst >= 0) {
        pinMode(config->pin_rst, OUTPUT);
        digitalWrite(config->pin_rst, LOW);
        delay(10);
        digitalWrite(config->pin_rst, HIGH);
        delay(50);
    }
    
    // Configure interrupt pin if provided
    if (config->pin_int >= 0) {
        pinMode(config->pin_int, INPUT_PULLUP);
    }
    
    initialized = true;
    
    // Small delay for chip to be ready
    delay(50);
    
    // Verify communication by reading chip ID or status
    uint8_t test_read;
    if (!ft5x26_i2c_read(FT5X26_REG_TD_STATUS, &test_read, 1)) {
        Serial.println("FT5x26: Warning - communication test failed (may be OK until first touch)");
    }
    
    Serial.println("FT5x26: Initialized successfully");
    return true;
}

bool ft5x26_read_touch(ft5x26_touch_t *touch_data) {
    if (!initialized || !touch_data) {
        return false;
    }
    
    memset(touch_data, 0, sizeof(ft5x26_touch_t));
    
    // Read 13 bytes starting from register 0x00
    uint8_t buf[13];
    if (!ft5x26_i2c_read(FT5X26_REG_MODE, buf, 13)) {
        return false;
    }
    
    // Extract gesture ID
    touch_data->gesture = buf[FT5X26_REG_GEST_ID];
    
    // Extract number of touch points
    uint8_t touch_count = buf[FT5X26_REG_TD_STATUS] & 0x0F;
    if (touch_count > FT5X26_MAX_TOUCHES) {
        touch_count = FT5X26_MAX_TOUCHES;
    }
    touch_data->touch_count = touch_count;
    
    if (touch_count == 0) {
        return false; // No touch detected
    }
    
    // Parse first touch point
    if (touch_count >= 1) {
        touch_data->points[0].event = (buf[FT5X26_REG_TOUCH1_XH] >> 6) & 0x03;
        touch_data->points[0].x = ((buf[FT5X26_REG_TOUCH1_XH] & 0x0F) << 8) | buf[FT5X26_REG_TOUCH1_XL];
        touch_data->points[0].y = ((buf[FT5X26_REG_TOUCH1_YH] & 0x0F) << 8) | buf[FT5X26_REG_TOUCH1_YL];
        touch_data->points[0].id = (buf[FT5X26_REG_TOUCH1_YH] >> 4) & 0x0F;
    }
    
    // Parse second touch point
    if (touch_count >= 2) {
        touch_data->points[1].event = (buf[FT5X26_REG_TOUCH2_XH] >> 6) & 0x03;
        touch_data->points[1].x = ((buf[FT5X26_REG_TOUCH2_XH] & 0x0F) << 8) | buf[FT5X26_REG_TOUCH2_XL];
        touch_data->points[1].y = ((buf[FT5X26_REG_TOUCH2_YH] & 0x0F) << 8) | buf[FT5X26_REG_TOUCH2_YL];
        touch_data->points[1].id = (buf[FT5X26_REG_TOUCH2_YH] >> 4) & 0x0F;
    }
    
    return true;
}

bool ft5x26_is_touched(void) {
    if (!initialized) {
        return false;
    }
    
    uint8_t status;
    if (!ft5x26_i2c_read(FT5X26_REG_TD_STATUS, &status, 1)) {
        return false;
    }
    
    uint8_t touch_count = status & 0x0F;
    return (touch_count > 0);
}

bool ft5x26_read_register(uint8_t reg, uint8_t *data) {
    if (!data) {
        return false;
    }
    
    return ft5x26_i2c_read(reg, data, 1);
}

bool ft5x26_write_register(uint8_t reg, uint8_t data) {
    return ft5x26_i2c_write(reg, data);
}
