/* SPDX-License-Identifier: MIT */

/**
 * @file lv_markdown.h
 * @brief LVGL Markdown Viewer Widget
 *
 * Renders markdown content as native LVGL widgets using md4c for parsing
 * and spangroups/labels for display.
 */

#ifndef LV_MARKDOWN_H
#define LV_MARKDOWN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "lv_markdown_style.h"

/**
 * Create a markdown viewer widget.
 * The widget grows to fit its content — wrap in a scrollable parent if needed.
 *
 * @param parent    pointer to the parent object
 * @return          pointer to the created markdown widget
 */
lv_obj_t * lv_markdown_create(lv_obj_t * parent);

/**
 * Set the markdown text to render.
 * Clears any previously rendered content and re-renders.
 * The text is copied internally.
 *
 * @param obj       pointer to a markdown widget
 * @param text      markdown string (NULL to clear)
 */
void lv_markdown_set_text(lv_obj_t * obj, const char * text);

/**
 * Set the markdown text without copying.
 * The caller must ensure the string remains valid for the widget's lifetime
 * (or until the next set_text/set_text_static call).
 *
 * @param obj       pointer to a markdown widget
 * @param text      markdown string (NULL to clear), must remain valid
 */
void lv_markdown_set_text_static(lv_obj_t * obj, const char * text);

/**
 * Set the style configuration for rendering.
 * The style struct is copied internally.
 *
 * @param obj       pointer to a markdown widget
 * @param style     pointer to a style configuration
 */
void lv_markdown_set_style(lv_obj_t * obj, const lv_markdown_style_t * style);

/**
 * Get the currently set markdown text.
 *
 * @param obj       pointer to a markdown widget
 * @return          the markdown string, or NULL if none set
 */
const char * lv_markdown_get_text(lv_obj_t * obj);

/**
 * Get the number of top-level blocks parsed from the markdown.
 * Useful for future lazy loading support.
 *
 * @param obj       pointer to a markdown widget
 * @return          number of top-level blocks
 */
uint32_t lv_markdown_get_block_count(lv_obj_t * obj);

#ifdef __cplusplus
}
#endif

#endif /* LV_MARKDOWN_H */
