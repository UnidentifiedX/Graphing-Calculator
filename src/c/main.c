#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "st7789/st7789.h"
#include "core/lexer.h"
#include "core/parser.h"
#include "core/evaluator.h"

static const uint8_t ST7789_PIN_CS    = 1;
static const uint8_t ST7789_PIN_SCK   = 2;
static const uint8_t ST7789_PIN_MOSI  = 3;

static const uint8_t ST7789_PIN_RESET = 8;
static const uint8_t ST7789_PIN_DC    = 9;
static const uint8_t ST7789_PIN_PWM   = 15;

int main()
{
    stdio_init_all();

    while(!stdio_usb_connected()) {
        sleep_ms(100);
    }

    ST7789 display = {
        ._cs = ST7789_PIN_CS,
        ._dc = ST7789_PIN_DC,
        ._reset = ST7789_PIN_RESET,
        ._sck = ST7789_PIN_SCK,
        ._mosi = ST7789_PIN_MOSI,
        ._pwm = ST7789_PIN_PWM
    };

    st7789_init(&display);
    st7789_draw_string(&display, 10, 10, "Hello, World!", color565(0, 0, 0), 2, color565(255, 255, 255));
    st7789_draw_string(&display, 10, 25, "This text should appear red!", color565(255, 0, 0), 1, color565(255, 255, 255));

    const char *input_string = "cos(pi/2)";

    Lexer lexer = {
        ._input_string = input_string,
        ._input_length = strlen(input_string),
        ._position = 0
    };

    SyntaxToken tokens[256];
    size_t token_count = tokenize(&lexer, tokens, 256);

    Parser parser = {
        ._tokens = tokens,
        ._token_count = token_count,
        ._position = 0
    };

    Expression expressions[256];
    size_t root_index = parse(&parser, expressions, 256);

    print_AST(expressions, root_index, "", 1);

    double result = evaluate(expressions, root_index);
    printf("Result: %g\n", result);

    while (true) {
        tight_loop_contents();
    }
}