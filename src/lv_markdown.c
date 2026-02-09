/* SPDX-License-Identifier: MIT */

#include "lv_markdown.h"
#include "md4c.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

/* --- List nesting state --- */

#define MD_LIST_MAX_DEPTH 16

typedef struct {
    uint8_t  is_ordered;   /**< 0 = bullet, 1 = ordered */
    uint8_t  is_tight;     /**< 1 = tight list (no P wrappers from md4c) */
    uint32_t counter;      /**< Current item number for ordered lists */
} md_list_level_t;

/* --- md4c renderer state --- */

typedef struct {
    lv_obj_t *             widget;        /**< The markdown widget (root container) */
    lv_markdown_data_t *   data;          /**< Widget data */
    lv_obj_t *             cur_span;      /**< Current spangroup being built */
    lv_obj_t *             cur_container; /**< Current parent for new blocks (widget or blockquote) */
    uint32_t               block_count;   /**< Running count of top-level blocks */
    int                    block_depth;   /**< Nesting depth for blocks */
    uint8_t                fmt_flags;     /**< Active inline formatting (MD_FMT_*) */

    /* List state */
    md_list_level_t        list_stack[MD_LIST_MAX_DEPTH];
    int                    list_depth;        /**< Current list nesting depth (0 = not in list) */
    uint8_t                li_first_paragraph; /**< 1 if next P inside LI should get bullet/number prefix */

    /* Code block state */
    uint8_t                in_code_block;  /**< 1 when inside MD_BLOCK_CODE */
    char *                 code_buf;       /**< Buffer for accumulating code block text */
    uint32_t               code_buf_len;   /**< Current length of code buffer */
    uint32_t               code_buf_cap;   /**< Allocated capacity of code buffer */
} md_render_ctx_t;

/* --- Code block buffer helper --- */

/**
 * Append text to the code block accumulation buffer, growing as needed.
 */
static void code_buf_append(md_render_ctx_t * ctx, const char * text, uint32_t len)
{
    if(len == 0) return;

    /* Overflow guard: reject if total would exceed uint32_t */
    if(len > UINT32_MAX - ctx->code_buf_len - 1) return;

    uint32_t needed = ctx->code_buf_len + len + 1;
    if(needed > ctx->code_buf_cap) {
        uint32_t new_cap = ctx->code_buf_cap == 0 ? 256 : ctx->code_buf_cap;
        while(new_cap < needed) new_cap *= 2;
        char * new_buf = (char *)lv_realloc(ctx->code_buf, new_cap);
        if(new_buf == NULL) return;
        ctx->code_buf = new_buf;
        ctx->code_buf_cap = new_cap;
    }

    memcpy(ctx->code_buf + ctx->code_buf_len, text, len);
    ctx->code_buf_len += len;
    ctx->code_buf[ctx->code_buf_len] = '\0';
}

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

/* --- List prefix helper --- */

/**
 * Prepend a bullet or number prefix span to a spangroup for a list item.
 */
static void prepend_list_prefix(lv_obj_t * sg, md_render_ctx_t * ctx, int level_idx)
{
    if(ctx->list_stack[level_idx].is_ordered) {
        char num_buf[16];
        snprintf(num_buf, sizeof(num_buf), "%u. ",
                 (unsigned)ctx->list_stack[level_idx].counter);
        lv_span_t * prefix = lv_spangroup_add_span(sg);
        if(prefix != NULL) {
            lv_span_set_text(prefix, num_buf);
        }
    }
    else {
        const char * bullet = ctx->data->style.list_bullet;
        if(bullet != NULL) {
            char buf[32];
            size_t blen = strlen(bullet);
            if(blen < sizeof(buf) - 2) {
                memcpy(buf, bullet, blen);
                buf[blen] = ' ';
                buf[blen + 1] = '\0';
                lv_span_t * prefix = lv_spangroup_add_span(sg);
                if(prefix != NULL) {
                    lv_span_set_text(prefix, buf);
                }
            }
        }
    }
}

/* --- Block spacing helper --- */

static void apply_block_spacing(lv_obj_t * block, md_render_ctx_t * ctx)
{
    /* Use the block's actual parent to check sibling count (works for blockquote children too) */
    lv_obj_t * parent = lv_obj_get_parent(block);
    if(lv_obj_get_child_count(parent) > 1) {
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
            /* Document root -- don't count as a block */
            break;
        case MD_BLOCK_UL: {
            /* Unordered list: push onto list stack */
            if(ctx->list_depth < MD_LIST_MAX_DEPTH) {
                MD_BLOCK_UL_DETAIL * ul = (MD_BLOCK_UL_DETAIL *)detail;
                ctx->list_stack[ctx->list_depth].is_ordered = 0;
                ctx->list_stack[ctx->list_depth].is_tight = ul->is_tight ? 1 : 0;
                ctx->list_stack[ctx->list_depth].counter = 0;
                ctx->list_depth++;
            }
            break;
        }
        case MD_BLOCK_OL: {
            /* Ordered list: push onto list stack with start number */
            if(ctx->list_depth < MD_LIST_MAX_DEPTH) {
                MD_BLOCK_OL_DETAIL * ol = (MD_BLOCK_OL_DETAIL *)detail;
                ctx->list_stack[ctx->list_depth].is_ordered = 1;
                ctx->list_stack[ctx->list_depth].is_tight = ol->is_tight ? 1 : 0;
                ctx->list_stack[ctx->list_depth].counter = ol->start;
                ctx->list_depth++;
            }
            break;
        }
        case MD_BLOCK_LI: {
            if(ctx->list_depth > 0) {
                int level_idx = ctx->list_depth - 1;
                if(ctx->list_stack[level_idx].is_tight) {
                    /* Tight list: md4c skips P blocks, so create spangroup here */
                    ctx->block_count++;

                    lv_obj_t * sg = lv_spangroup_create(ctx->cur_container);
                    lv_obj_set_width(sg, LV_PCT(100));
                    lv_spangroup_set_mode(sg, LV_SPAN_MODE_BREAK);
                    lv_obj_set_style_text_font(sg, ctx->data->style.body_font, 0);
                    lv_obj_set_style_text_color(sg, ctx->data->style.body_color, 0);
                    lv_obj_set_style_text_line_space(sg, ctx->data->style.line_spacing, 0);

                    /* Apply indentation */
                    int32_t indent = ctx->data->style.list_indent * ctx->list_depth;
                    lv_obj_set_style_pad_left(sg, indent, 0);

                    /* Add bullet or number prefix */
                    prepend_list_prefix(sg, ctx, level_idx);

                    apply_block_spacing(sg, ctx);
                    ctx->cur_span = sg;
                }
                else {
                    /* Loose list: mark that the next P should get bullet/number */
                    ctx->li_first_paragraph = 1;
                }
            }
            break;
        }
        case MD_BLOCK_CODE: {
            /* Fenced or indented code block: accumulate text, render on leave */
            ctx->block_count++;
            ctx->in_code_block = 1;
            ctx->code_buf = NULL;
            ctx->code_buf_len = 0;
            ctx->code_buf_cap = 0;
            break;
        }
        case MD_BLOCK_QUOTE: {
            /* Blockquote: create a container with left border and padding */
            ctx->block_count++;
            const lv_markdown_style_t * s = &ctx->data->style;

            lv_obj_t * bq = lv_obj_create(ctx->cur_container);
            lv_obj_remove_style_all(bq);
            lv_obj_set_width(bq, LV_PCT(100));
            lv_obj_set_height(bq, LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(bq, LV_FLEX_FLOW_COLUMN);

            /* Left border styling */
            lv_obj_set_style_border_color(bq, s->blockquote_border_color, 0);
            lv_obj_set_style_border_width(bq, s->blockquote_border_width, 0);
            lv_obj_set_style_border_side(bq, LV_BORDER_SIDE_LEFT, 0);
            lv_obj_set_style_border_opa(bq, LV_OPA_COVER, 0);

            /* Left padding */
            lv_obj_set_style_pad_left(bq, s->blockquote_pad_left, 0);

            apply_block_spacing(bq, ctx);

            /* Redirect child creation to the blockquote container */
            ctx->cur_container = bq;
            break;
        }
        case MD_BLOCK_P:
        case MD_BLOCK_H: {
            /* Don't count P blocks inside blockquotes as separate top-level blocks.
             * The blockquote itself was already counted. */
            if(ctx->cur_container == ctx->widget) {
                ctx->block_count++;
            }
            /* Create a spangroup for this paragraph/heading */
            lv_obj_t * sg = lv_spangroup_create(ctx->cur_container);
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
            lv_obj_set_style_text_line_space(sg, ctx->data->style.line_spacing, 0);

            /* Apply list indentation and bullet/number prefix if inside a list */
            if(ctx->list_depth > 0 && type == MD_BLOCK_P) {
                int32_t indent = ctx->data->style.list_indent * ctx->list_depth;
                lv_obj_set_style_pad_left(sg, indent, 0);

                /* Add bullet or number prefix on the first paragraph of a list item */
                if(ctx->li_first_paragraph) {
                    ctx->li_first_paragraph = 0;
                    prepend_list_prefix(sg, ctx, ctx->list_depth - 1);
                }
            }

            apply_block_spacing(sg, ctx);

            ctx->cur_span = sg;
            break;
        }
        case MD_BLOCK_HR: {
            ctx->block_count++;
            /* Horizontal rule: a thin colored bar */
            lv_obj_t * hr = lv_obj_create(ctx->cur_container);
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

    (void)detail;

    ctx->block_depth--;

    switch(type) {
        case MD_BLOCK_UL:
        case MD_BLOCK_OL:
            /* Pop from list stack */
            if(ctx->list_depth > 0) {
                ctx->list_depth--;
            }
            break;
        case MD_BLOCK_LI: {
            if(ctx->list_depth > 0) {
                int level_idx = ctx->list_depth - 1;
                /* For tight lists, finalize the spangroup created in LI enter */
                if(ctx->list_stack[level_idx].is_tight && ctx->cur_span != NULL) {
                    lv_spangroup_refresh(ctx->cur_span);
                    ctx->cur_span = NULL;
                }
                /* Increment counter for ordered lists (for the next item) */
                if(ctx->list_stack[level_idx].is_ordered) {
                    ctx->list_stack[level_idx].counter++;
                }
            }
            break;
        }
        case MD_BLOCK_CODE: {
            /* Create code block container with accumulated text */
            const lv_markdown_style_t * s = &ctx->data->style;

            lv_obj_t * container = lv_obj_create(ctx->cur_container);
            lv_obj_remove_style_all(container);
            lv_obj_set_width(container, LV_PCT(100));
            lv_obj_set_height(container, LV_SIZE_CONTENT);

            /* Background + corner radius + padding */
            lv_obj_set_style_bg_color(container, s->code_block_bg_color, 0);
            lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
            lv_obj_set_style_radius(container, s->code_block_corner_radius, 0);
            lv_obj_set_style_pad_all(container, s->code_block_pad, 0);

            apply_block_spacing(container, ctx);

            /* Create label inside the container with accumulated code text */
            if(ctx->code_buf != NULL && ctx->code_buf_len > 0) {
                /* Strip trailing newline if present (md4c adds one) */
                if(ctx->code_buf[ctx->code_buf_len - 1] == '\n') {
                    ctx->code_buf[ctx->code_buf_len - 1] = '\0';
                    ctx->code_buf_len--;
                }

                lv_obj_t * label = lv_label_create(container);
                lv_label_set_text(label, ctx->code_buf);
                lv_obj_set_width(label, LV_PCT(100));

                /* Apply code font + color */
                const lv_font_t * font = s->code_font ? s->code_font : s->body_font;
                lv_obj_set_style_text_font(label, font, 0);
                lv_obj_set_style_text_color(label, s->code_color, 0);
            }

            /* Free code buffer */
            if(ctx->code_buf != NULL) {
                lv_free(ctx->code_buf);
                ctx->code_buf = NULL;
            }
            ctx->code_buf_len = 0;
            ctx->code_buf_cap = 0;
            ctx->in_code_block = 0;
            break;
        }
        case MD_BLOCK_QUOTE: {
            /* Restore cur_container to parent */
            lv_obj_t * parent = lv_obj_get_parent(ctx->cur_container);
            ctx->cur_container = parent;
            break;
        }
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

    /* Inside a code block: accumulate text into buffer */
    if(ctx->in_code_block) {
        code_buf_append(ctx, text, size);
        return 0;
    }

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
        .widget             = obj,
        .data               = data,
        .cur_span           = NULL,
        .cur_container      = obj,
        .block_count        = 0,
        .block_depth        = 0,
        .fmt_flags          = 0,
        .list_stack         = {{0}},
        .list_depth         = 0,
        .li_first_paragraph = 0,
        .in_code_block      = 0,
        .code_buf           = NULL,
        .code_buf_len       = 0,
        .code_buf_cap       = 0,
    };

    md_parse(data->text_ptr, (MD_SIZE)strlen(data->text_ptr), &parser, &ctx);

    /* Safety: free code buffer if parsing was interrupted mid-block */
    if(ctx.code_buf != NULL) {
        lv_free(ctx.code_buf);
    }

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
