#include "draw_abs.h"
#include "calculator/ui/expression_drawer.h"
#include <stdint.h>
#include "pico/stdlib.h"

ExpressionDimensions get_abs_dimensions(ExpressionDimensions arg, uint16_t font_scale) {
    ExpressionDimensions dimensions;

    dimensions.w = 2 * ABS_VERTICAL_BAR_THICKNESS + MAX(arg.w, FONT_WIDTH_6x8 * font_scale) + 2 * ABS_MARGIN_BEFORE_AFTER_ABS;
    dimensions.ascent = arg.ascent; // padding
    dimensions.descent = arg.descent; // padding

    return dimensions;
}

uint16_t draw_abs_start(ST7789 *display, uint16_t *x, uint16_t baseline_y, ExpressionDimensions *arg_dimensions, uint16_t font_scale, uint16_t font_color, uint16_t bg_color) {
    // draw the left vertical bar
    st7789_draw_vertical_line(display,
        *x,
        baseline_y - arg_dimensions[0].ascent,
        arg_dimensions[0].ascent + arg_dimensions[0].descent,
        ABS_VERTICAL_BAR_THICKNESS,
        font_color,
        LINE_STYLE_SOLID,
        bg_color
    );

    // then the right vertical bar
    st7789_draw_vertical_line(display,
        *x + ABS_VERTICAL_BAR_THICKNESS + MAX(arg_dimensions[0].w, FONT_WIDTH_6x8 * font_scale) + ABS_MARGIN_BEFORE_AFTER_ABS,
        baseline_y - arg_dimensions[0].ascent,
        arg_dimensions[0].ascent + arg_dimensions[0].descent,
        ABS_VERTICAL_BAR_THICKNESS,
        font_color,
        LINE_STYLE_SOLID,
        bg_color
    );

    return *x + ABS_VERTICAL_BAR_THICKNESS + ABS_MARGIN_BEFORE_AFTER_ABS;
}