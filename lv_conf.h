/**
 * @file lv_conf.h
 * Configuration file for LVGL v9.2
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/* Color depth: 16 (RGB565), 24 (RGB888), 32 (XRGB8888) */
#define LV_COLOR_DEPTH 16

/* Enable font anti-aliasing (2 bit for smoother edges) */
#define LV_FONT_DEFAULT_MONTSERRAT_14

/*=========================
   MEMORY SETTINGS
 *=========================*/

/* Size of the memory available for `lv_malloc()` in bytes */
#define LV_MEM_SIZE (64 * 1024U)  /* 64KB */

/* Number of the intermediate memory buffer used during rendering */
#define LV_DRAW_BUF_STRIDE_ALIGN 1
#define LV_DRAW_BUF_ALIGN 4

/*=========================
   HAL SETTINGS
 *=========================*/

/* Default display refresh period in milliseconds */
#define LV_DEF_REFR_PERIOD 5  /* 200 FPS for maximum responsiveness */

/* Default Dot Per Inch */
#define LV_DPI_DEF 130

/* Inertial scroll settings for fast, responsive scrolling */
#define LV_INDEV_DEF_SCROLL_LIMIT 2       /* Very low threshold for scroll detection */
#define LV_INDEV_DEF_SCROLL_THROW 40      /* High throw value for fast scrolling */
#define LV_INDEV_DEF_LONG_PRESS_TIME 200  /* Faster long press detection */
#define LV_INDEV_DEF_LONG_PRESS_REPEAT_TIME 50  /* Faster repeat */

/* Disable hardware accelerated draw engines */
#define LV_USE_DRAW_VG_LITE 0
#define LV_USE_DRAW_PXP 0
#define LV_USE_DRAW_DMA2D 0
#define LV_USE_DRAW_DAVE2D 0
#define LV_USE_DRAW_NEMA_GFX 0
#define LV_USE_DRAW_G2D 0
#define LV_USE_DRAW_PPA 0
#define LV_USE_DRAW_EVE 0
#define LV_USE_DRAW_OPENGLES 0
#define LV_USE_DRAW_SDL 0

/*=======================
   OPERATING SYSTEM
 *=======================*/

/* Select an operating system to use. Possible options:
 * - LV_OS_NONE
 * - LV_OS_PTHREAD
 * - LV_OS_FREERTOS
 * - LV_OS_CMSIS_RTOS2
 * - LV_OS_RTTHREAD
 * - LV_OS_WINDOWS
 * - LV_OS_CUSTOM */
#define LV_USE_OS LV_OS_NONE  /* Use none, we'll handle threading manually */

/*================
 * ACCELERATORS
 *================*/

/* Disable ARM Helium acceleration (not compatible with Xtensa) */
#define LV_USE_DRAW_SW_HELIUM 0

/*==================
   INPUT DEVICES
 *==================*/

/* Touchpad interface */
#define LV_USE_INDEV_TOUCHPAD 1

/*==================
   WIDGETS
 *==================*/

#define LV_USE_LABEL 1
#define LV_USE_BUTTON 1
#define LV_USE_ROLLER 1
#define LV_USE_LIST 1
#define LV_USE_ARC 1

/*==================
   ANIMATIONS
 *==================*/

/* Default animation time in ms (0 = instant, no animation) */
#define LV_DEF_ANIM_TIME 0  /* Instant animations for maximum speed */

/* Text settings */
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1

/* Enable anti-aliasing for smooth text */
#define LV_FONT_SUBPX_BGR 0
#define LV_FONT_SUBPX_RGB 0

/*==================
   OTHERS
 *==================*/

/* Logging */
#define LV_USE_LOG 1
#if LV_USE_LOG
  #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
  #define LV_LOG_PRINTF 1
#endif

/* Enable assert checking */
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0

/* Performance monitor */
#define LV_USE_PERF_MONITOR 0

/* Memory monitor */
#define LV_USE_MEM_MONITOR 0

#endif /*LV_CONF_H*/
