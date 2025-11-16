#ifndef FT6236_H
#define FT6236_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// FT6236 I2C address (7-bit)
#define FT6236_ADDR 0x38

// Maximum touch points supported
#define FT6236_MAX_TOUCHES 2

// Touch point data structure
typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t event;  // 0=down, 1=up, 2=contact
    uint8_t id;     // Touch ID (0-4)
} ft6236_touch_point_t;

// Touch data structure
typedef struct {
    uint8_t touch_count;
    ft6236_touch_point_t points[FT6236_MAX_TOUCHES];
    uint8_t gesture;
} ft6236_touch_t;

// Configuration structure
typedef struct {
    int i2c_port;      // I2C port number (I2C_NUM_0 or I2C_NUM_1)
    int pin_sda;       // SDA pin
    int pin_scl;       // SCL pin
    int pin_int;       // Interrupt pin (-1 if not used)
    uint32_t i2c_freq; // I2C frequency in Hz (e.g., 400000)
} ft6236_config_t;

/**
 * @brief Initialize FT6236 touch controller
 * @param config I2C and pin configuration
 * @return true on success, false on failure
 */
bool ft6236_init(const ft6236_config_t *config);

/**
 * @brief Read touch data from FT6236
 * @param touch_data Pointer to touch data structure to fill
 * @return true if touch data is valid, false otherwise
 */
bool ft6236_read_touch(ft6236_touch_t *touch_data);

/**
 * @brief Check if touch screen is currently touched
 * @return true if touched, false otherwise
 */
bool ft6236_is_touched(void);

/**
 * @brief Read a single register from FT6236
 * @param reg Register address
 * @param data Pointer to store read data
 * @return true on success, false on failure
 */
bool ft6236_read_register(uint8_t reg, uint8_t *data);

/**
 * @brief Write a single register to FT6236
 * @param reg Register address
 * @param data Data to write
 * @return true on success, false on failure
 */
bool ft6236_write_register(uint8_t reg, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif // FT6236_H
