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

    return UNITY_END();
}
