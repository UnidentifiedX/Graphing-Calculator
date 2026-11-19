#include "calculator/ui/special_expressions/draw_frac.h"
#include <math.h>

uint16_t get_frac_half(uint16_t font_scale) {
    return (FONT_HEIGHT_6x8 * font_scale) / 2;
}

ExpressionDimensions get_frac_dimensions(ExpressionDimensions numerator, ExpressionDimensions denominator, uint16_t font_scale) { 
    ExpressionDimensions dimensions;
    uint16_t half = get_frac_half(font_scale);

    dimensions.w = fmax(fmax(numerator.w, denominator.w), FONT_WIDTH_6x8 * font_scale) + 2 * FRAC_MARGIN_BEFORE_AFTER_EXPRESSION;
    // everything above the baseline: half a line up to the bar, then the numerator
    dimensions.ascent = half + FRAC_SEPARATOR_THICKNESS + numerator.ascent + numerator.descent;
    // everything below the baseline: denominator minus the half-line it overlaps
    uint16_t below = FRAC_SEPARATOR_THICKNESS + denominator.ascent + denominator.descent;

    dimensions.descent = below > half ? below - half : 0; // beware unsigned underflow
    return dimensions;
}

uint16_t draw_frac_start(ST7789* display, uint16_t* x, uint16_t baseline_y, ExpressionDimensions* arg_dimensions, uint16_t font_scale, uint16_t font_color, uint16_t bg_color) {
    // [0] is numerator, [1] is denominator

    uint16_t fraction_center_y = baseline_y - get_frac_half(font_scale);

    // draw the fraction separator
    st7789_draw_horizontal_line(display,
        *x,
        fraction_center_y - FRAC_SEPARATOR_THICKNESS,
        fmax(fmax(arg_dimensions[0].w, arg_dimensions[1].w), FONT_WIDTH_6x8 * font_scale) + 2 * FRAC_MARGIN_BEFORE_AFTER_EXPRESSION,
        FRAC_SEPARATOR_THICKNESS,
        font_color,
        LINE_STYLE_SOLID,
        bg_color
    );

    return *x + FRAC_MARGIN_BEFORE_AFTER_EXPRESSION;
}