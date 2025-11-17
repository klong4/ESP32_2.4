/**
 * @file lv_conf.h
 * Configuration file for LVGL v9.2 - OPTIMIZED FOR FLASH SIZE
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/* Color depth: 16 (RGB565) - smallest option */
#define LV_COLOR_DEPTH 16

/* ST7789 expects RGB565 in big-endian byte order, enable swap */
#define LV_COLOR_16_SWAP 1

/*=========================
   MEMORY SETTINGS
 *=========================*/

/* Size of the memory available for `lv_malloc()` in bytes */
#define LV_MEM_SIZE (32 * 1024U)  /* Reduced to 32KB to save RAM */

/* Number of the intermediate memory buffer used during rendering */
#define LV_DRAW_BUF_STRIDE_ALIGN 1
#define LV_DRAW_BUF_ALIGN 4

/*=========================
   HAL SETTINGS
 *=========================*/

/* Default display refresh period in milliseconds */
#define LV_DEF_REFR_PERIOD 10

/* Default Dot Per Inch */
#define LV_DPI_DEF 130

/* Disable all hardware accelerated draw engines */
#define LV_USE_DRAW_SW 1
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

#define LV_USE_OS LV_OS_NONE

/*==================
   THEMES
 *==================*/

/* Disable all themes except default */
#define LV_USE_THEME_DEFAULT 1
#define LV_USE_THEME_MONO 0

/*==================
   FONTS
 *==================*/

/* Enable only the fonts we actually use */
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 0
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 0

/* Disable all other fonts */
#define LV_FONT_MONTSERRAT_8 0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

/* Disable subpixel rendering */
#define LV_FONT_SUBPX_BGR 0
#define LV_FONT_SUBPX_RGB 0

/* Disable uncompressed fonts */
#define LV_USE_FONT_COMPRESSED 0

/* Disable placeholder */
#define LV_USE_FONT_PLACEHOLDER 0

/*==================
   WIDGETS
 *==================*/

/* Enable ONLY the widgets we use */
#define LV_USE_LABEL 1
#define LV_USE_ROLLER 1

/* Disable ALL other widgets */
#define LV_USE_ANIMIMG 0
#define LV_USE_ARC 0
#define LV_USE_BAR 0
#define LV_USE_BUTTON 0
#define LV_USE_BUTTONMATRIX 0
#define LV_USE_CALENDAR 0
#define LV_USE_CANVAS 0
#define LV_USE_CHART 0
#define LV_USE_CHECKBOX 0
#define LV_USE_DROPDOWN 0
#define LV_USE_IMAGE 0
#define LV_USE_IMAGEBUTTON 0
#define LV_USE_KEYBOARD 0
#define LV_USE_LED 0
#define LV_USE_LINE 0
#define LV_USE_LIST 0
#define LV_USE_MENU 0
#define LV_USE_MSGBOX 0
#define LV_USE_SCALE 0
#define LV_USE_SLIDER 0
#define LV_USE_SPAN 0
#define LV_USE_SPINBOX 0
#define LV_USE_SPINNER 0
#define LV_USE_SWITCH 0
#define LV_USE_TABLE 0
#define LV_USE_TABVIEW 0
#define LV_USE_TEXTAREA 0
#define LV_USE_TILEVIEW 0
#define LV_USE_WIN 0

/*==================
   LAYOUTS
 *==================*/

/* Enable flex layout (needed for responsive UI) */
#define LV_USE_FLEX 1
#define LV_USE_GRID 0

/*==================
   ANIMATIONS
 *==================*/

/* Disable animations to save flash */
#define LV_USE_ANIM 0

/*==================
   OTHERS
 *==================*/

/* Disable snapshot */
#define LV_USE_SNAPSHOT 0

/* Disable monkey test */
#define LV_USE_MONKEY 0

/* Disable gridnav */
#define LV_USE_GRIDNAV 0

/* Disable fragment */
#define LV_USE_FRAGMENT 0

/* Disable imgfont */
#define LV_USE_IMGFONT 0

/* Disable observer */
#define LV_USE_OBSERVER 0

/* Disable message */
#define LV_USE_MSG 0

/* Disable IME pinyin */
#define LV_USE_IME_PINYIN 0

/* Disable file explorer */
#define LV_USE_FILE_EXPLORER 0

/* Logging - minimal */
#define LV_USE_LOG 0

/* Disable all asserts to save flash */
#define LV_USE_ASSERT_NULL 0
#define LV_USE_ASSERT_MALLOC 0
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0

/* Disable performance monitor */
#define LV_USE_PERF_MONITOR 0

/* Disable memory monitor */
#define LV_USE_MEM_MONITOR 0

/* Disable user data */
#define LV_USE_USER_DATA 0

/* Disable built-in image formats we don't use */
#define LV_USE_BMP 0
#define LV_USE_PNG 0
#define LV_USE_SJPG 0
#define LV_USE_GIF 0
#define LV_USE_QRCODE 0
#define LV_USE_BARCODE 0
#define LV_USE_FREETYPE 0
#define LV_USE_TINY_TTF 0
#define LV_USE_RLOTTIE 0
#define LV_USE_FFMPEG 0

#endif /*LV_CONF_H*/
