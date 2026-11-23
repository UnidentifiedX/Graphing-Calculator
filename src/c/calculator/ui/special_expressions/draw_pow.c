#include "draw_pow.h"
#include "calculator/ui/expression_drawer.h"
#include <stdint.h>
#include "pico/stdlib.h"

int get_pow_exponent_raise(ExpressionDimensions base, ExpressionDimensions exponent, uint16_t font_scale) {
    int base_ascent = base.ascent + base.descent;
    return base_ascent - ((int)base_ascent - (int)exponent.ascent) / 2;
}

ExpressionDimensions get_pow_dimensions(ExpressionDimensions base, ExpressionDimensions exponent, uint16_t font_scale) {
    ExpressionDimensions dimensions;

    int raise = get_pow_exponent_raise(base, exponent, font_scale);
    int exponent_ascent  = raise + (int)exponent.ascent;
    int exponent_descent = (int)exponent.descent - raise; // negative for normal exponents
    if (exponent_descent < 0) exponent_descent = 0;

    dimensions.w = MAX(base.w, FONT_WIDTH_6x8 * font_scale) + POW_MARGIN_BASE_TO_EXPONENT + MAX(exponent.w, FONT_WIDTH_6x8) + POW_MARGIN_AFTER_POW;
    dimensions.ascent  = (uint16_t)MAX((int)base.ascent, exponent_ascent);
    dimensions.descent = (uint16_t)MAX((int)base.descent, exponent_descent);
    return dimensions;
}