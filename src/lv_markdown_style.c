/* SPDX-License-Identifier: MIT */

#include "lv_markdown_style.h"
#include <string.h>

void lv_markdown_style_init(lv_markdown_style_t * style)
{
    if(style == NULL) return;

    memset(style, 0, sizeof(lv_markdown_style_t));

    /* Body defaults */
    style->body_font  = LV_FONT_DEFAULT;
    style->body_color = lv_color_black();

    /* Heading colors default to body color */
    for(int i = 0; i < 6; i++) {
        style->heading_font[i]  = NULL; /* fallback to body_font */
        style->heading_color[i] = lv_color_black();
    }

    /* Emphasis fonts: NULL = use fallback (faux bold / underline) */
    style->bold_font        = NULL;
    style->italic_font      = NULL;
    style->bold_italic_font = NULL;

    /* Inline code */
    style->code_font          = NULL;
    style->code_color         = lv_color_black();
    style->code_bg_color      = lv_color_make(230, 230, 230); /* light gray */
    style->code_corner_radius = 3;

    /* Code blocks */
    style->code_block_bg_color      = lv_color_make(245, 245, 245);
    style->code_block_corner_radius = 4;
    style->code_block_pad           = 8;

    /* Blockquotes */
    style->blockquote_border_color = lv_color_make(200, 200, 200);
    style->blockquote_border_width = 3;
    style->blockquote_pad_left     = 12;

    /* Horizontal rules */
    style->hr_color  = lv_color_make(200, 200, 200);
    style->hr_height = 1;

    /* Spacing */
    style->paragraph_spacing = 10;
    style->line_spacing      = 4;
    style->list_indent       = 20;
    style->list_bullet       = "\xe2\x80\xa2"; /* UTF-8 bullet: • */
}
