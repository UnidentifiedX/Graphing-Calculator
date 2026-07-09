#include "expression_drawer.h"
#include "st7789/st7789.h"
#include <string.h>
#include <stdio.h>
#include "core/inbuilt_constants.h"
#include "core/expression.h"
#include "core/inbuilt_functions.h"
#include <math.h>

static const uint16_t SQRT_TOP_BAR_MARGIN = 2;
static const uint16_t SQRT_TOP_BAR_THICKNESS = 2;
static const uint16_t SQRT_TICK_WIDTH = 2;
static const uint16_t SQRT_TICK_EXPRESSION_MARGIN = 3;
static const uint16_t SQRT_VERTICAL_BAR_THICKNESS = 2;
static const uint16_t SQRT_MARGIN_AFTER_EXPRESSION = 2;
static const uint16_t SQRT_MARGIN_AFTER_SQRT = 1;

static ExpressionDimensions calculate_dimensions_impl(TokenExpression* expr, size_t* index, uint16_t font_scale);
static void draw_expression_impl(ST7789* display, TokenExpression* expr, size_t* index, uint16_t* x, uint16_t baseline_y, ExpressionDimensions dimensions, uint16_t font_scale, uint16_t font_color, uint16_t bg_color, bool draw_cursor, Cursor* cursor);

static ExpressionDimensions get_sqrt_dimensions(ExpressionDimensions arg);

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

    for (size_t i = 0; i < arity; i++) {
        dimensions[i] = calculate_dimensions_impl(expr, index, font_scale);
    }
}

ExpressionDimensions calculate_dimensions(TokenExpression* expr, uint16_t font_scale) {
    size_t index = 0;
    return calculate_dimensions_impl(expr, &index, font_scale);
}

void draw_expression(ST7789* display, 
                TokenExpression* expr, 
                uint16_t x, 
                uint16_t baseline_y,
                ExpressionDimensions dimensions,
                uint16_t font_scale, 
                uint16_t font_color, 
                uint16_t bg_color, 
                bool draw_cursor, 
                Cursor* cursor) {
    uint16_t x_index = x;
    size_t index = 0;
    // clear the background before drawing anything
    size_t height = dimensions.ascent + dimensions.descent;
    // st7789_fill_rect(display, x, baseline_y - height, ST7789_WIDTH - x, height, bg_color);
    draw_expression_impl(display, expr, &index, &x_index, baseline_y, dimensions, font_scale, font_color, bg_color, draw_cursor, cursor);
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

                    switch (func) { // because each special inbuilt function calculates its own width and height differently
                        case INBUILT_FUNCTION_SQRT: {
                            // building a square root from left to right: the tick, then margin between tick and expression, then the expression and the top bar
                            ExpressionDimensions sqrt_dimensions = get_sqrt_dimensions(arg_dimensions[0]);

                            dimensions.w += sqrt_dimensions.w;
                            dimensions.ascent = fmax(dimensions.ascent, sqrt_dimensions.ascent);
                            dimensions.descent = fmax(dimensions.descent, sqrt_dimensions.descent);

                            break;
                        }
                    }

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
                            ExpressionDimensions sqrt_dimensions = get_sqrt_dimensions(arg_dimensions[0]);
                            uint16_t vertical_height = arg_dimensions[0].ascent + arg_dimensions[0].descent + SQRT_TOP_BAR_MARGIN + SQRT_TOP_BAR_THICKNESS;
                            uint16_t top_bar_y = baseline_y - arg_dimensions[0].ascent - SQRT_TOP_BAR_MARGIN - SQRT_TOP_BAR_THICKNESS;
                            uint16_t expression_x = *x + SQRT_TICK_WIDTH + SQRT_VERTICAL_BAR_THICKNESS + SQRT_TICK_EXPRESSION_MARGIN;

                            // tick
                            st7789_draw_horizontal_line(display, 
                                *x, 
                                baseline_y - SQRT_TOP_BAR_THICKNESS, 
                                SQRT_TICK_WIDTH, 
                                SQRT_TOP_BAR_THICKNESS, 
                                font_color, 
                                LINE_STYLE_SOLID, 
                                bg_color
                            );

                            // vertical line
                            st7789_draw_vertical_line(display, 
                                *x + SQRT_TICK_WIDTH, 
                                baseline_y - vertical_height, 
                                vertical_height, 
                                SQRT_TOP_BAR_THICKNESS, 
                                font_color, 
                                LINE_STYLE_SOLID, 
                                bg_color
                            );

                            // top bar
                            st7789_draw_horizontal_line(display, 
                                *x + SQRT_TICK_WIDTH + SQRT_VERTICAL_BAR_THICKNESS,
                                top_bar_y,
                                arg_dimensions[0].w + SQRT_TICK_EXPRESSION_MARGIN,
                                SQRT_TOP_BAR_THICKNESS,
                                font_color,
                                LINE_STYLE_SOLID,
                                bg_color
                            );

                            *x = expression_x;
                            draw_expression_impl(display, expr, index, x, baseline_y, arg_dimensions[0], font_scale, font_color, bg_color, draw_cursor, cursor);
                            *x += SQRT_MARGIN_AFTER_EXPRESSION + SQRT_MARGIN_AFTER_SQRT;

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

static ExpressionDimensions get_sqrt_dimensions(ExpressionDimensions arg) {
    ExpressionDimensions dimensions;

    dimensions.w = SQRT_TICK_WIDTH + 
        SQRT_VERTICAL_BAR_THICKNESS + 
        SQRT_TICK_EXPRESSION_MARGIN + 
        arg.w + 
        SQRT_MARGIN_AFTER_EXPRESSION +
        SQRT_MARGIN_AFTER_SQRT;
    dimensions.ascent = SQRT_TOP_BAR_MARGIN + SQRT_TOP_BAR_THICKNESS + arg.ascent;
    dimensions.descent = arg.descent;

    return dimensions;
}