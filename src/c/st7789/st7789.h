#pragma once

#include <stdint.h>

#define ST7789_WIDTH 320
#define ST7789_HEIGHT 240

typedef enum {
    LINE_STYLE_SOLID,
    LINE_STYLE_DASHED,
    LINE_STYLE_DOTTED
} LineStyle;

typedef struct {
    uint8_t _cs;
    uint8_t _dc;
    uint8_t _reset;
    uint8_t _sck;
    uint8_t _mosi;
    uint8_t _pwm;
} ST7789;

void st7789_init(ST7789 *display);
void st7789_set_window(ST7789 *display, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void st7789_draw_pixel(ST7789 *display, uint16_t x, uint16_t y, uint16_t color);
void st7789_fill_rect(ST7789 *display, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void st7789_fill_screen(ST7789 *display, uint16_t color);
void st7789_draw_horizontal_line(ST7789 *display, uint16_t x, uint16_t y, uint16_t w, uint16_t t, uint16_t color, LineStyle style, uint16_t bg);
void st7789_draw_char(ST7789 *display, uint16_t x, uint16_t y, char c, uint16_t color, uint8_t scale, uint16_t bg);
void st7789_draw_string(ST7789 *display, uint16_t x, uint16_t y, const char* str, uint16_t color, uint8_t scale, uint16_t bg);
void st7789_set_brightness(ST7789 *display, uint8_t brightness);

static inline uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}