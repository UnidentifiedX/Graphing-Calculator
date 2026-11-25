#pragma once

#include "st7789/st7789.h"
#include "calculator/evaluator/token_expression.h"

void draw_axes(ST7789* display, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                double x_min, double x_max, double y_min, double y_max);
void plot_expression(ST7789* display, TokenExpression* expr,
                        uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2,
                        double x_min, double x_max,
                        double y_min, double y_max,
                        uint16_t color);