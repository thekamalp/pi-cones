// font glyph

#ifndef __FONT_H
#define __FONT_H

#ifdef WIN32
#include <stdlib.h>
#include <stdbool.h>
typedef unsigned int uint;
#else
#include "pico/stdlib.h"
#endif
#include <string.h>
#include <stdint.h>

typedef struct FONT_T_ {
    uint8_t width;
    uint8_t height;
    uint8_t stride;
    uint8_t start;
    const uint8_t* glyphs;
} FONT_T;

#define TEXTBOX_MAX_LINES 8

typedef struct {
    const FONT_T* font;
    const char* text_lines[TEXTBOX_MAX_LINES];
    uint16_t start_x, start_y;
    uint16_t end_x, end_y;
    uint16_t offset_x, offset_y;
    uint16_t char_index, line;
    bool line_done;
    uint8_t cur_glyph;
} TEXTBOX_T;

#define NO_BACKGROUND 0x0020

#define MAX_FONT 8

#define FONT_ID_CONSOLE_5X8 0
#define FONT_ID_AIXOID9_F16 1
#define FONT_ID_BLKBOARD_F16 2
#define FONT_ID_BULKY_F16 3
#define FONT_ID_HOLLOW_F16 4
#define FONT_ID_MEDIEVAL_F16 5
#define FONT_ID_SCRAWL2_F16 6
#define FONT_ID_SCRIPT2_F14 7

extern const FONT_T* font[MAX_FONT];

extern const FONT_T console_font_5x8;
extern const FONT_T AIXOID9_F16;
extern const FONT_T BLKBOARD_F16;
extern const FONT_T BULKY_F16;
extern const FONT_T HOLLOW_F16;
extern const FONT_T MEDIEVAL_F16;
extern const FONT_T SCRAWL2_F16;
extern const FONT_T SCRIPT2_F14;

void textbox_set_font(TEXTBOX_T* tb, const FONT_T* font);
void textbox_set_text(TEXTBOX_T* tb, const char* text, uint8_t line);
void textbox_set_position(TEXTBOX_T* tb, uint x, uint y);
void textbox_reset(TEXTBOX_T* tb);
void textbox_set_cur_glyph(TEXTBOX_T* tb);

inline uint textbox_in_text(TEXTBOX_T* tb, uint x, uint y)
{
    bool in_text = x >= tb->start_x && x < tb->end_x&& y >= tb->start_y && y < tb->end_y;
    if (in_text) {
        if (!tb->line_done) {
            // We're in the text bounding box
            // Now check if we hit the glyph
            in_text = (tb->cur_glyph << tb->offset_x) & 0x80;
        } else {
            in_text = false;
        }
        if (x == tb->end_x - 1) {
            // end of row
            tb->offset_x = 0;
            tb->char_index = 0;
            tb->offset_y++;
            tb->line_done = false;
            if (tb->offset_y >= tb->font->height) {
                // end of a line
                tb->offset_y = 0;
                tb->line++;
            }
            textbox_set_cur_glyph(tb);
        } else {
            tb->offset_x++;
            if (tb->offset_x >= tb->font->width) {
                // end of a character
                tb->offset_x = 0;
                tb->char_index++;
                textbox_set_cur_glyph(tb);
            }
        }
    }
    return (in_text) ? tb->line + 1 : 0;
}

#endif
