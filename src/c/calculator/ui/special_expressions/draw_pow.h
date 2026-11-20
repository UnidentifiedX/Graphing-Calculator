#pragma once

#include "st7789/st7789.h"
#include "calculator/ui/expression_drawer.h"
#include <stdint.h>

static const uint16_t POW_MARGIN_BASE_TO_EXPONENT = 0;
static const uint16_t POW_MARGIN_AFTER_POW = 1;

ExpressionDimensions get_pow_dimensions(ExpressionDimensions base, ExpressionDimensions exponent, uint16_t font_scale);
int get_pow_exponent_raise(ExpressionDimensions base, ExpressionDimensions exponent, uint16_t font_scale);