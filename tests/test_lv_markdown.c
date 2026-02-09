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

    return UNITY_END();
}
