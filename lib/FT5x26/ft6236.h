#ifndef FT5X26_H
#define FT5X26_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// FT5x26 I2C address (7-bit)
#define FT5X26_ADDR 0x38

// Maximum touch points supported
#define FT5X26_MAX_TOUCHES 2

// Touch point data structure
typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t event;  // 0=down, 1=up, 2=contact
    uint8_t id;     // Touch ID (0-4)
} ft5x26_touch_point_t;

// Touch data structure
typedef struct {
    uint8_t touch_count;
    ft5x26_touch_point_t points[FT5X26_MAX_TOUCHES];
    uint8_t gesture;
} ft5x26_touch_t;

// Configuration structure
typedef struct {
    int pin_sda;       // SDA pin
    int pin_scl;       // SCL pin
    int pin_int;       // Interrupt pin (-1 if not used)
    int pin_rst;       // Reset pin (-1 if not used)
} ft5x26_config_t;

/**
 * @brief Initialize FT5x26 touch controller
 * @param config I2C and pin configuration
 * @return true on success, false on failure
 */
bool ft5x26_init(const ft5x26_config_t *config);

/**
 * @brief Read touch data from FT5x26
 * @param touch_data Pointer to touch data structure to fill
 * @return true if touch data is valid, false otherwise
 */
bool ft5x26_read_touch(ft5x26_touch_t *touch_data);

/**
 * @brief Check if touch screen is currently touched
 * @return true if touched, false otherwise
 */
bool ft5x26_is_touched(void);

/**
 * @brief Read a single register from FT5x26
 * @param reg Register address
 * @param data Pointer to store read data
 * @return true on success, false on failure
 */
bool ft5x26_read_register(uint8_t reg, uint8_t *data);

/**
 * @brief Write a single register to FT5x26
 * @param reg Register address
 * @param data Data to write
 * @return true on success, false on failure
 */
bool ft5x26_write_register(uint8_t reg, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif // FT5X26_H
