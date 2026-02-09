# lv_markdown

A lightweight markdown rendering widget for [LVGL](https://lvgl.io/) 9.x. Parses markdown with [md4c](https://github.com/mity/md4c) and renders it as native LVGL widgets (spangroups, labels, containers).

**MIT License** | **C99** | **Zero dependencies beyond LVGL**

## Features

| Markdown | Rendering |
|----------|-----------|
| `# Heading` through `###### Heading` | Spangroup with configurable font/color per level |
| `**bold**` | Dedicated font or faux bold (letter spacing +1px) |
| `*italic*` | Dedicated font or underline decoration |
| `***bold italic***` | Dedicated font or combined fallbacks |
| `` `inline code` `` | Configurable font + color |
| Fenced code blocks | Container with background + label |
| `> Blockquotes` | Container with left border (supports nesting) |
| `- Bullet lists` | Indented spangroup with bullet prefix |
| `1. Ordered lists` | Indented spangroup with number prefix |
| Nested lists | Increasing indentation per level |
| `---` Horizontal rules | Thin colored bar |
| Paragraphs | Spangroups with configurable spacing |

All fonts are optional. When not provided, fallback strategies kick in automatically.

## Quick Start

```c
#include "lv_markdown.h"

/* Create the widget (grows to fit content) */
lv_obj_t * md = lv_markdown_create(parent);

/* Set markdown text */
lv_markdown_set_text(md, "# Hello World\n\nThis is **bold** and *italic*.");
```

The widget uses `LV_SIZE_CONTENT` for height and `LV_PCT(100)` for width. Wrap it in a scrollable container if the content may exceed the screen.

### Static Text (Zero-Copy)

```c
/* Text must remain valid for the widget's lifetime */
static const char * notes = "# Release Notes\n\n- Bug fixes\n- New features";
lv_markdown_set_text_static(md, notes);
```

### Custom Styling

```c
lv_markdown_style_t style;
lv_markdown_style_init(&style);  /* fills sensible defaults */

/* Customize what you need */
style.body_font = &my_body_font;
style.heading_font[0] = &my_h1_font;      /* H1 */
style.heading_color[0] = lv_color_hex(0x1a73e8);
style.bold_font = &my_bold_font;           /* NULL = faux bold fallback */
style.code_font = &my_mono_font;           /* NULL = body_font fallback */
style.code_block_bg_color = lv_color_hex(0xf5f5f5);
style.paragraph_spacing = 12;

lv_markdown_set_style(md, &style);
```

## API Reference

```c
/* Create / destroy */
lv_obj_t * lv_markdown_create(lv_obj_t * parent);
/* Destroy: just delete the LVGL object normally. Cleanup is automatic. */

/* Set content */
void lv_markdown_set_text(lv_obj_t * obj, const char * text);         /* copies text */
void lv_markdown_set_text_static(lv_obj_t * obj, const char * text);  /* zero-copy */

/* Configure appearance */
void lv_markdown_set_style(lv_obj_t * obj, const lv_markdown_style_t * style);

/* Query */
const char * lv_markdown_get_text(lv_obj_t * obj);
uint32_t lv_markdown_get_block_count(lv_obj_t * obj);
```

## Style Configuration

Call `lv_markdown_style_init()` first, then override what you need:

| Field | Default | Description |
|-------|---------|-------------|
| `body_font` | `LV_FONT_DEFAULT` | Base font for all text |
| `body_color` | Black | Body text color |
| `heading_font[0..5]` | `NULL` (body_font) | Font per heading level (H1=0, H6=5) |
| `heading_color[0..5]` | Black | Color per heading level |
| `bold_font` | `NULL` (letter space) | Bold font, or faux bold fallback |
| `italic_font` | `NULL` (underline) | Italic font, or underline fallback |
| `bold_italic_font` | `NULL` (combined) | Bold-italic font, or combined fallbacks |
| `code_font` | `NULL` (body_font) | Monospace font for code |
| `code_color` | Black | Inline code text color |
| `code_block_bg_color` | `#f5f5f5` | Code block background |
| `code_block_corner_radius` | 4 | Code block corner radius |
| `code_block_pad` | 8 | Code block padding (all sides) |
| `blockquote_border_color` | `#c8c8c8` | Blockquote left border color |
| `blockquote_border_width` | 3 | Blockquote left border width |
| `blockquote_pad_left` | 12 | Blockquote left padding |
| `hr_color` | `#c8c8c8` | Horizontal rule color |
| `hr_height` | 1 | Horizontal rule height |
| `paragraph_spacing` | 10 | Vertical gap between blocks |
| `line_spacing` | 4 | Line spacing within a block |
| `list_indent` | 20 | Indent per list nesting level |
| `list_bullet` | `"•"` | Bullet character for unordered lists |

## Font Fallback Strategy

You don't need to provide every font. The widget handles missing fonts gracefully:

- **Bold**: Uses `bold_font` if set, otherwise increases letter spacing by 1px (faux bold)
- **Italic**: Uses `italic_font` if set, otherwise renders with underline decoration
- **Bold+Italic**: Tries `bold_italic_font` > `bold_font + underline` > `italic_font + letter space` > both fallbacks
- **Code**: Uses `code_font` if set, otherwise falls back to `body_font`
- **Headings**: Uses `heading_font[N]` if set, otherwise falls back to `body_font`

## Building

### As part of a project

Add to your build system:

```makefile
LV_MARKDOWN_DIR := lib/lv_markdown
LV_MARKDOWN_SRCS := $(wildcard $(LV_MARKDOWN_DIR)/src/*.c) $(LV_MARKDOWN_DIR)/deps/md4c/md4c.c
LV_MARKDOWN_OBJS := $(patsubst $(LV_MARKDOWN_DIR)/%.c,$(OBJ_DIR)/lv_markdown/%.o,$(LV_MARKDOWN_SRCS))

# Add to your include paths
INCLUDES += -I$(LV_MARKDOWN_DIR)/src -I$(LV_MARKDOWN_DIR)/deps/md4c
```

Then compile the `.c` files with your project's LVGL flags and link the objects.

### Standalone tests

```bash
# Requires LVGL source tree nearby
make test LVGL_PATH=../lvgl

# Run tests
./build/test_lv_markdown
# 98 Tests 0 Failures 0 Ignored
```

## Known Limitations

- **Inline code background color** (`code_bg_color`): LVGL spangroups don't support per-span backgrounds. Inline code gets font + color styling only. Code *blocks* have full background support.
- **Faux bold**: Uses letter spacing, not text shadow (LVGL spans don't support per-span shadow). Provide a real bold font for best results.
- **No images**: Image rendering is not supported.
- **No links**: Link syntax is parsed but rendered as plain text (no click handling).
- **No tables**: Table syntax renders as plain text.
- **List nesting**: Maximum 16 levels deep.

## Architecture

```
Markdown text
    │
    ▼
┌─────────┐     ┌──────────────────┐
│  md4c   │────▶│  SAX callbacks   │
│ parser  │     │  (enter/leave    │
└─────────┘     │   block/span)    │
                └──────────────────┘
                         │
                         ▼
                ┌──────────────────┐
                │  LVGL widgets    │
                │                  │
                │  Paragraph ──▶ spangroup
                │  Heading   ──▶ spangroup (styled)
                │  Code block ─▶ container + label
                │  Blockquote ─▶ container (border)
                │  List item  ──▶ spangroup (indented)
                │  HR         ──▶ styled obj
                └──────────────────┘
```

Each markdown block becomes a single LVGL widget. Inline formatting (bold, italic, code) creates styled spans within spangroups. The widget uses a flex column layout, so blocks stack vertically.

## License

MIT License. See [LICENSE](LICENSE).
