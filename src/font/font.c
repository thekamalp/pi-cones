#include "font.h"
#include "font/console_font_5x8.h"
#include "font/AIXOID9.h"
#include "font/BLKBOARD.h"
#include "font/BULKY.h"
#include "font/HOLLOW.h"
#include "font/MEDIEVAL.h"
#include "font/SCRAWL2.h"
#include "font/SCRIPT2.h"

const FONT_T console_font_5x8 = { 5, 8, 8, 0, console_font_5x8_glyphs };
const FONT_T AIXOID9_F16 = { 8, 16, 16, 0, AIXOID9_F16_glyphs };
const FONT_T BLKBOARD_F16 = { 8, 16, 16, 0, BLKBOARD_F16_glyphs };
const FONT_T BULKY_F16 = { 8, 16, 16, 0, BULKY_F16_glyphs };
const FONT_T HOLLOW_F16 = { 8, 16, 16, 0, HOLLOW_F16_glyphs };
const FONT_T MEDIEVAL_F16 = { 8, 16, 16, 0, MEDIEVAL_F16_glyphs };
const FONT_T SCRAWL2_F16 = { 8, 16, 16, 0, SCRAWL2_F16_glyphs };
const FONT_T SCRIPT2_F14 = { 8, 14, 14, 0, SCRIPT2_F14_glyphs };

const FONT_T* font[MAX_FONT] = {
    &console_font_5x8,
    &AIXOID9_F16,
    &BLKBOARD_F16,
    &BULKY_F16,
    &HOLLOW_F16,
    &MEDIEVAL_F16,
    &SCRAWL2_F16,
    &SCRIPT2_F14
};

void textbox_set_size(TEXTBOX_T* tb)
{
    if (tb->font == NULL) return;

    uint32_t i;
    uint32_t text_width = 0, max_width = 0;
    for (i = 0; i < TEXTBOX_MAX_LINES; i++) {
        if (tb->text_lines[i] == NULL) break;
        text_width = strlen(tb->text_lines[i]);
        if (text_width > max_width) max_width = text_width;
    }
    tb->end_x = tb->start_x + max_width * tb->font->width;
    tb->end_y = tb->start_y + i * tb->font->height;
}

void textbox_set_cur_glyph(TEXTBOX_T* tb)
{
    int text_index = 0;
    if (tb->text_lines[tb->line] == NULL) return;
    text_index = tb->text_lines[tb->line][tb->char_index];
    if (text_index == 0) tb->line_done = true;
    text_index *= tb->font->stride;
    tb->cur_glyph = tb->font->glyphs[text_index + tb->offset_y];
}

void textbox_set_font(TEXTBOX_T* tb, const FONT_T* font)
{
    tb->font = font;
    textbox_set_size(tb);
}

void textbox_set_text(TEXTBOX_T* tb, const char* text, uint8_t line)
{
    if (line < TEXTBOX_MAX_LINES) {
        tb->text_lines[line] = text;
        textbox_set_size(tb);
    }
}

void textbox_set_position(TEXTBOX_T* tb, uint x, uint y)
{
    tb->end_x -= tb->start_x;
    tb->end_y -= tb->start_y;
    tb->start_x = x;
    tb->start_y = y;
    tb->end_x += tb->start_x;
    tb->end_y += tb->start_y;
}

void textbox_reset(TEXTBOX_T* tb)
{
    tb->offset_x = 0;
    tb->offset_y = 0;
    tb->char_index = 0;
    tb->line = 0;
    tb->line_done = false;
    textbox_set_cur_glyph(tb);
}

