#pragma once

#include "st7789/st7789.h"
#include "calculator/ui/expression_drawer.h"
#include <stdint.h>

static const uint16_t SQRT_MARGIN_AFTER_EXPRESSION = 2;
static const uint16_t SQRT_MARGIN_AFTER_SQRT = 1;

// this just draws until the start of the expression. expression drawing is done by the drawer
//
// returns the start offset
uint16_t draw_sqrt_start(ST7789* display, 
    uint16_t* x, 
    uint16_t baseline_y, 
    ExpressionDimensions* arg_dimensions,
    uint16_t font_scale,
    uint16_t font_color, 
    uint16_t bg_color);

ExpressionDimensions get_sqrt_dimensions(ExpressionDimensions arg, uint16_t font_scale);