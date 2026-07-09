#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "st7789/st7789.h"

typedef struct {
    ST7789* display;
    uint16_t x;
    uint16_t y;
    uint16_t h;
    uint16_t t;
    bool visible;
    bool enabled;
} Cursor;

void init_cursor(Cursor* cursor, ST7789* display, uint16_t h, uint16_t t);
void toggle_cursor(Cursor* cursor);
void set_cursor_position(Cursor* cursor, uint16_t x, uint16_t y);
// cursor is drawn from top down
void enable_cursor(Cursor* cursor);
void disable_cursor(Cursor* cursor);