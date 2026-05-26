#include "st7789.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "fonts.h"
#include <cstdint>
#include <iostream>

#define ST7789_SPI_PORT spi0

#define ST7789_CMD_SWRESET 0x01 // Software reset
#define ST7789_CMD_SLPOUT 0x11 // Exit sleep mode
#define ST7789_CMD_INVON 0x21 // Invert display on
#define ST7789_CMD_DISPOFF 0x28
#define ST7789_CMD_DISPON 0x29
#define ST7789_CMD_CASET 0x2A // Column address set
#define ST7789_CMD_RASET 0x2B // Row address set
#define ST7789_CMD_RAMWR 0x2C // Memory write
#define ST7789_CMD_MADCTL 0x36 // Memory data access control
#define ST7789_CMD_COLMOD 0x3A // Interface pixel format

constexpr uint8_t FONT_WIDTH_6x8 = 6;
constexpr uint8_t FONT_HEIGHT_6x8 = 8;

ST7789::ST7789(int pin_cs, int pin_dc, int pin_reset, int pin_sck, int pin_mosi, int pin_pwm)
    : _cs(pin_cs), _dc(pin_dc), _reset(pin_reset), _sck(pin_sck), _mosi(pin_mosi), _pwm(pin_pwm) {}

void ST7789::pinLow(int pin) {
    gpio_put(pin, 0);
}

void ST7789::pinHigh(int pin) {
    gpio_put(pin, 1);
}

void ST7789::writeCommand(uint8_t cmd) {
    pinLow(_dc); // Command mode
    pinLow(_cs);
    spi_write_blocking(ST7789_SPI_PORT, &cmd, 1);
    pinHigh(_cs);
}

void ST7789::writeData(uint8_t data) {
    pinHigh(_dc); // Data mode
    pinLow(_cs);
    spi_write_blocking(ST7789_SPI_PORT, &data, 1);
    pinHigh(_cs);
}

void ST7789::writeData16(uint16_t data) {
    pinHigh(_dc); // Data mode
    pinLow(_cs);
    uint8_t high = data >> 8;
    uint8_t low = data & 0xFF;
    spi_write_blocking(ST7789_SPI_PORT, &high, 1);
    spi_write_blocking(ST7789_SPI_PORT, &low, 1);
    pinHigh(_cs);
}

void ST7789::spiInit() {
    spi_init(ST7789_SPI_PORT, 40000000); // 40 MHz
    gpio_set_function(_sck, GPIO_FUNC_SPI);
    gpio_set_function(_mosi, GPIO_FUNC_SPI);
    gpio_init(_cs);
    gpio_set_dir(_cs, GPIO_OUT);
    gpio_init(_dc);
    gpio_set_dir(_dc, GPIO_OUT);
    gpio_init(_reset);
    gpio_set_dir(_reset, GPIO_OUT);
    gpio_init(_pwm);
    gpio_set_dir(_pwm, GPIO_OUT);
}

void ST7789::hardwareReset() {
    pinHigh(_reset);
    sleep_ms(10);
    pinLow(_reset);
    sleep_ms(10);
    pinHigh(_reset);
    sleep_ms(120);
}

void ST7789::init() {
    spiInit();
    hardwareReset();

    writeCommand(ST7789_CMD_SWRESET);
    sleep_ms(150);
    writeCommand(ST7789_CMD_SLPOUT);
    sleep_ms(50);

    writeCommand(ST7789_CMD_COLMOD);
    writeData(0x55); // 16 bit colour
    sleep_ms(10);

    writeCommand(ST7789_CMD_MADCTL);
    writeData(0x60); // 90 degree rotation
    sleep_ms(10);

    writeCommand(ST7789_CMD_DISPON);
    sleep_ms(10);

    // pwm turn backlight on
    gpio_set_function(_pwm, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(_pwm);
    pwm_set_wrap(slice_num, 255);
    pwm_set_chan_level(slice_num, _pwm % 2 ? PWM_CHAN_B : PWM_CHAN_A, 255); // Full brightness
    pwm_set_enabled(slice_num, true);

    fillScreen(0xFFFF); // Clear screen
}

void ST7789::setBrightness(uint8_t brightness) {
    uint slice_num = pwm_gpio_to_slice_num(_pwm);
    pwm_set_chan_level(slice_num, _pwm % 2 ? PWM_CHAN_B : PWM_CHAN_A, brightness);
}

void ST7789::setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    writeCommand(ST7789_CMD_CASET);
    writeData16(x0);
    writeData16(x1);

    writeCommand(ST7789_CMD_RASET);
    writeData16(y0);
    writeData16(y1);

    writeCommand(ST7789_CMD_RAMWR);
}

void ST7789::drawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= WIDTH || y >= HEIGHT) return;

    setWindow(x, y, x, y);
    writeData16(color);
}

void ST7789::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= WIDTH || y >= HEIGHT) return;
    if (x + w > WIDTH) w = WIDTH - x;
    if (y + h > HEIGHT) h = HEIGHT - y;

    setWindow(x, y, x + w - 1, y + h - 1);
    pinHigh(_dc); // Data mode
    pinLow(_cs);
    for (uint32_t i = 0; i < w * h; i++) {
        uint8_t high = color >> 8;
        uint8_t low = color & 0xFF;
        spi_write_blocking(ST7789_SPI_PORT, &high, 1);
        spi_write_blocking(ST7789_SPI_PORT, &low, 1);
    }
    pinHigh(_cs);
}

void ST7789::fillScreen(uint16_t color) {
    fillRect(0, 0, WIDTH, HEIGHT, color);
}

void ST7789::drawHorizontalLine(uint16_t x, uint16_t y, uint16_t w, uint16_t t, uint16_t color, LineStyle style, uint16_t bg) {
    if (x >= WIDTH || y >= HEIGHT) return;
    if (x + w > WIDTH) w = WIDTH - x;
    if (y + t > HEIGHT) t = HEIGHT - y;

    setWindow(x, y, x + w - 1, y + t - 1);

    pinHigh(_dc);
    pinLow(_cs);

    for (size_t i = 0; i < t; i++) {
        for (size_t j = 0; j < w; j++) {
            bool draw =
                (style == LineStyle::Solid) ||
                (style == LineStyle::Dashed && (j / 8) % 2 == 0) ||
                (style == LineStyle::Dotted && j % 2 == 0);

            uint16_t c = draw ? color : bg;

            uint8_t high = c >> 8;
            uint8_t low = c & 0xFF;

            spi_write_blocking(ST7789_SPI_PORT, &high, 1);
            spi_write_blocking(ST7789_SPI_PORT, &low, 1);
        }
    }

    pinHigh(_cs);
}

void ST7789::drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint8_t scale, uint16_t bg) {
    if (c < 32 || c > 126) return; // Unsupported character
    
    // Font6x8 is const uint8_t Font6x8[][8], so use pointer-to-array
    const uint8_t (*charMap)[8] = Font6x8;

    for (uint8_t row = 0; row < FONT_HEIGHT_6x8; row++) {
        uint8_t rowData = charMap[c - 32][row];

        for (uint8_t col = 0; col < FONT_WIDTH_6x8; col++) {
            // 8-bit bitmask (MSB → LSB)
            if (rowData & (0x80 >> col)) {
                fillRect(x + col * scale, y + row * scale, scale, scale, color);
            } 
            else {
                fillRect(x + col * scale, y + row * scale, scale, scale, bg);
            }
        }
    }
}

void ST7789::drawString(uint16_t x, uint16_t y, const char* str, uint16_t color, uint8_t scale, uint16_t bg) {
    // don't draw anything off the screen
    while (*str) {
        if (x + FONT_WIDTH_6x8 * scale >= WIDTH) break; // No more space for characters

        drawChar(x, y, *str, color, scale, bg);
        x += FONT_WIDTH_6x8 * scale;
        str++;
    }
}

// void ST7789::drawParagraph(uint16_t x, uint16_t y, uint16_t w, const char* str, uint16_t color, FontSize size, uint16_t bg) {
//     while (*str) {
//         if (x + getFontWidth(size) >= WIDTH) { // Move to next line if no more space
//             x = 0;
//             y += getFontHeight(size); // Move down by character height
//             if (y + getFontHeight(size) >= HEIGHT) break; // No more space for lines
//         }
//         drawChar(x, y, *str, color, size, bg);
//         x += getFontWidth(size);
//         str++;
//     }
// }