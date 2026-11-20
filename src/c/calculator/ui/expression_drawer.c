#include "expression_drawer.h"
#include "st7789/st7789.h"
#include <string.h>
#include <stdio.h>
#include "core/inbuilt_constants.h"
#include "core/expression.h"
#include "core/inbuilt_functions.h"
#include <math.h>
#include "calculator/ui/special_expressions/draw_sqrt.h"
#include "calculator/ui/special_expressions/draw_frac.h"
#include "calculator/ui/special_expressions/draw_abs.h"
#include "calculator/ui/special_expressions/draw_nrt.h"
#include "calculator/ui/special_expressions/draw_pow.h"

/*
One REALLY IMPORTANT thing to note is that characters naturally have a pixel margin on all sides of the character EXCEPT THE TOP. 
So that means there is no need for a margin for any character/between characters except the top
*/

static ExpressionDimensions calculate_dimensions_impl(TokenExpression* expr, size_t* index, uint16_t font_scale);
static void draw_expression_impl(ST7789* display, TokenExpression* expr, size_t* index, uint16_t* x, uint16_t baseline_y, ExpressionDimensions dimensions, uint16_t font_scale, uint16_t font_color, uint16_t bg_color, bool draw_cursor, Cursor* cursor);

static char* syntaxkind_to_string(SyntaxKind kind) {
    switch (kind) {
        case SYNTAX_KIND_PLUS: return "+";
        case SYNTAX_KIND_MINUS: return "-";
        case SYNTAX_KIND_MULTIPLY: return "*";
        case SYNTAX_KIND_DIVIDE: return "/";
        case SYNTAX_KIND_FACTORIAL: return "!";
        case SYNTAX_KIND_PERCENTAGE: return "%";
        case SYNTAX_KIND_OPEN_PARENTHESIS: return "(";
        case SYNTAX_KIND_CLOSE_PARENTHESIS: return ")";
        default: return "?";
    }
}

static char* inbuiltconstant_to_string(InbuiltConstant constant) {
    switch (constant) {
        case INBUILT_CONSTANT_PI: return "\xe3"; // π
        case INBUILT_CONSTANT_E: return "e"; // e
        default: return "?";
    }
}

static void get_special_function_argument_dimensions(ExpressionDimensions* dimensions, InbuiltFunction func, TokenExpression* expr, size_t* index, uint16_t font_scale) {
    size_t arity = get_function_arity(func);

    if (func == INBUILT_FUNCTION_NRT) {
        // the first argument is the index, which is smaller and baseline shifted up
        dimensions[0] = calculate_dimensions_impl(expr, index, 1);
        dimensions[1] = calculate_dimensions_impl(expr, index, font_scale);
        return;
    }
    if (func == INBUILT_FUNCTION_POW) {
        // the second argument is the exponent, which is smaller and baseline shifted up
        dimensions[0] = calculate_dimensions_impl(expr, index, font_scale);
        dimensions[1] = calculate_dimensions_impl(expr, index, 1);
        return;
    }

    for (size_t i = 0; i < arity; i++) {
        dimensions[i] = calculate_dimensions_impl(expr, index, font_scale);
    }
}

ExpressionDimensions calculate_dimensions(TokenExpression* expr, uint16_t font_scale) {
    size_t index = 0;
    return calculate_dimensions_impl(expr, &index, font_scale);
}

void draw_empty_box(ST7789* display, uint16_t x, uint16_t y, uint16_t font_scale, uint16_t color, uint16_t bg_color) {
    st7789_draw_rect(display, 
        x,
        y - FONT_HEIGHT_6x8 * font_scale,
        (FONT_WIDTH_6x8 - 1) * font_scale, // fonts have a 1-pixel margin on the sides of the character
        (FONT_HEIGHT_6x8 - 1) * font_scale, // fonts have a 1-pixel margin below the character
        1,
        color, 
        LINE_STYLE_DOTTED, 
        bg_color
    );
}

void draw_empty_box_centred(ST7789* display, uint16_t cx, uint16_t y, uint16_t font_scale, uint16_t color, uint16_t bg_color) {
    draw_empty_box(display, cx - (FONT_WIDTH_6x8 * font_scale) / 2, y, font_scale, color, bg_color);
}

void draw_expression(ST7789* display, 
                TokenExpression* expr, 
                uint16_t x,
                uint16_t margin_bottom,
                ExpressionDimensions dimensions,
                uint16_t font_scale, 
                uint16_t font_color, 
                uint16_t bg_color, 
                bool draw_cursor, 
                Cursor* cursor) {
    uint16_t x_index = x;
    size_t index = 0;
    
    draw_expression_impl(display, expr, &index, &x_index, margin_bottom - dimensions.descent, dimensions, font_scale, font_color, bg_color, draw_cursor, cursor);
}

static ExpressionDimensions calculate_dimensions_impl(TokenExpression* expr, size_t* index, uint16_t font_scale) {
    // font scale is also equal to the margin between characters

    ExpressionDimensions dimensions = { .w = 0, .ascent = FONT_HEIGHT_6x8 * font_scale, .descent = 0 };

    while (*index < expr->size) {
        Node* node = &expr->node_buffer[*index];

        switch (node->type) {
            case NODE_TYPE_NUMBER: {
                dimensions.w += strlen(node->node.num_str) * FONT_WIDTH_6x8 * font_scale;
                (*index)++;
                break;
            }
            case NODE_TYPE_ATOM: {
                dimensions.w += FONT_WIDTH_6x8 * font_scale;
                (*index)++;
                break;
            }
            case NODE_TYPE_VARIABLE: {
                InbuiltConstant constant = get_constant_from_name(node->node.var_identifier);
                if (constant != INBUILT_CONSTANT_INVALID) {
                    char* constant_str = inbuiltconstant_to_string(constant);
                    dimensions.w += strlen(constant_str) * FONT_WIDTH_6x8 * font_scale;
                } else {
                    dimensions.w += strlen(node->node.var_identifier) * FONT_WIDTH_6x8 * font_scale;
                }

                (*index)++;
                break;
            }
            case NODE_TYPE_FUNCTION: {
                if (is_special_display_function(node->node.func_identifier)) {
                    (*index)++; // move past the function node

                    InbuiltFunction func = get_inbuilt_function_from_identifier(node->node.func_identifier);
                    ExpressionDimensions arg_dimensions[MAX_ARGUMENTS];
                    get_special_function_argument_dimensions(arg_dimensions, func, expr, index, font_scale);
                    ExpressionDimensions function_dimensions;

                    switch (func) { // because each special inbuilt function calculates its own width and height differently
                        case INBUILT_FUNCTION_SQRT: {
                            function_dimensions = get_sqrt_dimensions(arg_dimensions[0], font_scale);
                            break;
                        }
                        case INBUILT_FUNCTION_FRAC: {
                            function_dimensions = get_frac_dimensions(arg_dimensions[0], arg_dimensions[1], font_scale);                            
                            break;
                        }
                        case INBUILT_FUNCTION_ABS: {
                            function_dimensions = get_abs_dimensions(arg_dimensions[0], font_scale);
                            break;
                        }
                        case INBUILT_FUNCTION_NRT: {
                            function_dimensions = get_nrt_dimensions(arg_dimensions[0], arg_dimensions[1], font_scale);
                            break;
                        }
                        case INBUILT_FUNCTION_POW: {
                            function_dimensions = get_pow_dimensions(arg_dimensions[0], arg_dimensions[1], font_scale);
                            break;
                        }
                    }

                    dimensions.w += function_dimensions.w;
                    dimensions.ascent = fmax(dimensions.ascent, function_dimensions.ascent);
                    dimensions.descent = fmax(dimensions.descent, function_dimensions.descent);

                    printf("arg_dimensions[0].w: %d, arg_dimensions[0].ascent: %d, arg_dimensions[0].descent: %d\n", arg_dimensions[0].w, arg_dimensions[0].ascent, arg_dimensions[0].descent);
                    printf("dimensions.w: %d, dimensions.ascent: %d, dimensions.descent: %d\n", dimensions.w, dimensions.ascent, dimensions.descent);

                } else {
                    dimensions.w += (strlen(node->node.func_identifier) + 1) * FONT_WIDTH_6x8 * font_scale + font_scale; // +1 for the '('
                    (*index)++;
                }
                break;
            }
            case NODE_TYPE_END_OF_ARGUMENT: {
                (*index)++; // move past the end of argument node
                return dimensions; // return early because we don't want to continue calculating the dimensions of the rest of the expression
            }
        }
    }

    return dimensions;
}

// will draw until either the end of the expression or until it reaches an end of argument node, whichever comes first
void draw_expression_impl(ST7789* display, 
                TokenExpression* expr,
                size_t *index,
                uint16_t* x, 
                uint16_t baseline_y,
                ExpressionDimensions dimensions,
                uint16_t font_scale, 
                uint16_t font_color, 
                uint16_t bg_color, 
                bool draw_cursor, 
                Cursor* cursor) {

    size_t cursor_y = baseline_y - FONT_HEIGHT_6x8 * font_scale;

    if (expr->size == 0) {
        if (draw_cursor) {
            set_cursor_position(cursor, *x, cursor_y);
        }
        return;
    }
    
    while (*index < expr->size) {
        Node* node = &expr->node_buffer[*index];

        switch (node->type) {
            case NODE_TYPE_NUMBER: {
                if (draw_cursor) {
                    if (*index == expr->node_index) {
                        if (expr->char_index == __SIZE_MAX__) {
                            set_cursor_position(cursor, *x - font_scale, cursor_y);
                        } else {
                            uint16_t cursor_x = *x - font_scale + expr->char_index * FONT_WIDTH_6x8 * font_scale;
                            set_cursor_position(cursor, cursor_x, cursor_y);
                        }
                    }
                }

                st7789_draw_string(display, *x, baseline_y - FONT_HEIGHT_6x8 * font_scale, node->node.num_str, font_color, font_scale, bg_color);
                *x += strlen(node->node.num_str) * FONT_WIDTH_6x8 * font_scale;
                (*index)++;
                break;
            }
            case NODE_TYPE_ATOM: {
                if (draw_cursor && *index == expr->node_index) {
                    set_cursor_position(cursor, *x - font_scale, cursor_y);
                }

                st7789_draw_char(display, *x, baseline_y - FONT_HEIGHT_6x8 * font_scale, *syntaxkind_to_string(node->node.token.kind), font_color, font_scale, bg_color);
                *x += FONT_WIDTH_6x8 * font_scale;
                (*index)++;
                break;
            }
            case NODE_TYPE_VARIABLE: {
                if (draw_cursor && *index == expr->node_index) {
                    set_cursor_position(cursor, *x - font_scale, cursor_y);
                }

                InbuiltConstant constant = get_constant_from_name(node->node.var_identifier);
                if (constant != INBUILT_CONSTANT_INVALID) {
                    char* constant_str = inbuiltconstant_to_string(constant);
                    st7789_draw_string(display, *x, baseline_y - FONT_HEIGHT_6x8 * font_scale, constant_str, font_color, font_scale, bg_color);
                    *x += strlen(constant_str) * FONT_WIDTH_6x8 * font_scale;
                } else {
                    st7789_draw_string(display, *x, baseline_y - FONT_HEIGHT_6x8 * font_scale, node->node.var_identifier, font_color, font_scale, bg_color);
                    *x += strlen(node->node.var_identifier) * FONT_WIDTH_6x8 * font_scale;
                }
                (*index)++;
                break;
            }
            case NODE_TYPE_FUNCTION: {
                if (is_special_display_function(node->node.func_identifier)) {
                    if (draw_cursor && *index == expr->node_index) {
                        set_cursor_position(cursor, *x - font_scale, cursor_y);
                    }

                    (*index)++; // move past the function node

                    InbuiltFunction func = get_inbuilt_function_from_identifier(node->node.func_identifier);
                    ExpressionDimensions arg_dimensions[MAX_ARGUMENTS];
                    size_t arg_index = *index; // temporary index to calculate dimensions without modifying the original index
                    get_special_function_argument_dimensions(arg_dimensions, func, expr, &arg_index, font_scale); // unlike calculating dimensions we dont consume the subsequent nodes
                    
                    switch (func) {
                        case INBUILT_FUNCTION_SQRT: {
                            *x = draw_sqrt_start(display, x, baseline_y, arg_dimensions, font_scale, font_color, bg_color);
                            draw_expression_impl(display, expr, index, x, baseline_y, arg_dimensions[0], font_scale, font_color, bg_color, draw_cursor, cursor);
                            if (arg_dimensions[0].w == 0) { // draw box if square root is empty
                                draw_empty_box(display, *x, baseline_y, font_scale, font_color, bg_color);
                                *x += FONT_WIDTH_6x8 * font_scale;
                            }
                            *x += SQRT_MARGIN_AFTER_EXPRESSION + SQRT_MARGIN_AFTER_SQRT;

                            break;
                        }
                        // TODO: cursor placement and also this one extra pixel on the right of the fraction bar? minor issue though
                        case INBUILT_FUNCTION_FRAC: {
                            uint16_t frac_x0 = *x;
                            *x = draw_frac_start(display, x, baseline_y, arg_dimensions, font_scale, font_color, bg_color);
                            ExpressionDimensions frac_dimensions = get_frac_dimensions(arg_dimensions[0], arg_dimensions[1], font_scale);

                            uint16_t inner_w = fmax(fmax(arg_dimensions[0].w, arg_dimensions[1].w), FONT_WIDTH_6x8 * font_scale);
                            uint16_t numerator_x = *x + (inner_w - arg_dimensions[0].w) / 2;
                            uint16_t denominator_x = *x + (inner_w - arg_dimensions[1].w) / 2;

                            uint16_t fraction_center_y = baseline_y - get_frac_half(font_scale);
                            uint16_t numerator_y = fraction_center_y - FRAC_SEPARATOR_THICKNESS - arg_dimensions[0].descent;
                            uint16_t denominator_y = fraction_center_y + FRAC_SEPARATOR_THICKNESS + arg_dimensions[1].ascent;

                            draw_expression_impl(display, expr, index, &numerator_x, numerator_y, arg_dimensions[0], font_scale, font_color, bg_color, draw_cursor, cursor);
                            draw_expression_impl(display, expr, index, &denominator_x, denominator_y,arg_dimensions[1],font_scale, font_color, bg_color, draw_cursor, cursor);

                            if (arg_dimensions[0].w == 0) { // draw box if numerator is empty
                                draw_empty_box_centred(display, numerator_x, numerator_y, font_scale, font_color, bg_color);
                            }
                            if (arg_dimensions[1].w == 0) { // draw box if denominator is empty
                                draw_empty_box_centred(display, denominator_x, denominator_y, font_scale, font_color, bg_color);
                            }

                            *x = frac_x0 + frac_dimensions.w + FRAC_MARGIN_AFTER_FRAC; // move x to the right of the fraction

                            break;
                        }
                        case INBUILT_FUNCTION_ABS: {
                            *x = draw_abs_start(display, x, baseline_y, arg_dimensions, font_scale, font_color, bg_color);
                            draw_expression_impl(display, expr, index, x, baseline_y, arg_dimensions[0], font_scale, font_color, bg_color, draw_cursor, cursor);
                            if (arg_dimensions[0].w == 0) { // draw box if absolute value is empty
                                draw_empty_box(display, *x, baseline_y, font_scale, font_color, bg_color);
                                *x += FONT_WIDTH_6x8 * font_scale;
                            }
                            *x += ABS_MARGIN_BEFORE_AFTER_EXPRESSION + 2 * ABS_VERTICAL_BAR_THICKNESS; // move x to the right of the absolute value expression

                            break;
                        }
                        case INBUILT_FUNCTION_NRT: {
                            // drawing the nrt is pretty easy cause we just draw the index, then a space, then the square root
                            // the index is smaller and baseline shifted up so it's at the top of the square root
                            // the baseline of the index is such that half of the index is above the sqrt bar and half is below the sqrt bar

                            // index
                            uint16_t index_baseline_y = baseline_y - get_nrt_index_raise(arg_dimensions[0], arg_dimensions[1], font_scale);
                            draw_expression_impl(display, expr, index, x, index_baseline_y, arg_dimensions[0], 1, font_color, bg_color, draw_cursor, cursor);
                            if (arg_dimensions[0].w == 0) { // draw box if index is empty
                                draw_empty_box(display, *x, index_baseline_y, 1, font_color, bg_color);
                                *x += FONT_WIDTH_6x8;
                            }

                            // space between index and sqrt
                            *x += NRT_MARGIN_INDEX_TO_ROOT;

                            // sqrt
                            *x = draw_sqrt_start(display, x, baseline_y, &arg_dimensions[1], font_scale, font_color, bg_color);
                            draw_expression_impl(display, expr, index, x, baseline_y, arg_dimensions[1], font_scale, font_color, bg_color, draw_cursor, cursor);
                            if (arg_dimensions[1].w == 0) { // draw box if radicand is empty
                                draw_empty_box(display, *x, baseline_y, font_scale, font_color, bg_color);
                                *x += FONT_WIDTH_6x8 * font_scale;
                            }

                            *x += SQRT_MARGIN_AFTER_EXPRESSION + SQRT_MARGIN_AFTER_SQRT;

                            break;
                        }
                        case INBUILT_FUNCTION_POW: { // one limitation is that uhh you actually dont know if you're typing in the base or outside the base so that's something that needs to be fixed
                            // drawing the pow is basically drawing nrt but in reverse order

                            // base
                            draw_expression_impl(display, expr, index, x, baseline_y, arg_dimensions[0], font_scale, font_color, bg_color, draw_cursor, cursor);
                            if (arg_dimensions[0].w == 0) { // draw box if base is empty
                                draw_empty_box(display, *x, baseline_y, font_scale, font_color, bg_color);
                                *x += FONT_WIDTH_6x8 * font_scale;
                            }

                            // space between base and exponent
                            *x += POW_MARGIN_BASE_TO_EXPONENT;

                            // exponent
                            uint16_t exponent_baseline_y = baseline_y - get_pow_exponent_raise(arg_dimensions[0], arg_dimensions[1], font_scale);
                            draw_expression_impl(display, expr, index, x, exponent_baseline_y, arg_dimensions[1], 1, font_color, bg_color, draw_cursor, cursor);
                            if (arg_dimensions[1].w == 0) { // draw box if exponent is empty
                                draw_empty_box(display, *x, exponent_baseline_y, 1, font_color, bg_color);
                                *x += FONT_WIDTH_6x8;
                            }

                            *x += POW_MARGIN_AFTER_POW;

                            break;
                        }
                    }
                } else {
                    if (draw_cursor && *index == expr->node_index) {
                        set_cursor_position(cursor, *x - font_scale, cursor_y);
                    }

                    char display_str[MAX_FUNCTION_IDENTIFIER_LENGTH + 2];
                    snprintf(display_str, sizeof(display_str), "%s(", node->node.func_identifier);
                    
                    st7789_draw_string(display, *x, baseline_y - FONT_HEIGHT_6x8 * font_scale, display_str, font_color, font_scale, bg_color);
                    *x += strlen(display_str) * FONT_WIDTH_6x8 * font_scale;

                    (*index)++;
                }
                break;
            }
            case NODE_TYPE_END_OF_ARGUMENT: {
                if (draw_cursor && *index == expr->node_index) {
                    set_cursor_position(cursor, *x - font_scale, cursor_y);
                }

                (*index)++; // move past the end of argument node
                return;
            }
        }
    }

    if (draw_cursor && expr->node_index >= expr->size) {
        set_cursor_position(cursor, *x - font_scale, cursor_y);
    }
}

