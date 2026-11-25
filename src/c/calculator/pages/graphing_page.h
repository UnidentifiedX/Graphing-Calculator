#pragma once

#include "pages.h"
#include "calculator/ui/cursor.h"
#include "calculator/evaluator/token_expression.h"
#include "calculator/ui/expression_drawer.h"

#define GRAPHING_PAGE_MAX_PLOTS 3

typedef struct {
    TokenExpression expression;
    ExpressionDimensions expression_dimensions;
} PlottingExpression;

typedef struct {
    PlottingExpression plotting_expressions[GRAPHING_PAGE_MAX_PLOTS];
    size_t current_selected_plot_index;

    Cursor cursor;
} GraphingPageData;

extern Page graphing_page;
extern const uint16_t line_colors[GRAPHING_PAGE_MAX_PLOTS];