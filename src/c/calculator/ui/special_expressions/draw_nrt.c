#include "draw_sqrt.h"
#include "draw_nrt.h"
#include "calculator/ui/expression_drawer.h"
#include <stdint.h>
#include <math.h>

int get_nrt_index_raise(ExpressionDimensions index, ExpressionDimensions radicand, uint16_t font_scale) {
    int sqrt_ascent = get_sqrt_dimensions(radicand, font_scale).ascent;
    return sqrt_ascent - ((int)sqrt_ascent - (int)index.ascent) / 2;
}

ExpressionDimensions get_nrt_dimensions(ExpressionDimensions index, ExpressionDimensions radicand, uint16_t font_scale) {
    ExpressionDimensions sqrt_dimensions = get_sqrt_dimensions(radicand, font_scale);
    ExpressionDimensions dimensions;

    int raise = get_nrt_index_raise(index, radicand, font_scale);
    int index_ascent  = raise + (int)index.ascent;
    int index_descent = (int)index.descent - raise; // negative for normal indexes
    if (index_descent < 0) index_descent = 0;

    dimensions.w = fmax(index.w, FONT_WIDTH_6x8) + NRT_MARGIN_INDEX_TO_ROOT + sqrt_dimensions.w;
    dimensions.ascent  = (uint16_t)fmax(index_ascent, (int)sqrt_dimensions.ascent);
    dimensions.descent = (uint16_t)fmax(index_descent, (int)sqrt_dimensions.descent);
    return dimensions;
}