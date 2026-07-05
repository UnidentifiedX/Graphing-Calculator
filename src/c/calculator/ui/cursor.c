#include "cursor.h"

void init_cursor(Cursor* cursor, ST7789* display, uint16_t h, uint16_t t) {
    cursor->display = display;
    cursor->x = 0;
    cursor->y = 0;
    cursor->h = h;
    cursor->t = t;
    cursor->visible = false;
    cursor->enabled = false;
}

static void draw_cursor(Cursor* cursor, uint16_t color) {
    st7789_draw_vertical_line(cursor->display, cursor->x, cursor->y, cursor->h, cursor->t, color, LINE_STYLE_SOLID, color565(255, 255, 255));
}

static void erase_cursor(Cursor* cursor) {
    draw_cursor(cursor, color565(255, 255, 255)); // White (background color)
}

void toggle_cursor(Cursor* cursor) {
    if (!cursor->enabled) {
        return; // Do nothing if the cursor is not enabled
    }

    cursor->visible ? erase_cursor(cursor) : draw_cursor(cursor, color565(0, 0, 0)); // Black (cursor color)
    cursor->visible = !cursor->visible;
}

void enable_cursor(Cursor* cursor) {
    cursor->enabled = true;
    cursor->visible = false; // Start with the cursor invisible
}

void disable_cursor(Cursor* cursor) {
    if (cursor->visible) {
        erase_cursor(cursor);
    }
    cursor->enabled = false;
    cursor->visible = false;
}

void set_cursor_position(Cursor* cursor, uint16_t x, uint16_t y) {
    cursor->x = x;
    cursor->y = y;
}