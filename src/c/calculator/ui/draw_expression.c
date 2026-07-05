#include "draw_expression.h"
#include "st7789/st7789.h"
#include <string.h>

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

void draw_expression(ST7789* display, 
                TokenExpression* expr, 
                uint16_t x, 
                uint16_t y,
                uint16_t h,
                uint16_t font_scale, 
                uint16_t font_color, 
                uint16_t bg_color, 
                bool draw_cursor, 
                Cursor* cursor) {
    // clear the background before drawing anything
    st7789_fill_rect(display, x, y, ST7789_WIDTH - x, h, bg_color);

    size_t cursor_y = y - (cursor->h - FONT_HEIGHT_6x8 * font_scale); // center the cursor vertically with respect to the text

    if (expr->size == 0) {
        if (draw_cursor) {
            set_cursor_position(cursor, x, cursor_y);
        }
        return;
    }
    
    for (size_t i = 0; i < expr->size; i++) {
        Node* node = &expr->node_buffer[i];

        switch (node->type) {
            case NODE_TYPE_NUMBER: {
                if (draw_cursor) {
                    if (i == expr->node_index) {
                        if (expr->char_index == __SIZE_MAX__) {
                            set_cursor_position(cursor, x - font_scale, cursor_y);
                        } else {
                            uint16_t cursor_x = x - font_scale + expr->char_index * FONT_WIDTH_6x8 * font_scale;
                            set_cursor_position(cursor, cursor_x, cursor_y);
                        }
                    }
                }

                st7789_draw_string(display, x, y, node->node.num_str, font_color, font_scale, bg_color);
                x += strlen(node->node.num_str) * FONT_WIDTH_6x8 * font_scale;
                break;
            }
            case NODE_TYPE_ATOM: {
                if (draw_cursor && i == expr->node_index) {
                    set_cursor_position(cursor, x - font_scale, cursor_y);
                }

                st7789_draw_char(display, x, y, *syntaxkind_to_string(node->node.token.kind), font_color, font_scale, bg_color);
                x += FONT_WIDTH_6x8 * font_scale;
                break;
            }
        }
    }

    if (draw_cursor && expr->node_index >= expr->size) {
        set_cursor_position(cursor, x - font_scale, cursor_y);
    }
}