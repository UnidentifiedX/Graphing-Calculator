#pragma once
#include <stdint.h>
#include <stddef.h>
#include "fonts.h"

enum class LineStyle {
    Solid,
    Dashed,
    Dotted
};

struct ST7789 {
    static constexpr int WIDTH = 320;
    static constexpr int HEIGHT = 240;

    ST7789(int pin_cs, int pin_dc, int pin_reset, int pin_sck, int pin_mosi, int pin_pwm);

    void init();
    void setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void fillScreen(uint16_t color);
    void drawHorizontalLine(uint16_t x, uint16_t y, uint16_t w, uint16_t t, uint16_t color, LineStyle style, uint16_t bg = 0xFFFF);
    void drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint8_t scale, uint16_t bg = 0xFFFF);
    void drawString(uint16_t x, uint16_t y, const char* str, uint16_t color, uint8_t scale, uint16_t bg = 0xFFFF);
    void drawParagraph(uint16_t x, uint16_t y, uint16_t w, const char* str, uint16_t color, FontSize size, uint16_t bg = 0xFFFF);
    void setBrightness(uint8_t brightness); // 0-255

private:
    int _cs, _dc, _reset, _sck, _mosi, _pwm;

    void pinLow(int pin);
    void pinHigh(int pin);
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void writeData16(uint16_t data);
    void hardwareReset();
    void spiInit();
};

inline constexpr uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}