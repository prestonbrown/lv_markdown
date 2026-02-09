/* SPDX-License-Identifier: MIT */

/**
 * @file lv_conf.h
 * @brief Minimal LVGL configuration for lv_markdown tests
 */

#ifndef LV_CONF_H
#define LV_CONF_H

/* Color depth: 32-bit for testing */
#define LV_COLOR_DEPTH 32

/* Use stdlib for memory, strings, etc. */
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING    LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_CLIB

/* Memory: use clib malloc */
#define LV_MEM_SIZE             (256 * 1024)

/* Display defaults */
#define LV_DPI_DEF 130

/* No OS */
#define LV_USE_OS   LV_OS_NONE

/* Logging disabled for tests (reduces noise) */
#define LV_USE_LOG 0

/* Needed widgets */
#define LV_USE_LABEL    1
#define LV_USE_SPAN     1
#define LV_USE_OBJ      1

/* Not needed — keep build small */
#define LV_USE_ARC          0
#define LV_USE_BAR          0
#define LV_USE_BUTTON       0
#define LV_USE_BUTTONMATRIX 0
#define LV_USE_CALENDAR     0
#define LV_USE_CANVAS       0
#define LV_USE_CHART        0
#define LV_USE_CHECKBOX     0
#define LV_USE_DROPDOWN     0
#define LV_USE_IMAGE        0
#define LV_USE_IMAGEBUTTON  0
#define LV_USE_KEYBOARD     0
#define LV_USE_LED          0
#define LV_USE_LINE         0
#define LV_USE_LIST         0
#define LV_USE_MENU         0
#define LV_USE_MSGBOX       0
#define LV_USE_ROLLER       0
#define LV_USE_SCALE        0
#define LV_USE_SLIDER       0
#define LV_USE_SPINBOX      0
#define LV_USE_SPINNER      0
#define LV_USE_SWITCH       0
#define LV_USE_TABLE        0
#define LV_USE_TABVIEW      0
#define LV_USE_TEXTAREA     0
#define LV_USE_TILEVIEW     0
#define LV_USE_WIN          0
#define LV_USE_ANIMIMG      0
#define LV_USE_LOTTIE       0

/* Disable features we don't need */
#define LV_USE_FLEX     1   /* Need flex for layout */
#define LV_USE_GRID     0
#define LV_USE_OBSERVER 0
#define LV_USE_XML      0
#define LV_USE_FREETYPE 0
#define LV_USE_TINY_TTF 0
#define LV_USE_LODEPNG  0
#define LV_USE_LIBPNG   0
#define LV_USE_LIBJPEG_TURBO 0
#define LV_USE_BMP      0
#define LV_USE_GIF      0
#define LV_USE_QRCODE   0
#define LV_USE_BARCODE  0
#define LV_USE_IMGFONT  0
#define LV_USE_SYSMON   0
#define LV_USE_PROFILER 0
#define LV_USE_MONKEY   0
#define LV_USE_GRIDNAV  0
#define LV_USE_FRAGMENT  0
#define LV_USE_IME_PINYIN 0

/* Font: just the default */
#define LV_FONT_MONTSERRAT_14  1
#define LV_FONT_DEFAULT        &lv_font_montserrat_14

/* Draw engine */
#define LV_USE_DRAW_SW 1

/* Span config */
#define LV_SPAN_SNIPPET_STACK_SIZE 64

#endif /* LV_CONF_H */
