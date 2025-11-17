/**
 * @file lvgl_port.h
 * LVGL porting layer for ILI9341 + FT6236
 */

#ifndef LVGL_PORT_H
#define LVGL_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * Initialize LVGL with display and touch drivers
 * @return true if successful
 */
bool lvgl_port_init(void);

/**
 * Task handler for LVGL - call periodically
 */
void lvgl_port_task_handler(void);

/**
 * Global flush counter for debugging
 */
extern volatile uint32_t lvgl_flush_count;

#ifdef __cplusplus
}
#endif

#endif /* LVGL_PORT_H */
