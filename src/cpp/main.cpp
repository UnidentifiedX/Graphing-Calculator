#include <stdio.h>
#include <iostream>
#include "pico/stdlib.h"
#include "drivers/st7789/st7789.h"
#include "hardware/spi.h"
#include "calculator/ui.h"
#include "core/parser.h"
#include "core/evaluator.h"

#define ST7789_PIN_CS 1
#define ST7789_PIN_SCK 2
#define ST7789_PIN_MOSI 3

#define ST7789_PIN_RESET 8
#define ST7789_PIN_DC 9
#define ST7789_PIN_PWM 15

int main()
{
    stdio_init_all();

    while(!stdio_usb_connected()) {
        sleep_ms(100);
    }

    ST7789 display(ST7789_PIN_CS, ST7789_PIN_DC, ST7789_PIN_RESET, ST7789_PIN_SCK, ST7789_PIN_MOSI, ST7789_PIN_PWM);
    display.init();
    display.setBrightness(255);

    MainCalculatorPage mainPage;
    mainPage.draw(display);

    std::cout << "umm" << std::endl;

    std::string testExpressions[] = {
        "3! * 2 + 50% * 10",
        "100 - 3!% * 200",
        "sin(cos(0) * pi / 2)",
        "pow(2, 3) + sqrt(abs(-25))",
        "1 / (1 + pow(e, -0))" // Should equal 0.5
    };

    for (const auto& exprStr : testExpressions) {
        Evaluator eval(exprStr);
        std::cout << exprStr << " = " << eval.evaluate() << std::endl;
    }

    while (true) {
        tight_loop_contents();
    }
}