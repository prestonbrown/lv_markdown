/* SPDX-License-Identifier: MIT */

/**
 * @file test_lv_markdown.c
 * @brief Tests for lv_markdown widget
 *
 * Uses Unity test framework (bundled with LVGL).
 */

#include "lvgl.h"
#include "lvgl_private.h"
#include "lv_markdown.h"

#include "unity/unity.h"

#include <string.h>

/* Forward declarations for test runner */
void setUp(void);
void tearDown(void);

/* --- Test fixtures --- */

static lv_display_t * test_disp = NULL;
static uint8_t test_buf[800 * 480 * 4];

static void dummy_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    (void)area;
    (void)px_map;
    lv_display_flush_ready(disp);
}

void setUp(void)
{
    lv_init();
    test_disp = lv_display_create(800, 480);
    lv_display_set_flush_cb(test_disp, dummy_flush);
    lv_display_set_buffers(test_disp, test_buf, NULL, sizeof(test_buf), LV_DISPLAY_RENDER_MODE_DIRECT);
}

void tearDown(void)
{
    lv_obj_clean(lv_screen_active());
    lv_deinit();
    test_disp = NULL;
}

/* ===== Creation Tests ===== */

void test_markdown_create_returns_valid_obj(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    TEST_ASSERT_NOT_NULL(md);
}

void test_markdown_create_starts_with_no_children(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    TEST_ASSERT_EQUAL_UINT32(0, lv_obj_get_child_count(md));
}

void test_markdown_get_text_returns_null_initially(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    TEST_ASSERT_NULL(lv_markdown_get_text(md));
}

void test_markdown_get_block_count_zero_initially(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    TEST_ASSERT_EQUAL_UINT32(0, lv_markdown_get_block_count(md));
}

/* ===== Plain Paragraph Tests ===== */

void test_markdown_plain_paragraph_creates_one_child(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "Hello world");

    /* A plain paragraph should produce one block (spangroup) */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
}

void test_markdown_plain_paragraph_block_count(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "Hello world");

    TEST_ASSERT_EQUAL_UINT32(1, lv_markdown_get_block_count(md));
}

void test_markdown_set_text_null_clears_content(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "Hello world");
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));

    lv_markdown_set_text(md, NULL);
    TEST_ASSERT_EQUAL_UINT32(0, lv_obj_get_child_count(md));
    TEST_ASSERT_NULL(lv_markdown_get_text(md));
}

void test_markdown_set_text_empty_string(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "");
    TEST_ASSERT_EQUAL_UINT32(0, lv_obj_get_child_count(md));
}

void test_markdown_set_text_replaces_previous(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "First paragraph");
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));

    lv_markdown_set_text(md, "Second paragraph");
    /* Should still be 1 child — old content replaced */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
}

void test_markdown_two_paragraphs_creates_two_children(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "First paragraph\n\nSecond paragraph");

    TEST_ASSERT_EQUAL_UINT32(2, lv_obj_get_child_count(md));
    TEST_ASSERT_EQUAL_UINT32(2, lv_markdown_get_block_count(md));
}

void test_markdown_get_text_returns_set_text(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "Hello world");

    TEST_ASSERT_EQUAL_STRING("Hello world", lv_markdown_get_text(md));
}

void test_markdown_set_text_static_works(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    const char * text = "Static text";
    lv_markdown_set_text_static(md, text);

    /* Should return the same pointer (not a copy) */
    TEST_ASSERT_EQUAL_PTR(text, lv_markdown_get_text(md));
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
}

/* ===== Heading Tests ===== */

void test_markdown_h1_creates_one_child(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "# Heading");

    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
}

void test_markdown_h1_block_count(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "# Heading");

    TEST_ASSERT_EQUAL_UINT32(1, lv_markdown_get_block_count(md));
}

void test_markdown_h2_creates_one_child(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "## Heading");

    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
}

void test_markdown_all_heading_levels(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "# H1\n\n## H2\n\n### H3\n\n#### H4\n\n##### H5\n\n###### H6");

    TEST_ASSERT_EQUAL_UINT32(6, lv_obj_get_child_count(md));
    TEST_ASSERT_EQUAL_UINT32(6, lv_markdown_get_block_count(md));
}

void test_markdown_heading_uses_style_font(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    /* Create a custom style with a specific H1 font */
    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.heading_font[0] = LV_FONT_DEFAULT; /* explicitly set (not NULL fallback) */
    lv_markdown_set_style(md, &style);

    lv_markdown_set_text(md, "# Test");

    lv_obj_t * child = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(child);

    const lv_font_t * applied = lv_obj_get_style_text_font(child, 0);
    TEST_ASSERT_EQUAL_PTR(LV_FONT_DEFAULT, applied);
}

void test_markdown_heading_uses_style_color(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.heading_color[0] = lv_color_make(255, 0, 0); /* red */
    lv_markdown_set_style(md, &style);

    lv_markdown_set_text(md, "# Test");

    lv_obj_t * child = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(child);

    lv_color_t applied = lv_obj_get_style_text_color(child, 0);
    TEST_ASSERT_EQUAL_UINT32(lv_color_make(255, 0, 0).red, applied.red);
    TEST_ASSERT_EQUAL_UINT32(lv_color_make(255, 0, 0).green, applied.green);
    TEST_ASSERT_EQUAL_UINT32(lv_color_make(255, 0, 0).blue, applied.blue);
}

/* ===== Horizontal Rule Tests ===== */

void test_markdown_hr_creates_one_child(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "---");

    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
}

void test_markdown_hr_block_count(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "---");

    TEST_ASSERT_EQUAL_UINT32(1, lv_markdown_get_block_count(md));
}

void test_markdown_hr_uses_style_height(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.hr_height = 5;
    lv_markdown_set_style(md, &style);

    lv_markdown_set_text(md, "---");

    /* Force layout calculation */
    lv_refr_now(NULL);

    lv_obj_t * child = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(child);
    TEST_ASSERT_EQUAL_INT32(5, lv_obj_get_height(child));
}

void test_markdown_hr_uses_style_color(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.hr_color = lv_color_make(0, 128, 255);
    lv_markdown_set_style(md, &style);

    lv_markdown_set_text(md, "---");

    lv_obj_t * child = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(child);

    lv_color_t applied = lv_obj_get_style_bg_color(child, 0);
    TEST_ASSERT_EQUAL_UINT32(0, applied.red);
    TEST_ASSERT_EQUAL_UINT32(128, applied.green);
    TEST_ASSERT_EQUAL_UINT32(255, applied.blue);
}

/* ===== Mixed Block Tests ===== */

void test_markdown_heading_then_paragraph(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "# Title\n\nBody text");

    TEST_ASSERT_EQUAL_UINT32(2, lv_obj_get_child_count(md));
    TEST_ASSERT_EQUAL_UINT32(2, lv_markdown_get_block_count(md));
}

void test_markdown_paragraph_hr_paragraph(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "Before\n\n---\n\nAfter");

    TEST_ASSERT_EQUAL_UINT32(3, lv_obj_get_child_count(md));
    TEST_ASSERT_EQUAL_UINT32(3, lv_markdown_get_block_count(md));
}

/* ===== Style Configuration Tests ===== */

void test_markdown_set_style_re_renders(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "# Hello\n\nWorld");
    TEST_ASSERT_EQUAL_UINT32(2, lv_obj_get_child_count(md));

    /* Change style — should re-render with same children count */
    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.heading_color[0] = lv_color_make(0, 255, 0);
    lv_markdown_set_style(md, &style);

    TEST_ASSERT_EQUAL_UINT32(2, lv_obj_get_child_count(md));
}

void test_markdown_set_style_before_text(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    /* Set style first, then text */
    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.heading_font[0] = LV_FONT_DEFAULT;
    lv_markdown_set_style(md, &style);

    lv_markdown_set_text(md, "# Test");

    lv_obj_t * child = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(child);

    const lv_font_t * applied = lv_obj_get_style_text_font(child, 0);
    TEST_ASSERT_EQUAL_PTR(LV_FONT_DEFAULT, applied);
}

/* ===== Fallback Tests ===== */

void test_markdown_heading_null_font_uses_body_font(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    /* Default style has heading_font[0] = NULL, should fallback to body_font */
    lv_markdown_set_text(md, "# Test");

    lv_obj_t * child = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(child);

    const lv_font_t * applied = lv_obj_get_style_text_font(child, 0);
    TEST_ASSERT_EQUAL_PTR(LV_FONT_DEFAULT, applied);
}

/* ===== Block Spacing Tests ===== */

void test_markdown_first_block_no_top_margin(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "First\n\nSecond");

    lv_obj_t * first = lv_obj_get_child(md, 0);
    TEST_ASSERT_EQUAL_INT32(0, lv_obj_get_style_margin_top(first, 0));
}

void test_markdown_second_block_has_spacing(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "First\n\nSecond");

    lv_obj_t * second = lv_obj_get_child(md, 1);
    /* Default paragraph_spacing is 10 */
    TEST_ASSERT_EQUAL_INT32(10, lv_obj_get_style_margin_top(second, 0));
}

/* ===== Edge Case Tests ===== */

void test_markdown_only_newlines(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "\n\n\n");

    TEST_ASSERT_EQUAL_UINT32(0, lv_obj_get_child_count(md));
}

void test_markdown_single_newline_no_break(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "line1\nline2");

    /* Soft break: same paragraph, single child */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
}

/* ===== Inline Formatting Helper ===== */

/**
 * Get a style property from a span's local style.
 * Returns LV_STYLE_RES_FOUND if the property is set.
 */
static lv_style_res_t get_span_style_prop(lv_span_t * span, lv_style_prop_t prop, lv_style_value_t * value)
{
    lv_style_t * style = lv_span_get_style(span);
    return lv_style_get_prop(style, prop, value);
}

/* ===== Bold Tests ===== */

void test_markdown_bold_creates_spans(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "plain **bold** plain");

    /* Should produce 1 spangroup with 3 spans */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
    lv_obj_t * sg = lv_obj_get_child(md, 0);
    TEST_ASSERT_EQUAL_UINT32(3, lv_spangroup_get_span_count(sg));
}

void test_markdown_bold_with_font(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.bold_font = LV_FONT_DEFAULT; /* Use default as the "bold" font */
    lv_markdown_set_style(md, &style);

    lv_markdown_set_text(md, "**bold**");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    lv_span_t * span = lv_spangroup_get_child(sg, 0);
    TEST_ASSERT_NOT_NULL(span);

    lv_style_value_t val;
    lv_style_res_t res = get_span_style_prop(span, LV_STYLE_TEXT_FONT, &val);
    TEST_ASSERT_EQUAL(LV_STYLE_RES_FOUND, res);
    TEST_ASSERT_EQUAL_PTR(LV_FONT_DEFAULT, val.ptr);
}

void test_markdown_bold_fallback_letter_space(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    /* Leave bold_font = NULL (default), should use letter_space fallback */
    lv_markdown_set_text(md, "**bold**");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    lv_span_t * span = lv_spangroup_get_child(sg, 0);
    TEST_ASSERT_NOT_NULL(span);

    /* Faux bold: letter spacing increased by 1 */
    lv_style_value_t val;
    lv_style_res_t res = get_span_style_prop(span, LV_STYLE_TEXT_LETTER_SPACE, &val);
    TEST_ASSERT_EQUAL(LV_STYLE_RES_FOUND, res);
    TEST_ASSERT_EQUAL_INT32(1, val.num);
}

/* ===== Italic Tests ===== */

void test_markdown_italic_with_font(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.italic_font = LV_FONT_DEFAULT;
    lv_markdown_set_style(md, &style);

    lv_markdown_set_text(md, "*italic*");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    lv_span_t * span = lv_spangroup_get_child(sg, 0);
    TEST_ASSERT_NOT_NULL(span);

    lv_style_value_t val;
    lv_style_res_t res = get_span_style_prop(span, LV_STYLE_TEXT_FONT, &val);
    TEST_ASSERT_EQUAL(LV_STYLE_RES_FOUND, res);
    TEST_ASSERT_EQUAL_PTR(LV_FONT_DEFAULT, val.ptr);
}

void test_markdown_italic_fallback_underline(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    /* Leave italic_font = NULL (default), should use underline fallback */
    lv_markdown_set_text(md, "*italic*");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    lv_span_t * span = lv_spangroup_get_child(sg, 0);
    TEST_ASSERT_NOT_NULL(span);

    lv_style_value_t val;
    lv_style_res_t res = get_span_style_prop(span, LV_STYLE_TEXT_DECOR, &val);
    TEST_ASSERT_EQUAL(LV_STYLE_RES_FOUND, res);
    TEST_ASSERT_EQUAL_INT32(LV_TEXT_DECOR_UNDERLINE, val.num);
}

/* ===== Bold+Italic Tests ===== */

void test_markdown_bold_italic_with_font(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.bold_italic_font = LV_FONT_DEFAULT;
    lv_markdown_set_style(md, &style);

    lv_markdown_set_text(md, "***both***");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    lv_span_t * span = lv_spangroup_get_child(sg, 0);
    TEST_ASSERT_NOT_NULL(span);

    lv_style_value_t val;
    lv_style_res_t res = get_span_style_prop(span, LV_STYLE_TEXT_FONT, &val);
    TEST_ASSERT_EQUAL(LV_STYLE_RES_FOUND, res);
    TEST_ASSERT_EQUAL_PTR(LV_FONT_DEFAULT, val.ptr);
}

void test_markdown_bold_italic_fallback(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    /* Leave all emphasis fonts NULL — should get letter_space + underline */
    lv_markdown_set_text(md, "***both***");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    lv_span_t * span = lv_spangroup_get_child(sg, 0);
    TEST_ASSERT_NOT_NULL(span);

    /* Should have underline (italic fallback) */
    lv_style_value_t val;
    lv_style_res_t res = get_span_style_prop(span, LV_STYLE_TEXT_DECOR, &val);
    TEST_ASSERT_EQUAL(LV_STYLE_RES_FOUND, res);
    TEST_ASSERT_EQUAL_INT32(LV_TEXT_DECOR_UNDERLINE, val.num);

    /* Should have letter_space (bold fallback) */
    res = get_span_style_prop(span, LV_STYLE_TEXT_LETTER_SPACE, &val);
    TEST_ASSERT_EQUAL(LV_STYLE_RES_FOUND, res);
    TEST_ASSERT_EQUAL_INT32(1, val.num);
}

/* ===== Inline Code Tests ===== */

void test_markdown_inline_code_with_font(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.code_font = LV_FONT_DEFAULT;
    lv_markdown_set_style(md, &style);

    lv_markdown_set_text(md, "`code`");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    lv_span_t * span = lv_spangroup_get_child(sg, 0);
    TEST_ASSERT_NOT_NULL(span);

    lv_style_value_t val;
    lv_style_res_t res = get_span_style_prop(span, LV_STYLE_TEXT_FONT, &val);
    TEST_ASSERT_EQUAL(LV_STYLE_RES_FOUND, res);
    TEST_ASSERT_EQUAL_PTR(LV_FONT_DEFAULT, val.ptr);
}

void test_markdown_inline_code_color(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.code_color = lv_color_make(255, 0, 0);
    lv_markdown_set_style(md, &style);

    lv_markdown_set_text(md, "`code`");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    lv_span_t * span = lv_spangroup_get_child(sg, 0);
    TEST_ASSERT_NOT_NULL(span);

    lv_style_value_t val;
    lv_style_res_t res = get_span_style_prop(span, LV_STYLE_TEXT_COLOR, &val);
    TEST_ASSERT_EQUAL(LV_STYLE_RES_FOUND, res);
    TEST_ASSERT_EQUAL_UINT32(255, val.color.red);
    TEST_ASSERT_EQUAL_UINT32(0, val.color.green);
    TEST_ASSERT_EQUAL_UINT32(0, val.color.blue);
}

void test_markdown_inline_code_fallback_body_font(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    /* Leave code_font = NULL — should use body_font */
    lv_markdown_set_text(md, "`code`");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    lv_span_t * span = lv_spangroup_get_child(sg, 0);
    TEST_ASSERT_NOT_NULL(span);

    /* Font should be set to body_font (LV_FONT_DEFAULT) */
    lv_style_value_t val;
    lv_style_res_t res = get_span_style_prop(span, LV_STYLE_TEXT_FONT, &val);
    TEST_ASSERT_EQUAL(LV_STYLE_RES_FOUND, res);
    TEST_ASSERT_EQUAL_PTR(LV_FONT_DEFAULT, val.ptr);
}

/* ===== Mixed Formatting Tests ===== */

void test_markdown_multiple_formats_in_paragraph(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "normal **bold** and *italic* text");

    /* Should produce 1 spangroup */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    /* "normal " + "bold" + " and " + "italic" + " text" = 5 spans */
    TEST_ASSERT_EQUAL_UINT32(5, lv_spangroup_get_span_count(sg));
}

/* ===== Formatting Edge Cases ===== */

void test_markdown_plain_spans_have_no_formatting(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "plain **bold** plain");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    /* Span 0 = "plain ", span 2 = " plain" — should have no formatting */
    lv_span_t * span0 = lv_spangroup_get_child(sg, 0);
    lv_span_t * span2 = lv_spangroup_get_child(sg, 2);

    lv_style_value_t val;
    TEST_ASSERT_NOT_EQUAL(LV_STYLE_RES_FOUND, get_span_style_prop(span0, LV_STYLE_TEXT_LETTER_SPACE, &val));
    TEST_ASSERT_NOT_EQUAL(LV_STYLE_RES_FOUND, get_span_style_prop(span0, LV_STYLE_TEXT_DECOR, &val));
    TEST_ASSERT_NOT_EQUAL(LV_STYLE_RES_FOUND, get_span_style_prop(span2, LV_STYLE_TEXT_LETTER_SPACE, &val));
    TEST_ASSERT_NOT_EQUAL(LV_STYLE_RES_FOUND, get_span_style_prop(span2, LV_STYLE_TEXT_DECOR, &val));
}

void test_markdown_bold_inside_heading(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "# Hello **world**");

    /* 1 child (heading spangroup), 2 spans ("Hello " + "world") */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
    lv_obj_t * sg = lv_obj_get_child(md, 0);
    TEST_ASSERT_EQUAL_UINT32(2, lv_spangroup_get_span_count(sg));

    /* Second span should have bold fallback (letter_space) */
    lv_span_t * bold_span = lv_spangroup_get_child(sg, 1);
    lv_style_value_t val;
    lv_style_res_t res = get_span_style_prop(bold_span, LV_STYLE_TEXT_LETTER_SPACE, &val);
    TEST_ASSERT_EQUAL(LV_STYLE_RES_FOUND, res);
    TEST_ASSERT_EQUAL_INT32(1, val.num);
}

/* ===== Bullet List Tests ===== */

void test_markdown_bullet_list_creates_children(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "- Item 1\n- Item 2\n- Item 3");

    /* Each list item's paragraph becomes a spangroup child */
    TEST_ASSERT_EQUAL_UINT32(3, lv_obj_get_child_count(md));
}

void test_markdown_bullet_list_block_count(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "- Item 1\n- Item 2\n- Item 3");

    TEST_ASSERT_EQUAL_UINT32(3, lv_markdown_get_block_count(md));
}

void test_markdown_bullet_list_has_indent(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "- Item 1");

    lv_obj_t * child = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(child);

    /* Default list_indent is 20, depth 1 => pad_left = 20 */
    int32_t pad = lv_obj_get_style_pad_left(child, 0);
    TEST_ASSERT_EQUAL_INT32(20, pad);
}

/* ===== Ordered List Tests ===== */

void test_markdown_ordered_list_creates_children(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "1. First\n2. Second\n3. Third");

    TEST_ASSERT_EQUAL_UINT32(3, lv_obj_get_child_count(md));
}

void test_markdown_ordered_list_block_count(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "1. First\n2. Second\n3. Third");

    TEST_ASSERT_EQUAL_UINT32(3, lv_markdown_get_block_count(md));
}

/* ===== Nested List Tests ===== */

void test_markdown_nested_bullet_list(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "- Outer\n  - Inner");

    /* 2 list items total = 2 children */
    TEST_ASSERT_EQUAL_UINT32(2, lv_obj_get_child_count(md));
}

void test_markdown_nested_list_indent_increases(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "- Outer\n  - Inner");

    lv_obj_t * outer = lv_obj_get_child(md, 0);
    lv_obj_t * inner = lv_obj_get_child(md, 1);
    TEST_ASSERT_NOT_NULL(outer);
    TEST_ASSERT_NOT_NULL(inner);

    /* Outer: depth 1 => pad_left = 20, Inner: depth 2 => pad_left = 40 */
    TEST_ASSERT_EQUAL_INT32(20, lv_obj_get_style_pad_left(outer, 0));
    TEST_ASSERT_EQUAL_INT32(40, lv_obj_get_style_pad_left(inner, 0));
}

/* ===== Mixed Lists + Other Content ===== */

void test_markdown_paragraph_then_list(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "Intro\n\n- Item 1\n- Item 2");

    /* 1 paragraph + 2 list items = 3 children */
    TEST_ASSERT_EQUAL_UINT32(3, lv_obj_get_child_count(md));
}

void test_markdown_list_then_paragraph(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "- Item\n\nAfter");

    /* 1 list item + 1 paragraph = 2 children */
    TEST_ASSERT_EQUAL_UINT32(2, lv_obj_get_child_count(md));
}

/* ===== Bullet Text Tests ===== */

void test_markdown_bullet_list_has_bullet_span(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "- Hello");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(sg);

    /* First span should be the bullet prefix, second span the text */
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, lv_spangroup_get_span_count(sg));

    lv_span_t * bullet_span = lv_spangroup_get_child(sg, 0);
    TEST_ASSERT_NOT_NULL(bullet_span);

    /* Bullet span text should contain the bullet character */
    const char * span_text = lv_span_get_text(bullet_span);
    TEST_ASSERT_NOT_NULL(span_text);
    /* Default bullet is "bullet" (UTF-8 0xE2 0x80 0xA2), check it contains that */
    TEST_ASSERT_NOT_NULL(strstr(span_text, "\xe2\x80\xa2"));
}

void test_markdown_ordered_list_has_number_span(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "1. First\n2. Second");

    /* Check first item has "1. " prefix */
    lv_obj_t * sg0 = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(sg0);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, lv_spangroup_get_span_count(sg0));

    lv_span_t * num_span0 = lv_spangroup_get_child(sg0, 0);
    const char * text0 = lv_span_get_text(num_span0);
    TEST_ASSERT_NOT_NULL(text0);
    TEST_ASSERT_NOT_NULL(strstr(text0, "1."));

    /* Check second item has "2. " prefix */
    lv_obj_t * sg1 = lv_obj_get_child(md, 1);
    TEST_ASSERT_NOT_NULL(sg1);

    lv_span_t * num_span1 = lv_spangroup_get_child(sg1, 0);
    const char * text1 = lv_span_get_text(num_span1);
    TEST_ASSERT_NOT_NULL(text1);
    TEST_ASSERT_NOT_NULL(strstr(text1, "2."));
}

/* ===== List Edge Cases ===== */

void test_markdown_single_item_list(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "- Only one");

    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
}

void test_markdown_empty_list_item(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "-\n- Second");

    /* Should handle gracefully — at least the "Second" item appears */
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1, lv_obj_get_child_count(md));
}

void test_markdown_list_item_no_indent_on_regular_paragraph(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "Normal paragraph");

    lv_obj_t * child = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(child);

    /* Regular paragraph should NOT have list indentation */
    int32_t pad = lv_obj_get_style_pad_left(child, 0);
    TEST_ASSERT_EQUAL_INT32(0, pad);
}

void test_markdown_paragraph_after_list_no_indent(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "- Item\n\nAfter");

    /* Second child is the paragraph after the list */
    lv_obj_t * para = lv_obj_get_child(md, 1);
    TEST_ASSERT_NOT_NULL(para);

    /* Paragraph after list should NOT have list indentation */
    int32_t pad = lv_obj_get_style_pad_left(para, 0);
    TEST_ASSERT_EQUAL_INT32(0, pad);
}

/* ===== Code Block Tests ===== */

void test_markdown_code_block_creates_child(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "```\ncode\n```");

    /* Fenced code block should produce 1 child (container) */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
}

void test_markdown_code_block_block_count(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "```\ncode\n```");

    TEST_ASSERT_EQUAL_UINT32(1, lv_markdown_get_block_count(md));
}

void test_markdown_code_block_has_bg_color(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "```\ncode\n```");

    lv_obj_t * child = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(child);

    /* Default code_block_bg_color is (245, 245, 245) */
    lv_color_t bg = lv_obj_get_style_bg_color(child, 0);
    TEST_ASSERT_EQUAL_UINT32(245, bg.red);
    TEST_ASSERT_EQUAL_UINT32(245, bg.green);
    TEST_ASSERT_EQUAL_UINT32(245, bg.blue);
}

void test_markdown_code_block_has_padding(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "```\ncode\n```");

    lv_obj_t * child = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(child);

    /* Default code_block_pad is 8 */
    TEST_ASSERT_EQUAL_INT32(8, lv_obj_get_style_pad_top(child, 0));
    TEST_ASSERT_EQUAL_INT32(8, lv_obj_get_style_pad_bottom(child, 0));
    TEST_ASSERT_EQUAL_INT32(8, lv_obj_get_style_pad_left(child, 0));
    TEST_ASSERT_EQUAL_INT32(8, lv_obj_get_style_pad_right(child, 0));
}

void test_markdown_code_block_preserves_text(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "```\nhello world\n```");

    lv_obj_t * container = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(container);

    /* The container should have 1 child: the label */
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1, lv_obj_get_child_count(container));
    lv_obj_t * label = lv_obj_get_child(container, 0);
    TEST_ASSERT_NOT_NULL(label);

    const char * text = lv_label_get_text(label);
    TEST_ASSERT_NOT_NULL(text);
    TEST_ASSERT_NOT_NULL(strstr(text, "hello world"));
}

void test_markdown_code_block_uses_code_font(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.code_font = LV_FONT_DEFAULT;
    lv_markdown_set_style(md, &style);

    lv_markdown_set_text(md, "```\ncode\n```");

    lv_obj_t * container = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(container);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1, lv_obj_get_child_count(container));

    lv_obj_t * label = lv_obj_get_child(container, 0);
    const lv_font_t * font = lv_obj_get_style_text_font(label, 0);
    TEST_ASSERT_EQUAL_PTR(LV_FONT_DEFAULT, font);
}

void test_markdown_code_block_multiline(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "```\nline1\nline2\nline3\n```");

    lv_obj_t * container = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(container);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1, lv_obj_get_child_count(container));

    lv_obj_t * label = lv_obj_get_child(container, 0);
    const char * text = lv_label_get_text(label);
    TEST_ASSERT_NOT_NULL(text);
    TEST_ASSERT_NOT_NULL(strstr(text, "line1"));
    TEST_ASSERT_NOT_NULL(strstr(text, "line2"));
    TEST_ASSERT_NOT_NULL(strstr(text, "line3"));
}

/* ===== Blockquote Tests ===== */

void test_markdown_blockquote_creates_child(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "> Quote");

    /* Blockquote should produce 1 top-level child (the blockquote container) */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
}

void test_markdown_blockquote_block_count(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "> Quote");

    TEST_ASSERT_EQUAL_UINT32(1, lv_markdown_get_block_count(md));
}

void test_markdown_blockquote_has_border(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "> Quote");

    lv_obj_t * child = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(child);

    /* Default blockquote_border_color is (200, 200, 200) */
    lv_color_t border = lv_obj_get_style_border_color(child, 0);
    TEST_ASSERT_EQUAL_UINT32(200, border.red);
    TEST_ASSERT_EQUAL_UINT32(200, border.green);
    TEST_ASSERT_EQUAL_UINT32(200, border.blue);

    /* Default blockquote_border_width is 3 */
    int32_t bw = lv_obj_get_style_border_width(child, 0);
    TEST_ASSERT_EQUAL_INT32(3, bw);
}

void test_markdown_blockquote_has_padding(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "> Quote");

    lv_obj_t * child = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(child);

    /* Default blockquote_pad_left is 12 */
    TEST_ASSERT_EQUAL_INT32(12, lv_obj_get_style_pad_left(child, 0));
}

void test_markdown_blockquote_contains_paragraph(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "> Quote text");

    lv_obj_t * bq = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(bq);

    /* The blockquote container should have 1 child (the inner paragraph spangroup) */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(bq));
}

/* ===== Mixed Code Block / Blockquote Tests ===== */

void test_markdown_paragraph_codeblock_paragraph(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "Before\n\n```\ncode\n```\n\nAfter");

    /* paragraph + code block + paragraph = 3 children */
    TEST_ASSERT_EQUAL_UINT32(3, lv_obj_get_child_count(md));
}

void test_markdown_blockquote_then_paragraph(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "> Quote\n\nAfter");

    /* blockquote + paragraph = 2 children */
    TEST_ASSERT_EQUAL_UINT32(2, lv_obj_get_child_count(md));
}

/* ===== Edge Case Tests for Code Blocks / Blockquotes ===== */

void test_markdown_empty_code_block(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "```\n```");

    /* Should handle gracefully: 0 or 1 child, no crash */
    uint32_t count = lv_obj_get_child_count(md);
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(1, count);
}

void test_markdown_nested_blockquote(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "> > Nested");

    /* Should produce nested containers (outer blockquote with inner blockquote) */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
    lv_obj_t * outer = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(outer);

    /* The outer blockquote should contain the inner blockquote */
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1, lv_obj_get_child_count(outer));
}

/* ===== Cross-Feature Tests ===== */

void test_markdown_list_inside_blockquote(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "> - Item 1\n> - Item 2");

    /* 1 top-level child (blockquote container) */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
    lv_obj_t * bq = lv_obj_get_child(md, 0);
    /* List items should be inside the blockquote */
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, lv_obj_get_child_count(bq));
}

void test_markdown_code_block_inside_blockquote(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "> ```\n> code\n> ```");

    /* 1 top-level child (blockquote container) */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
    lv_obj_t * bq = lv_obj_get_child(md, 0);
    /* Code block container should be inside the blockquote */
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1, lv_obj_get_child_count(bq));
}

/* ===== Line Spacing Tests ===== */

void test_markdown_line_spacing_applied_to_paragraph(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "Hello world");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(sg);

    /* Default line_spacing is 4 */
    int32_t ls = lv_obj_get_style_text_line_space(sg, 0);
    TEST_ASSERT_EQUAL_INT32(4, ls);
}

void test_markdown_line_spacing_applied_to_heading(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "# Heading");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(sg);

    int32_t ls = lv_obj_get_style_text_line_space(sg, 0);
    TEST_ASSERT_EQUAL_INT32(4, ls);
}

void test_markdown_line_spacing_applied_to_tight_list(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "- Item 1\n- Item 2");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(sg);

    int32_t ls = lv_obj_get_style_text_line_space(sg, 0);
    TEST_ASSERT_EQUAL_INT32(4, ls);
}

void test_markdown_custom_line_spacing(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.line_spacing = 12;
    lv_markdown_set_style(md, &style);

    lv_markdown_set_text(md, "Hello world");

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(sg);

    int32_t ls = lv_obj_get_style_text_line_space(sg, 0);
    TEST_ASSERT_EQUAL_INT32(12, ls);
}

/* ===== Unicode / Edge Case Tests ===== */

void test_markdown_unicode_heading(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "# H\xc3\xa9llo W\xc3\xb6rld");

    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
    TEST_ASSERT_EQUAL_UINT32(1, lv_markdown_get_block_count(md));
}

void test_markdown_very_long_paragraph(void)
{
    /* Generate a ~500 char string */
    char long_text[520];
    memset(long_text, 'A', 500);
    long_text[500] = '\0';

    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, long_text);

    /* Should produce 1 child without crash */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
}

void test_markdown_many_blocks(void)
{
    /* Build markdown with 20 paragraphs separated by blank lines */
    char buf[512];
    int pos = 0;
    for(int i = 0; i < 20; i++) {
        if(i > 0) {
            buf[pos++] = '\n';
            buf[pos++] = '\n';
        }
        pos += snprintf(buf + pos, sizeof(buf) - pos, "Para %d", i + 1);
    }
    buf[pos] = '\0';

    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, buf);

    TEST_ASSERT_EQUAL_UINT32(20, lv_markdown_get_block_count(md));
    TEST_ASSERT_EQUAL_UINT32(20, lv_obj_get_child_count(md));
}

void test_markdown_escaped_asterisks(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "Text with \\*escaped\\* asterisks");

    /* Should render as 1 paragraph, no bold */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    uint32_t span_count = lv_spangroup_get_span_count(sg);

    /* Verify no bold formatting on any span */
    for(uint32_t i = 0; i < span_count; i++) {
        lv_span_t * span = lv_spangroup_get_child(sg, i);
        lv_style_value_t val;
        lv_style_res_t res = get_span_style_prop(span, LV_STYLE_TEXT_LETTER_SPACE, &val);
        if(res == LV_STYLE_RES_FOUND) {
            /* If letter_space is set, it shouldn't be the bold fallback value */
            TEST_ASSERT_NOT_EQUAL(1, val.num);
        }
    }
}

void test_markdown_mixed_list_types(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "- Bullet\n\n1. Numbered");

    /* Bullet list item + ordered list item = 2 children */
    TEST_ASSERT_EQUAL_UINT32(2, lv_obj_get_child_count(md));
}

void test_markdown_heading_after_list(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "- Item\n\n# Heading");

    /* List item + heading = 2 children */
    TEST_ASSERT_EQUAL_UINT32(2, lv_obj_get_child_count(md));
    TEST_ASSERT_EQUAL_UINT32(2, lv_markdown_get_block_count(md));
}

void test_markdown_bold_in_list_item(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "- **Bold item**");

    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));

    lv_obj_t * sg = lv_obj_get_child(md, 0);
    /* Should have bullet prefix span + bold text span (at minimum) */
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(2, lv_spangroup_get_span_count(sg));

    /* The text span (after bullet) should have bold formatting */
    uint32_t last = lv_spangroup_get_span_count(sg) - 1;
    lv_span_t * bold_span = lv_spangroup_get_child(sg, last);
    lv_style_value_t val;
    lv_style_res_t res = get_span_style_prop(bold_span, LV_STYLE_TEXT_LETTER_SPACE, &val);
    TEST_ASSERT_EQUAL(LV_STYLE_RES_FOUND, res);
    TEST_ASSERT_EQUAL_INT32(1, val.num);
}

void test_markdown_code_block_with_backticks(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    /* Use 4-backtick fence to contain triple backticks */
    lv_markdown_set_text(md, "````\n```\ncode\n```\n````");

    /* Should produce 1 code block child */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));

    lv_obj_t * container = lv_obj_get_child(md, 0);
    TEST_ASSERT_NOT_NULL(container);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1, lv_obj_get_child_count(container));

    lv_obj_t * label = lv_obj_get_child(container, 0);
    const char * text = lv_label_get_text(label);
    TEST_ASSERT_NOT_NULL(text);
    TEST_ASSERT_NOT_NULL(strstr(text, "```"));
}

void test_markdown_empty_blockquote(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, ">");

    /* Should handle gracefully: 0 or 1 child, no crash */
    uint32_t count = lv_obj_get_child_count(md);
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(1, count);
}

void test_markdown_multiple_hrs(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "---\n\n---\n\n---");

    TEST_ASSERT_EQUAL_UINT32(3, lv_obj_get_child_count(md));
    TEST_ASSERT_EQUAL_UINT32(3, lv_markdown_get_block_count(md));
}

/* ===== Style Reconfiguration Edge Cases ===== */

void test_markdown_repeated_set_style(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "Hello");

    lv_markdown_style_t style;
    lv_markdown_style_init(&style);

    /* Call set_style 3 times in a row — no crash, correct final state */
    style.paragraph_spacing = 5;
    lv_markdown_set_style(md, &style);

    style.paragraph_spacing = 15;
    lv_markdown_set_style(md, &style);

    style.paragraph_spacing = 25;
    lv_markdown_set_style(md, &style);

    /* Should still have 1 child, correct text */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
    TEST_ASSERT_EQUAL_STRING("Hello", lv_markdown_get_text(md));
}

void test_markdown_set_text_set_style_set_text(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());

    /* Set text -> set style -> set text (full cycle) */
    lv_markdown_set_text(md, "First");
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));

    lv_markdown_style_t style;
    lv_markdown_style_init(&style);
    style.hr_height = 3;
    lv_markdown_set_style(md, &style);
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));

    lv_markdown_set_text(md, "Second\n\nThird");
    TEST_ASSERT_EQUAL_UINT32(2, lv_obj_get_child_count(md));
    TEST_ASSERT_EQUAL_STRING("Second\n\nThird", lv_markdown_get_text(md));
}

void test_markdown_set_style_null(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "Hello");

    /* Should not crash */
    lv_markdown_set_style(md, NULL);

    /* Content should be unchanged */
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));
    TEST_ASSERT_EQUAL_STRING("Hello", lv_markdown_get_text(md));
}

/* ===== API Completeness / NULL Safety Tests ===== */

void test_markdown_get_text_null_obj_data(void)
{
    /* Create a plain LVGL object (not a markdown widget) with NULL user data */
    lv_obj_t * obj = lv_obj_create(lv_screen_active());
    lv_obj_set_user_data(obj, NULL);

    /* Should return NULL, not crash */
    TEST_ASSERT_NULL(lv_markdown_get_text(obj));
}

void test_markdown_get_block_count_null_obj_data(void)
{
    lv_obj_t * obj = lv_obj_create(lv_screen_active());
    lv_obj_set_user_data(obj, NULL);

    /* Should return 0, not crash */
    TEST_ASSERT_EQUAL_UINT32(0, lv_markdown_get_block_count(obj));
}

void test_markdown_set_text_null_obj_data(void)
{
    lv_obj_t * obj = lv_obj_create(lv_screen_active());
    lv_obj_set_user_data(obj, NULL);

    /* Should not crash */
    lv_markdown_set_text(obj, "test");
}

void test_markdown_set_text_static_null_clears(void)
{
    lv_obj_t * md = lv_markdown_create(lv_screen_active());
    lv_markdown_set_text(md, "First");
    TEST_ASSERT_EQUAL_UINT32(1, lv_obj_get_child_count(md));

    lv_markdown_set_text_static(md, NULL);
    TEST_ASSERT_EQUAL_UINT32(0, lv_obj_get_child_count(md));
    TEST_ASSERT_NULL(lv_markdown_get_text(md));
}

/* ===== Unity test runner ===== */

int main(void)
{
    UNITY_BEGIN();

    /* Creation */
    RUN_TEST(test_markdown_create_returns_valid_obj);
    RUN_TEST(test_markdown_create_starts_with_no_children);
    RUN_TEST(test_markdown_get_text_returns_null_initially);
    RUN_TEST(test_markdown_get_block_count_zero_initially);

    /* Plain paragraphs */
    RUN_TEST(test_markdown_plain_paragraph_creates_one_child);
    RUN_TEST(test_markdown_plain_paragraph_block_count);
    RUN_TEST(test_markdown_set_text_null_clears_content);
    RUN_TEST(test_markdown_set_text_empty_string);
    RUN_TEST(test_markdown_set_text_replaces_previous);
    RUN_TEST(test_markdown_two_paragraphs_creates_two_children);
    RUN_TEST(test_markdown_get_text_returns_set_text);
    RUN_TEST(test_markdown_set_text_static_works);

    /* Headings */
    RUN_TEST(test_markdown_h1_creates_one_child);
    RUN_TEST(test_markdown_h1_block_count);
    RUN_TEST(test_markdown_h2_creates_one_child);
    RUN_TEST(test_markdown_all_heading_levels);
    RUN_TEST(test_markdown_heading_uses_style_font);
    RUN_TEST(test_markdown_heading_uses_style_color);

    /* Horizontal rules */
    RUN_TEST(test_markdown_hr_creates_one_child);
    RUN_TEST(test_markdown_hr_block_count);
    RUN_TEST(test_markdown_hr_uses_style_height);
    RUN_TEST(test_markdown_hr_uses_style_color);

    /* Mixed blocks */
    RUN_TEST(test_markdown_heading_then_paragraph);
    RUN_TEST(test_markdown_paragraph_hr_paragraph);

    /* Style configuration */
    RUN_TEST(test_markdown_set_style_re_renders);
    RUN_TEST(test_markdown_set_style_before_text);

    /* Fallbacks */
    RUN_TEST(test_markdown_heading_null_font_uses_body_font);

    /* Block spacing */
    RUN_TEST(test_markdown_first_block_no_top_margin);
    RUN_TEST(test_markdown_second_block_has_spacing);

    /* Edge cases */
    RUN_TEST(test_markdown_only_newlines);
    RUN_TEST(test_markdown_single_newline_no_break);

    /* Bold */
    RUN_TEST(test_markdown_bold_creates_spans);
    RUN_TEST(test_markdown_bold_with_font);
    RUN_TEST(test_markdown_bold_fallback_letter_space);

    /* Italic */
    RUN_TEST(test_markdown_italic_with_font);
    RUN_TEST(test_markdown_italic_fallback_underline);

    /* Bold + Italic */
    RUN_TEST(test_markdown_bold_italic_with_font);
    RUN_TEST(test_markdown_bold_italic_fallback);

    /* Inline code */
    RUN_TEST(test_markdown_inline_code_with_font);
    RUN_TEST(test_markdown_inline_code_color);
    RUN_TEST(test_markdown_inline_code_fallback_body_font);

    /* Mixed formatting */
    RUN_TEST(test_markdown_multiple_formats_in_paragraph);

    /* Formatting edge cases */
    RUN_TEST(test_markdown_plain_spans_have_no_formatting);
    RUN_TEST(test_markdown_bold_inside_heading);

    /* Bullet lists */
    RUN_TEST(test_markdown_bullet_list_creates_children);
    RUN_TEST(test_markdown_bullet_list_block_count);
    RUN_TEST(test_markdown_bullet_list_has_indent);

    /* Ordered lists */
    RUN_TEST(test_markdown_ordered_list_creates_children);
    RUN_TEST(test_markdown_ordered_list_block_count);

    /* Nested lists */
    RUN_TEST(test_markdown_nested_bullet_list);
    RUN_TEST(test_markdown_nested_list_indent_increases);

    /* Lists mixed with other content */
    RUN_TEST(test_markdown_paragraph_then_list);
    RUN_TEST(test_markdown_list_then_paragraph);

    /* Bullet/number text */
    RUN_TEST(test_markdown_bullet_list_has_bullet_span);
    RUN_TEST(test_markdown_ordered_list_has_number_span);

    /* List edge cases */
    RUN_TEST(test_markdown_single_item_list);
    RUN_TEST(test_markdown_empty_list_item);
    RUN_TEST(test_markdown_list_item_no_indent_on_regular_paragraph);
    RUN_TEST(test_markdown_paragraph_after_list_no_indent);

    /* Code blocks */
    RUN_TEST(test_markdown_code_block_creates_child);
    RUN_TEST(test_markdown_code_block_block_count);
    RUN_TEST(test_markdown_code_block_has_bg_color);
    RUN_TEST(test_markdown_code_block_has_padding);
    RUN_TEST(test_markdown_code_block_preserves_text);
    RUN_TEST(test_markdown_code_block_uses_code_font);
    RUN_TEST(test_markdown_code_block_multiline);

    /* Blockquotes */
    RUN_TEST(test_markdown_blockquote_creates_child);
    RUN_TEST(test_markdown_blockquote_block_count);
    RUN_TEST(test_markdown_blockquote_has_border);
    RUN_TEST(test_markdown_blockquote_has_padding);
    RUN_TEST(test_markdown_blockquote_contains_paragraph);

    /* Mixed code block / blockquote */
    RUN_TEST(test_markdown_paragraph_codeblock_paragraph);
    RUN_TEST(test_markdown_blockquote_then_paragraph);

    /* Code block / blockquote edge cases */
    RUN_TEST(test_markdown_empty_code_block);
    RUN_TEST(test_markdown_nested_blockquote);

    /* Cross-feature */
    RUN_TEST(test_markdown_list_inside_blockquote);
    RUN_TEST(test_markdown_code_block_inside_blockquote);

    /* Line spacing */
    RUN_TEST(test_markdown_line_spacing_applied_to_paragraph);
    RUN_TEST(test_markdown_line_spacing_applied_to_heading);
    RUN_TEST(test_markdown_line_spacing_applied_to_tight_list);
    RUN_TEST(test_markdown_custom_line_spacing);

    /* Unicode / edge cases */
    RUN_TEST(test_markdown_unicode_heading);
    RUN_TEST(test_markdown_very_long_paragraph);
    RUN_TEST(test_markdown_many_blocks);
    RUN_TEST(test_markdown_escaped_asterisks);
    RUN_TEST(test_markdown_mixed_list_types);
    RUN_TEST(test_markdown_heading_after_list);
    RUN_TEST(test_markdown_bold_in_list_item);
    RUN_TEST(test_markdown_code_block_with_backticks);
    RUN_TEST(test_markdown_empty_blockquote);
    RUN_TEST(test_markdown_multiple_hrs);

    /* Style reconfiguration edge cases */
    RUN_TEST(test_markdown_repeated_set_style);
    RUN_TEST(test_markdown_set_text_set_style_set_text);
    RUN_TEST(test_markdown_set_style_null);

    /* API completeness / NULL safety */
    RUN_TEST(test_markdown_get_text_null_obj_data);
    RUN_TEST(test_markdown_get_block_count_null_obj_data);
    RUN_TEST(test_markdown_set_text_null_obj_data);
    RUN_TEST(test_markdown_set_text_static_null_clears);

    return UNITY_END();
}
