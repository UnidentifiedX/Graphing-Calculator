#pragma once

#include "st7789/st7789.h"
#include "calculator/ui/expression_drawer.h"
#include <stdint.h>

static const uint16_t FRAC_SEPARATOR_THICKNESS = 2;
static const uint16_t FRAC_MARGIN_BEFORE_AFTER_EXPRESSION = 2;
static const uint16_t FRAC_MARGIN_AFTER_FRAC = 1;

uint16_t draw_frac_start(ST7789* display, 
    uint16_t* x, 
    uint16_t baseline_y, 
    ExpressionDimensions* arg_dimensions,
    uint16_t font_scale,
    uint16_t font_color, 
    uint16_t bg_color);

ExpressionDimensions get_frac_dimensions(ExpressionDimensions numerator, ExpressionDimensions denominator, uint16_t font_scale);
uint16_t get_frac_half(uint16_t font_scale);