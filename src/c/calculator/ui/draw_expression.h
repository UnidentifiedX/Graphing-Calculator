#pragma once

#include "st7789/st7789.h"
#include "calculator/evaluator/token_expression.h"
#include <stdint.h>
#include <stdbool.h>
#include "cursor.h"

/// @brief draws the expression. if duraw cursor is true it only updates its position. you need to activate it yourself
/// @param display 
/// @param expr 
/// @param x 
/// @param y 
/// @param font_scale 
/// @param font_color 
/// @param bg_color 
/// @param draw_cursor whether to draw the cursor or not
/// @param cursor the cursor to draw, if draw_cursor is true
void draw_expression(ST7789* display, 
    TokenExpression* expr, 
    uint16_t x, 
    uint16_t y,
    uint16_t h,
    uint16_t font_scale, 
    uint16_t font_color, 
    uint16_t bg_color, 
    bool draw_cursor, 
    Cursor* cursor);