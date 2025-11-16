/**
 * @file lvgl_port.c
 * LVGL porting layer for ILI9341 + FT6236
 */

#include "lvgl_port.h"
#include "lvgl.h"
#include "../ILI9341/ili9341.h"
#include "../FT6236/ft6236.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "LVGL_PORT";

/* Display buffer */
#define DISP_BUF_SIZE (ILI9341_WIDTH * 80)  // 80 lines for faster scrolling
static lv_color_t disp_buf1[DISP_BUF_SIZE];

/* Display and input device objects */
static lv_display_t *disp;
static lv_indev_t *indev_touchpad;

/* Forward declarations */
static void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data);

bool lvgl_port_init(void)
{
    ESP_LOGI(TAG, "Initializing LVGL");
    
    /* Initialize LVGL */
    lv_init();
    
    /* Create display */
    disp = lv_display_create(ILI9341_WIDTH, ILI9341_HEIGHT);
    if (!disp) {
        ESP_LOGE(TAG, "Failed to create display");
        return false;
    }
    
    /* Set display buffer */
    lv_display_set_buffers(disp, disp_buf1, NULL, sizeof(disp_buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    /* Set display flush callback */
    lv_display_set_flush_cb(disp, disp_flush);
    
    /* Create touchpad input device */
    indev_touchpad = lv_indev_create();
    if (!indev_touchpad) {
        ESP_LOGE(TAG, "Failed to create input device");
        return false;
    }
    
    lv_indev_set_type(indev_touchpad, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_touchpad, touchpad_read);
    
    ESP_LOGI(TAG, "LVGL initialized successfully");
    return true;
}

void lvgl_port_task_handler(void)
{
    // Update LVGL tick (in milliseconds)
    static uint32_t last_tick = 0;
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t elapsed = now - last_tick;
    if (elapsed > 0) {
        lv_tick_inc(elapsed);
        last_tick = now;
    }
    
    lv_timer_handler();
}

/* Display flush callback */
static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map)
{
    uint16_t *color_p = (uint16_t *)px_map;
    
    /* Set the drawing region */
    ili9341_set_addr_window(area->x1, area->y1, area->x2, area->y2);
    
    /* Apply Swap+Invert color transformation to match screensaver */
    uint32_t size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
    for (uint32_t i = 0; i < size; i++) {
        uint16_t pixel = color_p[i];
        pixel = (pixel >> 8) | (pixel << 8);  // Byte swap
        color_p[i] = ~pixel;                   // Invert
    }
    
    /* Write all pixels in one batch - much faster than pixel-by-pixel */
    extern void ili9341_write_pixels(const uint16_t* pixels, uint32_t length);
    ili9341_write_pixels(color_p, size);
    
    /* Indicate flush is complete */
    lv_display_flush_ready(disp_drv);
}

/* Touchpad read callback */
static void touchpad_read(lv_indev_t *indev_drv, lv_indev_data_t *data)
{
    static int16_t last_x = 0;
    static int16_t last_y = 0;
    
    ft6236_touch_t touch_data;
    if (ft6236_read_touch(&touch_data) && touch_data.touch_count > 0) {
        uint16_t raw_x = touch_data.points[0].x;
        uint16_t raw_y = touch_data.points[0].y;
        
        // Calibrated coordinate mapping (same as original code)
        int32_t x = 320 - ((raw_y - 31) * 320) / 285;  
        int32_t y = ((raw_x - 26) * 240) / 213;
        
        // Clamp to display bounds
        if (x < 0) x = 0;
        if (x >= 320) x = 319;
        if (y < 0) y = 0;
        if (y >= 240) y = 239;
        
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
        
        last_x = x;
        last_y = y;
        
        // Update touch time (for screensaver)
        extern void update_touch_time(void);
        update_touch_time();
    } else {
        data->point.x = last_x;
        data->point.y = last_y;
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
