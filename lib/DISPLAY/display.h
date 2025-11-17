#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

// Include appropriate display driver based on build configuration
#ifdef DISPLAY_ST7789
    #include "st7789.h"
    
    // Map display functions to ST7789
    #define DISPLAY_WIDTH  ST7789_WIDTH
    #define DISPLAY_HEIGHT ST7789_HEIGHT
    
    // Color mapping (convert RGB565 macros)
    #define DISPLAY_BLACK   ST7789_BLACK
    #define DISPLAY_WHITE   ST7789_WHITE
    #define DISPLAY_RED     ST7789_RED
    #define DISPLAY_GREEN   ST7789_GREEN
    #define DISPLAY_BLUE    ST7789_BLUE
    #define DISPLAY_CYAN    ST7789_CYAN
    #define DISPLAY_MAGENTA ST7789_MAGENTA
    #define DISPLAY_YELLOW  ST7789_YELLOW
    
    typedef st7789_config_t display_config_t;
    typedef uint16_t display_color_t; // RGB565
    
    #define display_init(cfg)           st7789_init(cfg)
    #define display_fill_screen(color)  st7789_fill_screen(color)
    #define display_fill_rect(x,y,w,h,c) st7789_fill_rect(x,y,w,h,c)
    #define display_draw_pixel(x,y,c)   st7789_draw_pixel(x,y,c)
    #define display_backlight(en)       st7789_backlight(en)
    #define display_on()                st7789_display_on()
    #define display_set_spi_settings    st7789_set_spi_settings
    #define display_set_spi_mode        st7789_set_spi_mode
    
#elif defined(DISPLAY_ST7789_PARALLEL)
    #include "../ST7789_PARALLEL/st7789_parallel.h"
    
    // Map display functions to ST7789 Parallel
    #define DISPLAY_WIDTH  ST7789_PARALLEL_WIDTH
    #define DISPLAY_HEIGHT ST7789_PARALLEL_HEIGHT
    
    // Color mapping (RGB565 macros)
    #define DISPLAY_BLACK   ST7789_PARALLEL_BLACK
    #define DISPLAY_WHITE   ST7789_PARALLEL_WHITE
    #define DISPLAY_RED     ST7789_PARALLEL_RED
    #define DISPLAY_GREEN   ST7789_PARALLEL_GREEN
    #define DISPLAY_BLUE    ST7789_PARALLEL_BLUE
    #define DISPLAY_CYAN    ST7789_PARALLEL_CYAN
    #define DISPLAY_MAGENTA ST7789_PARALLEL_MAGENTA
    #define DISPLAY_YELLOW  ST7789_PARALLEL_YELLOW
    
    typedef st7789_parallel_config_t display_config_t;
    typedef uint16_t display_color_t; // RGB565
    
    #define display_init(cfg)           st7789_parallel_init(cfg)
    #define display_fill_screen(color)  st7789_parallel_fill_screen(color)
    #define display_fill_rect(x,y,w,h,c) st7789_parallel_fill_rect(x,y,w,h,c)
    #define display_draw_pixel(x,y,c)   st7789_parallel_draw_pixel(x,y,c)
    #define display_backlight(en)       st7789_parallel_backlight(en)
    #define display_on()                st7789_parallel_display_on()
    #define display_set_addr_window(x0,y0,x1,y1) st7789_parallel_set_addr_window(x0,y0,x1,y1)
    #define display_write_pixels(p,l)   st7789_parallel_write_pixels(p,l)
    
#elif defined(DISPLAY_ST7262)
    #include "st7262.h"
    
    // Map display functions to ST7262
    #define DISPLAY_WIDTH  ST7262_WIDTH
    #define DISPLAY_HEIGHT ST7262_HEIGHT
    
    // Color mapping (RGB888 macros)
    #define DISPLAY_BLACK   ST7262_BLACK
    #define DISPLAY_WHITE   ST7262_WHITE
    #define DISPLAY_RED     ST7262_RED
    #define DISPLAY_GREEN   ST7262_GREEN
    #define DISPLAY_BLUE    ST7262_BLUE
    #define DISPLAY_CYAN    ST7262_CYAN
    #define DISPLAY_MAGENTA ST7262_MAGENTA
    #define DISPLAY_YELLOW  ST7262_YELLOW
    
    typedef st7262_config_t display_config_t;
    typedef uint32_t display_color_t; // RGB888
    
    #define display_init(cfg)           st7262_init(cfg)
    #define display_fill_screen(color)  st7262_fill_screen(color)
    #define display_fill_rect(x,y,w,h,c) st7262_fill_rect(x,y,w,h,c)
    #define display_draw_pixel(x,y,c)   st7262_draw_pixel(x,y,c)
    #define display_backlight(en)       st7262_backlight(en)
    #define display_on()                ((void)0) // No separate display on for parallel
    
#else
    #error "No display type defined! Define DISPLAY_ST7789, DISPLAY_ST7789_PARALLEL, or DISPLAY_ST7262 in platformio.ini"
#endif

#endif // DISPLAY_H
