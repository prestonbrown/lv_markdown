# SPDX-License-Identifier: MIT
#
# lv_markdown build system
#
# Usage:
#   make test LVGL_PATH=/path/to/lvgl    # Build and run tests
#   make test-build LVGL_PATH=...        # Build tests only
#   make clean                           # Clean build artifacts
#

# LVGL path is required for tests
LVGL_PATH ?= ../lvgl

# Resolve to absolute path to avoid ../ in object file paths
LVGL_ABS := $(abspath $(LVGL_PATH))

# Directories
BUILD_DIR   := build
SRC_DIR     := src
DEPS_DIR    := deps
TEST_DIR    := tests

# Compiler settings
CC      := cc
CFLAGS  := -std=c99 -Wall -Wextra -Wpedantic -g -O0
CFLAGS  += -Wno-unused-parameter

# Include paths:
#   - tests/ dir first (so tests/lv_conf.h is found before any project lv_conf.h)
#   - src/ for lv_markdown headers
#   - deps/ for md4c
#   - LVGL paths
CFLAGS  += -I$(TEST_DIR)
CFLAGS  += -I$(SRC_DIR)
CFLAGS  += -I$(DEPS_DIR)/md4c
CFLAGS  += -I$(LVGL_ABS)/..
CFLAGS  += -I$(LVGL_ABS)
CFLAGS  += -I$(LVGL_ABS)/tests

# LV_CONF_PATH so LVGL picks up our test config (needs quotes for #include)
CFLAGS  += '-DLV_CONF_PATH="lv_conf.h"'

# Enable test builds (Unity is guarded by this)
CFLAGS  += -DLV_BUILD_TEST=1

# --- Source files ---

# lv_markdown sources
LV_MD_SRCS := $(wildcard $(SRC_DIR)/*.c)

# md4c
MD4C_SRCS  := $(DEPS_DIR)/md4c/md4c.c

# Unity test framework (from LVGL)
UNITY_SRCS := $(LVGL_ABS)/tests/unity/unity.c

# LVGL sources (just src/, no demos/examples)
LVGL_SRCS  := $(shell find $(LVGL_ABS)/src -name '*.c' 2>/dev/null)

# Test sources
TEST_SRCS  := $(wildcard $(TEST_DIR)/test_*.c)

# --- Object files ---
ALL_SRCS   := $(LV_MD_SRCS) $(MD4C_SRCS) $(UNITY_SRCS) $(LVGL_SRCS) $(TEST_SRCS)
ALL_OBJS   := $(patsubst %.c,$(BUILD_DIR)/%.o,$(ALL_SRCS))

# Test binary
TEST_BIN   := $(BUILD_DIR)/test_lv_markdown

# --- Targets ---

.PHONY: test test-build clean

test: test-build
	@echo "Running lv_markdown tests..."
	@./$(TEST_BIN)

test-build: $(TEST_BIN)
	@echo "Build complete: $(TEST_BIN)"

$(TEST_BIN): $(ALL_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ -lm

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
