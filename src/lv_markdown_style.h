/* SPDX-License-Identifier: MIT */

/**
 * @file lv_markdown_style.h
 * @brief Style configuration for the LVGL Markdown Viewer Widget
 */

#ifndef LV_MARKDOWN_STYLE_H
#define LV_MARKDOWN_STYLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/**
 * Style configuration for markdown rendering.
 *
 * All font pointers are optional except body_font. When a font is NULL,
 * a fallback strategy is used:
 *   - bold_font:        faux bold via text shadow (1px offset, same color)
 *   - italic_font:      underline decoration
 *   - bold_italic_font: faux bold + underline
 *   - heading_font[N]:  falls back to body_font
 *   - code_font:        falls back to body_font
 */
typedef struct {
    /* Body text — the base everything derives from */
    const lv_font_t * body_font;            /**< Required. Base font for all text. */
    lv_color_t         body_color;           /**< Body text color */

    /* Headings (H1 = index 0, H6 = index 5). NULL = use body_font */
    const lv_font_t * heading_font[6];
    lv_color_t         heading_color[6];

    /* Emphasis. NULL = use fallback (faux bold / underline) */
    const lv_font_t * bold_font;
    const lv_font_t * italic_font;
    const lv_font_t * bold_italic_font;

    /* Inline code */
    const lv_font_t * code_font;             /**< NULL = use body_font */
    lv_color_t         code_color;
    lv_color_t         code_bg_color;
    int32_t            code_corner_radius;

    /* Fenced code blocks */
    lv_color_t         code_block_bg_color;
    int32_t            code_block_corner_radius;
    int32_t            code_block_pad;

    /* Blockquotes */
    lv_color_t         blockquote_border_color;
    int32_t            blockquote_border_width;
    int32_t            blockquote_pad_left;

    /* Horizontal rules */
    lv_color_t         hr_color;
    int32_t            hr_height;

    /* Spacing */
    int32_t            paragraph_spacing;    /**< Vertical gap between blocks */
    int32_t            line_spacing;         /**< Line spacing within a block */
    int32_t            list_indent;          /**< Indent per list nesting level */
    const char *       list_bullet;          /**< Bullet character (default: "•") */
} lv_markdown_style_t;

/**
 * Initialize a style configuration with sensible defaults.
 * After calling this, only body_font is strictly required to be set.
 *
 * @param style     pointer to a style struct to initialize
 */
void lv_markdown_style_init(lv_markdown_style_t * style);

#ifdef __cplusplus
}
#endif

#endif /* LV_MARKDOWN_STYLE_H */
