#include <stdio.h>
#include "pico/stdlib.h"
#include "drivers/st7789/st7789.h"
#include "hardware/spi.h"

#define ST7789_PIN_CS 1
#define ST7789_PIN_SCK 2
#define ST7789_PIN_MOSI 3

#define ST7789_PIN_RESET 8
#define ST7789_PIN_DC 9
#define ST7789_PIN_PWM 15

int main()
{
    stdio_init_all();

    ST7789 display(ST7789_PIN_CS, ST7789_PIN_DC, ST7789_PIN_RESET, ST7789_PIN_SCK, ST7789_PIN_MOSI, ST7789_PIN_PWM);
    display.init();

    display.setBrightness(128); // Set brightness to 50%

    // display.fillScreen(color565(0, 255, 255));
    display.fillRect(100, 50, 100, 100, color565(255, 0, 255));

    // display.drawChar(10, 10, 'A', color565(255, 255, 255), color565(255, 0, 0));
    // display.drawString(10, 10, "Hello, world!", color565(0, 0, 0));
    display.drawParagraph(0, 60, 200, "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.", color565(0, 0, 0));

    // Always draw in top left corner so i dont forget
    display.fillRect(0, 0, 5, 5, color565(255, 0, 0));

    while (true) {
        tight_loop_contents();
    }
}