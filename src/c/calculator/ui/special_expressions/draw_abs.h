#pragma once

#include "st7789/st7789.h"
#include "calculator/ui/expression_drawer.h"
#include <stdint.h>

static const uint16_t ABS_MARGIN_BEFORE_AFTER_ABS = 2;
static const uint16_t ABS_VERTICAL_BAR_THICKNESS = 2;

uint16_t draw_abs_start(ST7789* display, 
    uint16_t* x, 
    uint16_t baseline_y, 
    ExpressionDimensions* arg_dimensions,
    uint16_t font_scale,
    uint16_t font_color, 
    uint16_t bg_color);

ExpressionDimensions get_abs_dimensions(ExpressionDimensions arg, uint16_t font_scale);