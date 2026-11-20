#include "draw_sqrt.h"
#include "calculator/ui/expression_drawer.h"
#include <stdint.h>
#include <math.h>

static const uint16_t SQRT_TOP_BAR_MARGIN = 2;
static const uint16_t SQRT_TOP_BAR_THICKNESS = 2;
static const uint16_t SQRT_TICK_WIDTH = 2;
static const uint16_t SQRT_TICK_EXPRESSION_MARGIN = 3;
static const uint16_t SQRT_VERTICAL_BAR_THICKNESS = 2;

// IMPORTANT: we need to add pixels to the bottom of the sqrt so that it acts like a font

// building a square root from left to right: the tick, then margin between tick and expression, then the expression and the top bar
ExpressionDimensions get_sqrt_dimensions(ExpressionDimensions arg, uint16_t font_scale)
{
    ExpressionDimensions dimensions;

    dimensions.w = SQRT_TICK_WIDTH +
                   SQRT_VERTICAL_BAR_THICKNESS +
                   SQRT_TICK_EXPRESSION_MARGIN +
                   fmax(arg.w, FONT_WIDTH_6x8 * font_scale) +
                   SQRT_MARGIN_AFTER_EXPRESSION +
                   SQRT_MARGIN_AFTER_SQRT;
    dimensions.ascent = SQRT_TOP_BAR_MARGIN + SQRT_TOP_BAR_THICKNESS + arg.ascent;
    dimensions.descent = arg.descent + font_scale + font_scale; // tick goes under the baseline. add twice to add padding at the bottom

    return dimensions;
}

uint16_t draw_sqrt_start(ST7789 *display, uint16_t *x, uint16_t baseline_y, ExpressionDimensions *arg_dimensions, uint16_t font_scale, uint16_t font_color, uint16_t bg_color)
{
    uint16_t top = baseline_y - arg_dimensions[0].ascent - SQRT_TOP_BAR_MARGIN - SQRT_TOP_BAR_THICKNESS;
    uint16_t bottom = baseline_y + arg_dimensions[0].descent + font_scale; // tick sits under the content
    uint16_t vertical_height = bottom - top;
    uint16_t expression_x = *x + SQRT_TICK_WIDTH + SQRT_VERTICAL_BAR_THICKNESS + SQRT_TICK_EXPRESSION_MARGIN;

    // tick
    st7789_draw_horizontal_line(display, *x, bottom - SQRT_TOP_BAR_THICKNESS, SQRT_TICK_WIDTH, SQRT_TOP_BAR_THICKNESS, 
                            font_color, LINE_STYLE_SOLID, bg_color);
    // vertical bar
    st7789_draw_vertical_line(display, *x + SQRT_TICK_WIDTH, top, vertical_height,
                          SQRT_TOP_BAR_THICKNESS, font_color, LINE_STYLE_SOLID, bg_color);
    // top bar
    st7789_draw_horizontal_line(display, *x + SQRT_TICK_WIDTH + SQRT_VERTICAL_BAR_THICKNESS, top,
                                fmax(arg_dimensions[0].w, FONT_WIDTH_6x8 * font_scale) + SQRT_TICK_EXPRESSION_MARGIN,
                                SQRT_TOP_BAR_THICKNESS, font_color, LINE_STYLE_SOLID, bg_color);

    return expression_x;
}