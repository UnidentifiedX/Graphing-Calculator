#include "st7789.h"
#include <stdint.h>
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include "fonts.h"

static spi_inst_t *ST7789_SPI_PORT = spi0;

static const uint8_t ST7789_CMD_SWRESET = 0x01; // Software reset
static const uint8_t ST7789_CMD_SLPOUT  = 0x11; // Exit sleep mode
static const uint8_t ST7789_CMD_INVON   = 0x21; // Invert display on
static const uint8_t ST7789_CMD_DISPOFF = 0x28;
static const uint8_t ST7789_CMD_DISPON  = 0x29;
static const uint8_t ST7789_CMD_CASET   = 0x2A; // Column address set
static const uint8_t ST7789_CMD_RASET   = 0x2B; // Row address set
static const uint8_t ST7789_CMD_RAMWR   = 0x2C; // Memory write
static const uint8_t ST7789_CMD_MADCTL  = 0x36; // Memory data access control
static const uint8_t ST7789_CMD_COLMOD  = 0x3A; // Interface pixel format

static void st7789_pin_low(uint8_t pin) {
    gpio_put(pin, 0);
}

static void st7789_pin_high(uint8_t pin) {
    gpio_put(pin, 1);
}

static void st7789_write_command(ST7789 *display, uint8_t cmd) {
    st7789_pin_low(display->_dc); // Command mode
    st7789_pin_low(display->_cs);
    spi_write_blocking(ST7789_SPI_PORT, &cmd, 1);
    st7789_pin_high(display->_cs);
}

static void st7789_write_data(ST7789 *display, uint8_t data) {
    st7789_pin_high(display->_dc); // Data mode
    st7789_pin_low(display->_cs);
    spi_write_blocking(ST7789_SPI_PORT, &data, 1);
    st7789_pin_high(display->_cs);
}

static void st7789_write_data_16(ST7789 *display, uint16_t data) {
    st7789_pin_high(display->_dc); // Data mode
    st7789_pin_low(display->_cs);
    uint8_t high = data >> 8;
    uint8_t low = data & 0xFF;
    spi_write_blocking(ST7789_SPI_PORT, &high, 1);
    spi_write_blocking(ST7789_SPI_PORT, &low, 1);
    st7789_pin_high(display->_cs);
}

static void st7789_spi_init(ST7789 *display) {
    spi_init(ST7789_SPI_PORT, 40000000); // 40 MHz
    gpio_set_function(display->_sck, GPIO_FUNC_SPI);
    gpio_set_function(display->_mosi, GPIO_FUNC_SPI);
    gpio_init(display->_cs);
    gpio_set_dir(display->_cs, GPIO_OUT);
    gpio_init(display->_dc);
    gpio_set_dir(display->_dc, GPIO_OUT);
    gpio_init(display->_reset);
    gpio_set_dir(display->_reset, GPIO_OUT);
    gpio_init(display->_pwm);
    gpio_set_dir(display->_pwm, GPIO_OUT);
}

static void st7789_reset(ST7789 *display) {
    st7789_pin_high(display->_reset);
    sleep_ms(10);
    st7789_pin_low(display->_reset);
    sleep_ms(10);
    st7789_pin_high(display->_reset);
    sleep_ms(120);
}

void st7789_init(ST7789 *display) {
    st7789_spi_init(display);
    st7789_reset(display);

    st7789_write_command(display, ST7789_CMD_SWRESET);
    sleep_ms(150);
    st7789_write_command(display, ST7789_CMD_SLPOUT);
    sleep_ms(50);

    st7789_write_command(display, ST7789_CMD_COLMOD);
    st7789_write_data(display, 0x55); // 16-bit color
    sleep_ms(10);

    st7789_write_command(display, ST7789_CMD_MADCTL);
    st7789_write_data(display, 0x60); // 90-degree rotation
    sleep_ms(10);

    st7789_write_command(display, ST7789_CMD_DISPON);
    sleep_ms(10);

    gpio_set_function(display->_pwm, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(display->_pwm);
    pwm_set_wrap(slice_num, 255);
    pwm_set_chan_level(slice_num, display->_pwm % 2 ? PWM_CHAN_B : PWM_CHAN_A, 255); // Full brightness
    pwm_set_enabled(slice_num, true);

    st7789_fill_screen(display, 0xFFFF); // Clear screen
}

void st7789_set_brightness(ST7789 *display, uint8_t brightness) {
    uint slice_num = pwm_gpio_to_slice_num(display->_pwm);
    pwm_set_chan_level(slice_num, display->_pwm % 2 ? PWM_CHAN_B : PWM_CHAN_A, brightness);
}

void st7789_set_window(ST7789 *display, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    st7789_write_command(display, ST7789_CMD_CASET);
    st7789_write_data_16(display, x0);
    st7789_write_data_16(display, x1);

    st7789_write_command(display, ST7789_CMD_RASET);
    st7789_write_data_16(display, y0);
    st7789_write_data_16(display, y1);

    st7789_write_command(display, ST7789_CMD_RAMWR);
}

void st7789_draw_pixel(ST7789 *display, uint16_t x, uint16_t y, uint16_t color) {
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;

    st7789_set_window(display, x, y, x, y);
    st7789_write_data_16(display, color);
}

void st7789_fill_rect(ST7789 *display, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;
    if (x + w > ST7789_WIDTH) w = ST7789_WIDTH - x;
    if (y + h > ST7789_HEIGHT) h = ST7789_HEIGHT - y;

    st7789_set_window(display, x, y, x + w - 1, y + h - 1);
    st7789_pin_high(display->_dc); // Data mode
    st7789_pin_low(display->_cs);
    for (uint32_t i = 0; i < w * h; i++) {
        uint8_t high = color >> 8;
        uint8_t low = color & 0xFF;
        spi_write_blocking(ST7789_SPI_PORT, &high, 1);
        spi_write_blocking(ST7789_SPI_PORT, &low, 1);
    }
    st7789_pin_high(display->_cs);
}

void st7789_fill_screen(ST7789 *display, uint16_t color) {
    st7789_fill_rect(display, 0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

void st7789_draw_horizontal_line(ST7789 *display, uint16_t x, uint16_t y, uint16_t w, uint16_t t, uint16_t color, LineStyle style, uint16_t bg) {
    if (y >= ST7789_HEIGHT || x >= ST7789_WIDTH) return;
    if (x + w > ST7789_WIDTH) w = ST7789_WIDTH - x;
    if (y + t > ST7789_HEIGHT) t = ST7789_HEIGHT - y;

    st7789_set_window(display, x, y, x + w - 1, y + t - 1);
    st7789_pin_high(display->_dc); // Data mode
    st7789_pin_low(display->_cs);

    for (size_t i = 0; i < t; i++) {
        for (size_t j = 0; j < w; j++) {
            bool draw =
                (style == LINE_STYLE_SOLID) ||
                (style == LINE_STYLE_DASHED && (j / 8) % 2 == 0) ||
                (style == LINE_STYLE_DOTTED && j % 2 == 0);

            uint16_t c = draw ? color : bg;

            uint8_t high = c >> 8;
            uint8_t low = c & 0xFF;

            spi_write_blocking(ST7789_SPI_PORT, &high, 1);
            spi_write_blocking(ST7789_SPI_PORT, &low, 1);
        }
    }

    st7789_pin_high(display->_cs);
}

void st7789_draw_vertical_line(ST7789 *display, uint16_t x, uint16_t y, uint16_t h, uint16_t t, uint16_t color, LineStyle style, uint16_t bg) {
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;
    if (x + t > ST7789_WIDTH) t = ST7789_WIDTH - x;
    if (y + h > ST7789_HEIGHT) h = ST7789_HEIGHT - y;

    st7789_set_window(display, x, y, x + t - 1, y + h - 1);
    st7789_pin_high(display->_dc); // Data mode
    st7789_pin_low(display->_cs);

    for (size_t i = 0; i < h; i++) {
        for (size_t j = 0; j < t; j++) {
            bool draw =
                (style == LINE_STYLE_SOLID) ||
                (style == LINE_STYLE_DASHED && (i / 8) % 2 == 0) ||
                (style == LINE_STYLE_DOTTED && i % 2 == 0);

            uint16_t c = draw ? color : bg;

            uint8_t high = c >> 8;
            uint8_t low = c & 0xFF;

            spi_write_blocking(ST7789_SPI_PORT, &high, 1);
            spi_write_blocking(ST7789_SPI_PORT, &low, 1);
        }
    }

    st7789_pin_high(display->_cs);
}

void st7789_draw_char(ST7789 *display, uint16_t x, uint16_t y, char c, uint16_t color, uint8_t scale, uint16_t bg) {
    if (c < 32 || c > 127) return; // Unsupported character

    const uint8_t (*char_map)[8] = Font6x8;

    for (uint8_t row = 0; row < FONT_HEIGHT_6x8; row++) {
        uint8_t rowData = char_map[c - 32][row];

        for (uint8_t col = 0; col < FONT_WIDTH_6x8; col++) {
            // 8-bit bitmask (MSB → LSB)
            if (rowData & (0x80 >> col)) {
                st7789_fill_rect(display, x + col * scale, y + row * scale, scale, scale, color);
            } 
            else {
                st7789_fill_rect(display, x + col * scale, y + row * scale, scale, scale, bg);
            }
        }
    }
}

void st7789_draw_string(ST7789 *display, uint16_t x, uint16_t y, const char* str, uint16_t color, uint8_t scale, uint16_t bg) {
    // don't draw anything off the screen
    while (*str) {
        if (x + FONT_WIDTH_6x8 * scale >= ST7789_WIDTH) break; // No more space for characters

        st7789_draw_char(display, x, y, *str, color, scale, bg);
        x += FONT_WIDTH_6x8 * scale;
        str++;
    }
}