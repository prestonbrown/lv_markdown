/* SPDX-License-Identifier: MIT */

#include "lv_markdown.h"
#include "md4c.h"
#include <string.h>
#include <stdlib.h>

/* --- Internal data --- */

typedef struct {
    char *                 text;        /**< Owned copy of markdown text (NULL if static) */
    const char *           text_ptr;    /**< Pointer to current text (owned or static) */
    uint8_t                is_static;   /**< 1 if text_ptr points to caller-owned memory */
    lv_markdown_style_t    style;       /**< Rendering style config */
    uint32_t               block_count; /**< Number of top-level blocks */
} lv_markdown_data_t;

/* --- Inline formatting flags (can be combined) --- */

#define MD_FMT_BOLD   (1 << 0)
#define MD_FMT_ITALIC (1 << 1)
#define MD_FMT_CODE   (1 << 2)

/* --- md4c renderer state --- */

typedef struct {
    lv_obj_t *             widget;      /**< The markdown widget (container) */
    lv_markdown_data_t *   data;        /**< Widget data */
    lv_obj_t *             cur_span;    /**< Current spangroup being built */
    uint32_t               block_count; /**< Running count of top-level blocks */
    int                    block_depth; /**< Nesting depth for blocks */
    uint8_t                fmt_flags;   /**< Active inline formatting (MD_FMT_*) */
} md_render_ctx_t;

/* --- Inline formatting helper --- */

/**
 * Apply inline formatting styles to a span based on the active fmt_flags.
 *
 * Font selection priority:
 *   - BOLD+ITALIC: bold_italic_font > bold_font > italic_font > fallbacks
 *   - BOLD:        bold_font > letter_space fallback
 *   - ITALIC:      italic_font > underline fallback
 *   - CODE:        code_font > body_font, plus code_color
 *
 * Fallback note: LVGL spangroups do not support per-span shadow styles,
 * so faux-bold uses increased letter_space (+1) instead of text shadow.
 */
static void apply_span_formatting(lv_span_t * span, md_render_ctx_t * ctx)
{
    const lv_markdown_style_t * s = &ctx->data->style;
    uint8_t flags = ctx->fmt_flags;
    lv_style_t * style = lv_span_get_style(span);

    if(flags & MD_FMT_CODE) {
        /* Inline code: font + color. Code suppresses bold/italic per markdown spec. */
        const lv_font_t * font = s->code_font ? s->code_font : s->body_font;
        lv_style_set_text_font(style, font);
        lv_style_set_text_color(style, s->code_color);
        /* code_bg_color is a documented limitation: LVGL spangroups
         * don't support per-span backgrounds, so skip it. */
        return;
    }

    int is_bold   = (flags & MD_FMT_BOLD) != 0;
    int is_italic = (flags & MD_FMT_ITALIC) != 0;

    if(is_bold && is_italic) {
        /* Bold+Italic: try dedicated font first */
        if(s->bold_italic_font != NULL) {
            lv_style_set_text_font(style, s->bold_italic_font);
            return;
        }
        /* Try bold font with italic fallback */
        if(s->bold_font != NULL) {
            lv_style_set_text_font(style, s->bold_font);
            lv_style_set_text_decor(style, LV_TEXT_DECOR_UNDERLINE);
            return;
        }
        /* Try italic font with bold fallback */
        if(s->italic_font != NULL) {
            lv_style_set_text_font(style, s->italic_font);
            lv_style_set_text_letter_space(style, 1);
            return;
        }
        /* All NULL: combine both fallbacks */
        lv_style_set_text_letter_space(style, 1);
        lv_style_set_text_decor(style, LV_TEXT_DECOR_UNDERLINE);
        return;
    }

    if(is_bold) {
        if(s->bold_font != NULL) {
            lv_style_set_text_font(style, s->bold_font);
        }
        else {
            /* Faux bold via letter spacing (+1px) */
            lv_style_set_text_letter_space(style, 1);
        }
        return;
    }

    if(is_italic) {
        if(s->italic_font != NULL) {
            lv_style_set_text_font(style, s->italic_font);
        }
        else {
            /* Italic fallback via underline decoration */
            lv_style_set_text_decor(style, LV_TEXT_DECOR_UNDERLINE);
        }
        return;
    }
}

/* --- Block spacing helper --- */

static void apply_block_spacing(lv_obj_t * block, md_render_ctx_t * ctx)
{
    if(lv_obj_get_child_count(ctx->widget) > 1) {
        lv_obj_set_style_margin_top(block, ctx->data->style.paragraph_spacing, 0);
    }
}

/* --- md4c callbacks --- */

static int md_enter_block(MD_BLOCKTYPE type, void * detail, void * userdata)
{
    md_render_ctx_t * ctx = (md_render_ctx_t *)userdata;

    ctx->block_depth++;

    switch(type) {
        case MD_BLOCK_DOC:
            /* Document root — don't count as a block */
            break;
        case MD_BLOCK_P:
        case MD_BLOCK_H: {
            ctx->block_count++;
            /* Create a spangroup for this paragraph/heading */
            lv_obj_t * sg = lv_spangroup_create(ctx->widget);
            lv_obj_set_width(sg, LV_PCT(100));
            lv_spangroup_set_mode(sg, LV_SPAN_MODE_BREAK);

            const lv_font_t * font = ctx->data->style.body_font;
            lv_color_t color = ctx->data->style.body_color;

            if(type == MD_BLOCK_H) {
                MD_BLOCK_H_DETAIL * h = (MD_BLOCK_H_DETAIL *)detail;
                int level = h->level - 1; /* 0-indexed */
                if(level >= 0 && level < 6) {
                    if(ctx->data->style.heading_font[level] != NULL) {
                        font = ctx->data->style.heading_font[level];
                    }
                    color = ctx->data->style.heading_color[level];
                }
            }

            lv_obj_set_style_text_font(sg, font, 0);
            lv_obj_set_style_text_color(sg, color, 0);

            apply_block_spacing(sg, ctx);

            ctx->cur_span = sg;
            break;
        }
        case MD_BLOCK_HR: {
            ctx->block_count++;
            /* Horizontal rule: a thin colored bar */
            lv_obj_t * hr = lv_obj_create(ctx->widget);
            lv_obj_remove_style_all(hr);
            lv_obj_set_width(hr, LV_PCT(100));
            lv_obj_set_height(hr, ctx->data->style.hr_height);
            lv_obj_set_style_bg_color(hr, ctx->data->style.hr_color, 0);
            lv_obj_set_style_bg_opa(hr, LV_OPA_COVER, 0);

            apply_block_spacing(hr, ctx);
            break;
        }
        default:
            break;
    }

    return 0;
}

static int md_leave_block(MD_BLOCKTYPE type, void * detail, void * userdata)
{
    md_render_ctx_t * ctx = (md_render_ctx_t *)userdata;

    (void)type;
    (void)detail;

    ctx->block_depth--;

    switch(type) {
        case MD_BLOCK_P:
        case MD_BLOCK_H:
            if(ctx->cur_span != NULL) {
                lv_spangroup_refresh(ctx->cur_span);
                ctx->cur_span = NULL;
            }
            break;
        default:
            break;
    }

    return 0;
}

static int md_enter_span(MD_SPANTYPE type, void * detail, void * userdata)
{
    md_render_ctx_t * ctx = (md_render_ctx_t *)userdata;

    (void)detail;

    switch(type) {
        case MD_SPAN_STRONG:
            ctx->fmt_flags |= MD_FMT_BOLD;
            break;
        case MD_SPAN_EM:
            ctx->fmt_flags |= MD_FMT_ITALIC;
            break;
        case MD_SPAN_CODE:
            ctx->fmt_flags |= MD_FMT_CODE;
            break;
        default:
            break;
    }

    return 0;
}

static int md_leave_span(MD_SPANTYPE type, void * detail, void * userdata)
{
    md_render_ctx_t * ctx = (md_render_ctx_t *)userdata;

    (void)detail;

    switch(type) {
        case MD_SPAN_STRONG:
            ctx->fmt_flags &= ~MD_FMT_BOLD;
            break;
        case MD_SPAN_EM:
            ctx->fmt_flags &= ~MD_FMT_ITALIC;
            break;
        case MD_SPAN_CODE:
            ctx->fmt_flags &= ~MD_FMT_CODE;
            break;
        default:
            break;
    }

    return 0;
}

static int md_text(MD_TEXTTYPE type, const MD_CHAR * text, MD_SIZE size, void * userdata)
{
    md_render_ctx_t * ctx = (md_render_ctx_t *)userdata;

    (void)type;

    if(ctx->cur_span == NULL) return 0;

    lv_span_t * span = lv_spangroup_add_span(ctx->cur_span);
    if(span == NULL) return 0;

    /* md4c text is not null-terminated, so we need a temporary copy */
    char * tmp = (char *)lv_malloc(size + 1);
    if(tmp == NULL) return 0;
    memcpy(tmp, text, size);
    tmp[size] = '\0';

    lv_span_set_text(span, tmp);
    lv_free(tmp);

    /* Apply inline formatting (bold, italic, code) if any flags are active */
    if(ctx->fmt_flags != 0) {
        apply_span_formatting(span, ctx);
    }

    return 0;
}

/* --- Internal helpers --- */

static void lv_markdown_clear(lv_obj_t * obj, lv_markdown_data_t * data)
{
    lv_obj_clean(obj);

    if(data->text != NULL) {
        lv_free(data->text);
        data->text = NULL;
    }
    data->text_ptr    = NULL;
    data->is_static   = 0;
    data->block_count = 0;
}

static void lv_markdown_render(lv_obj_t * obj, lv_markdown_data_t * data)
{
    if(data->text_ptr == NULL || data->text_ptr[0] == '\0') return;

    MD_PARSER parser = {
        .abi_version = 0,
        .flags       = 0,
        .enter_block = md_enter_block,
        .leave_block = md_leave_block,
        .enter_span  = md_enter_span,
        .leave_span  = md_leave_span,
        .text        = md_text,
        .debug_log   = NULL,
        .syntax      = NULL,
    };

    md_render_ctx_t ctx = {
        .widget      = obj,
        .data        = data,
        .cur_span    = NULL,
        .block_count = 0,
        .block_depth = 0,
        .fmt_flags   = 0,
    };

    md_parse(data->text_ptr, (MD_SIZE)strlen(data->text_ptr), &parser, &ctx);

    data->block_count = ctx.block_count;
}

/* --- Cleanup event handler --- */

static void lv_markdown_delete_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    lv_markdown_data_t * data = (lv_markdown_data_t *)lv_obj_get_user_data(obj);
    if(data != NULL) {
        if(data->text != NULL) {
            lv_free(data->text);
        }
        lv_free(data);
        lv_obj_set_user_data(obj, NULL);
    }
}

/* --- Public API --- */

lv_obj_t * lv_markdown_create(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_create(parent);
    if(obj == NULL) return NULL;

    /* Set up as a clean container */
    lv_obj_remove_style_all(obj);
    lv_obj_set_width(obj, LV_PCT(100));
    lv_obj_set_height(obj, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);

    /* Allocate and attach internal data */
    lv_markdown_data_t * data = (lv_markdown_data_t *)lv_calloc(1, sizeof(lv_markdown_data_t));
    if(data == NULL) {
        lv_obj_delete(obj);
        return NULL;
    }

    lv_markdown_style_init(&data->style);
    lv_obj_set_user_data(obj, data);

    /* Register cleanup on delete */
    lv_obj_add_event_cb(obj, lv_markdown_delete_cb, LV_EVENT_DELETE, NULL);

    return obj;
}

void lv_markdown_set_text(lv_obj_t * obj, const char * text)
{
    lv_markdown_data_t * data = (lv_markdown_data_t *)lv_obj_get_user_data(obj);
    if(data == NULL) return;

    lv_markdown_clear(obj, data);

    if(text == NULL) return;

    /* Copy the text */
    size_t len = strlen(text);
    data->text = (char *)lv_malloc(len + 1);
    if(data->text == NULL) return;
    memcpy(data->text, text, len + 1);
    data->text_ptr  = data->text;
    data->is_static = 0;

    lv_markdown_render(obj, data);
}

void lv_markdown_set_text_static(lv_obj_t * obj, const char * text)
{
    lv_markdown_data_t * data = (lv_markdown_data_t *)lv_obj_get_user_data(obj);
    if(data == NULL) return;

    lv_markdown_clear(obj, data);

    if(text == NULL) return;

    data->text_ptr  = text;
    data->is_static = 1;

    lv_markdown_render(obj, data);
}

void lv_markdown_set_style(lv_obj_t * obj, const lv_markdown_style_t * style)
{
    lv_markdown_data_t * data = (lv_markdown_data_t *)lv_obj_get_user_data(obj);
    if(data == NULL || style == NULL) return;

    memcpy(&data->style, style, sizeof(lv_markdown_style_t));

    /* Re-render with new style if text is set */
    if(data->text_ptr != NULL) {
        const char * saved_ptr = data->text_ptr;
        uint8_t was_static = data->is_static;
        char * saved_text = data->text;

        /* Clear children but preserve text */
        lv_obj_clean(obj);
        data->block_count = 0;

        /* Restore text pointer (clear would free it) */
        data->text     = saved_text;
        data->text_ptr = saved_ptr;
        data->is_static = was_static;

        lv_markdown_render(obj, data);
    }
}

const char * lv_markdown_get_text(lv_obj_t * obj)
{
    lv_markdown_data_t * data = (lv_markdown_data_t *)lv_obj_get_user_data(obj);
    if(data == NULL) return NULL;

    return data->text_ptr;
}

uint32_t lv_markdown_get_block_count(lv_obj_t * obj)
{
    lv_markdown_data_t * data = (lv_markdown_data_t *)lv_obj_get_user_data(obj);
    if(data == NULL) return 0;

    return data->block_count;
}
