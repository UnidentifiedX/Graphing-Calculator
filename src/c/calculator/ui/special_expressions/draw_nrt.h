#pragma once

#include "st7789/st7789.h"
#include "calculator/ui/expression_drawer.h"
#include <stdint.h>

static const uint16_t NRT_MARGIN_INDEX_TO_ROOT = 0;

ExpressionDimensions get_nrt_dimensions(ExpressionDimensions index, ExpressionDimensions radicand, uint16_t font_scale);
int get_nrt_index_raise(ExpressionDimensions index, ExpressionDimensions radicand, uint16_t font_scale);