#include "pages.h"
#include "st7789/st7789.h"
#include "calculator/keystrokes.h"
#include "calculator/evaluator/token_expression.h"
#include "calculator/ui/cursor.h"
#include "core/parser.h"
#include "core/evaluator.h"
#include <stdio.h>
#include "calculator/ui/expression_drawer.h"
#include "core/inbuilt_functions.h"

#define CALCULATOR_PAGE_MAX_HISTORY_SIZE 4

static const uint16_t CALCULATOR_PAGE_CURSOR_THICKNESS = 1;
static const uint16_t CALCULATOR_PAGE_CURSOR_HEIGHT = 20;
static const uint16_t CALCULATOR_PAGE_LEFT_MARGIN = 5;
static const uint16_t CALCULATOR_PAGE_RIGHT_MARGIN = 10;
static const uint16_t CALCULATOR_PAGE_TOP_MARGIN = 5;
static const uint16_t CALCULATOR_PAGE_BOTTOM_MARGIN = 5;
static const uint16_t CALCULATOR_PAGE_EXPRESSION_FONT_SCALE = 2;
#define CALCULATOR_PAGE_EXPRESSION_FONT_COLOR color565(0, 0, 0)
#define CALCULATOR_PAGE_EXPRESSION_BG_COLOR color565(255, 255, 255)

static const uint16_t EXPRESSION_RESULT_MARGIN = 2; // leave some margin between expression and result
static const uint16_t SEPARATOR_LINE_MARGIN = 4; // leave some margin between the separator line and expressions
static const uint16_t SEPARATOR_LINE_THICKNESS = 1; // thickness of the separator line

typedef struct {
    TokenExpression expression;
    TokenExpression result;
    ExpressionDimensions expression_dimensions;
    ExpressionDimensions result_dimensions;
} ExpressionResult;

typedef struct {
    ExpressionResult history[CALCULATOR_PAGE_MAX_HISTORY_SIZE];
    size_t history_size;
    size_t history_latest; // points to the most recent expression for circular buffer

    size_t browsing_history_index; // when viewing history, this points to the expression being viewed

    TokenExpression current_expression;

    Cursor cursor;
} CalculatorPageData;

static CalculatorPageData calculator_page_data = {
    .history_size = 0,
    .browsing_history_index = 0,
    .current_expression = (TokenExpression){
        .node_index = 0,
        .char_index = __SIZE_MAX__,
        .size = 0
    },
    .cursor = {0}
};

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} Alignment;

static void render_expressions(ST7789* display);
static void keystroke_to_action(PageManager* manager, Keystroke key);

static void init(Page* self, PageManager* manager) {
    ST7789* display = &manager->display;
    init_cursor(&calculator_page_data.cursor, display, CALCULATOR_PAGE_CURSOR_HEIGHT, CALCULATOR_PAGE_CURSOR_THICKNESS);
    render_top_bar(display);
    render_expressions(display);
    enable_cursor(&calculator_page_data.cursor);
}

static void on_key(Page* self, PageManager* manager, Keystroke key) {
    keystroke_to_action(manager, key);
}

static void on_tick(Page* self, PageManager* manager) {
    toggle_cursor(&calculator_page_data.cursor);
}

Page calculator_page = {
    .init = init,
    .on_key = on_key,
    .on_tick = on_tick,
    .data = &calculator_page_data
};

static void clear_display(ST7789* display) {
    st7789_fill_rect(display, 0, 0, ST7789_WIDTH, ST7789_HEIGHT, CALCULATOR_PAGE_EXPRESSION_BG_COLOR);
    render_top_bar(display);
}

static void clear_history() {
    calculator_page_data.history_size = 0;
    calculator_page_data.history_latest = 0;
    calculator_page_data.browsing_history_index = 0;
}

/// @brief returns the height of the current expresion in pixels
/// @param display 
/// @param expr 
/// @param y_offset offset from the bottom of the screen in pixels
/// @param alignment 
/// @param draw_cursor 
/// @return 
static void render_expression(ST7789* display, TokenExpression* expr, ExpressionDimensions dimensions, uint16_t y_offset, Alignment alignment, bool draw_cursor) {
    uint16_t x = CALCULATOR_PAGE_LEFT_MARGIN;

    if (alignment == ALIGN_RIGHT) {
        x = ST7789_WIDTH - CALCULATOR_PAGE_RIGHT_MARGIN - dimensions.w;
    }
    
    draw_expression(display, 
        expr,
        x,
        ST7789_HEIGHT - dimensions.h - y_offset,
        dimensions,
        CALCULATOR_PAGE_EXPRESSION_FONT_SCALE,
        CALCULATOR_PAGE_EXPRESSION_FONT_COLOR,
        CALCULATOR_PAGE_EXPRESSION_BG_COLOR,
        draw_cursor,
        &calculator_page_data.cursor
    );
}

/*
I think the general logic behind this is that we render things from the bottom of the screen to the top, and
we keep track of the height of the rendered expressions and only render the history that fits in the screen
*/
static void render_expressions(ST7789* display) {
    // temporarily disable the cursor
    bool was_cursor_enabled = calculator_page_data.cursor.enabled;
    disable_cursor(&calculator_page_data.cursor);

    // render the current expression at the bottom of the screen
    ExpressionDimensions current_dimensions = calculate_dimensions(&calculator_page_data.current_expression, CALCULATOR_PAGE_EXPRESSION_FONT_SCALE);
    uint16_t height = current_dimensions.h;
    render_expression(display, &calculator_page_data.current_expression, current_dimensions, CALCULATOR_PAGE_BOTTOM_MARGIN, ALIGN_LEFT, true);
    height += 8 - SEPARATOR_LINE_MARGIN; // some space for the cursor

    // render history from the bottom of the screen to the top
    /// TODO: fix rendering too many history expressions that don't fit on the screen
    for (size_t i = 0; i < calculator_page_data.history_size; i++) {
        // horizonatal seperator first
        st7789_draw_horizontal_line(display, 
            CALCULATOR_PAGE_LEFT_MARGIN,
            ST7789_HEIGHT - CALCULATOR_PAGE_BOTTOM_MARGIN - height - SEPARATOR_LINE_MARGIN,
            ST7789_WIDTH - CALCULATOR_PAGE_LEFT_MARGIN - CALCULATOR_PAGE_RIGHT_MARGIN,
            SEPARATOR_LINE_THICKNESS,
            CALCULATOR_PAGE_EXPRESSION_FONT_COLOR,
            LINE_STYLE_DOTTED,
            CALCULATOR_PAGE_EXPRESSION_BG_COLOR
        );
        height += SEPARATOR_LINE_MARGIN + SEPARATOR_LINE_THICKNESS; // add the height of the separator line and the margin

        // then result
        size_t index = (calculator_page_data.history_latest + CALCULATOR_PAGE_MAX_HISTORY_SIZE - i) % CALCULATOR_PAGE_MAX_HISTORY_SIZE;
        ExpressionResult* result = &calculator_page_data.history[index];
        height += SEPARATOR_LINE_MARGIN;
        ExpressionDimensions result_dimensions = result->result_dimensions;
        render_expression(display, &result->result, result_dimensions, height, ALIGN_RIGHT, false);
        height += result_dimensions.h;

        // then expression
        height += EXPRESSION_RESULT_MARGIN;
        ExpressionDimensions expression_dimensions = result->expression_dimensions;
        render_expression(display, &result->expression, expression_dimensions, height, ALIGN_LEFT, false);
        height += expression_dimensions.h;
    }

    // restore the cursor state
    if (was_cursor_enabled) {
        enable_cursor(&calculator_page_data.cursor);
    }
}

static void keystroke_to_action(PageManager* manager, Keystroke key) {
    TokenExpression* expr = &calculator_page_data.current_expression;

    switch (key) {
        case KEYSTROKE_0:                 insert_node(expr, &(Node){ .type = NODE_TYPE_NUMBER, .node.num_str = "0" }); break;
        case KEYSTROKE_1:                 insert_node(expr, &(Node){ .type = NODE_TYPE_NUMBER, .node.num_str = "1" }); break;
        case KEYSTROKE_2:                 insert_node(expr, &(Node){ .type = NODE_TYPE_NUMBER, .node.num_str = "2" }); break;
        case KEYSTROKE_3:                 insert_node(expr, &(Node){ .type = NODE_TYPE_NUMBER, .node.num_str = "3" }); break;
        case KEYSTROKE_4:                 insert_node(expr, &(Node){ .type = NODE_TYPE_NUMBER, .node.num_str = "4" }); break;
        case KEYSTROKE_5:                 insert_node(expr, &(Node){ .type = NODE_TYPE_NUMBER, .node.num_str = "5" }); break;
        case KEYSTROKE_6:                 insert_node(expr, &(Node){ .type = NODE_TYPE_NUMBER, .node.num_str = "6" }); break;
        case KEYSTROKE_7:                 insert_node(expr, &(Node){ .type = NODE_TYPE_NUMBER, .node.num_str = "7" }); break;
        case KEYSTROKE_8:                 insert_node(expr, &(Node){ .type = NODE_TYPE_NUMBER, .node.num_str = "8" }); break;
        case KEYSTROKE_9:                 insert_node(expr, &(Node){ .type = NODE_TYPE_NUMBER, .node.num_str = "9" }); break;
        case KEYSTROKE_DECIMAL_POINT:     insert_node(expr, &(Node){ .type = NODE_TYPE_NUMBER, .node.num_str = "." }); break;
        case KEYSTROKE_OPEN_PARENTHESIS:  insert_node(expr, &(Node){ .type = NODE_TYPE_ATOM, .node.token = (SyntaxToken){ .kind = SYNTAX_KIND_OPEN_PARENTHESIS } }); break;
        case KEYSTROKE_CLOSE_PARENTHESIS: insert_node(expr, &(Node){ .type = NODE_TYPE_ATOM, .node.token = (SyntaxToken){ .kind = SYNTAX_KIND_CLOSE_PARENTHESIS } }); break;
        case KEYSTROKE_PLUS:              insert_node(expr, &(Node){ .type = NODE_TYPE_ATOM, .node.token = (SyntaxToken){ .kind = SYNTAX_KIND_PLUS } }); break;
        case KEYSTROKE_MINUS:             insert_node(expr, &(Node){ .type = NODE_TYPE_ATOM, .node.token = (SyntaxToken){ .kind = SYNTAX_KIND_MINUS } }); break;
        case KEYSTROKE_MULTIPLY:          insert_node(expr, &(Node){ .type = NODE_TYPE_ATOM, .node.token = (SyntaxToken){ .kind = SYNTAX_KIND_MULTIPLY } }); break;
        case KEYSTROKE_DIVIDE:            insert_node(expr, &(Node){ .type = NODE_TYPE_ATOM, .node.token = (SyntaxToken){ .kind = SYNTAX_KIND_DIVIDE } }); break;
        case KEYSTROKE_SIN:               insert_node(expr, &(Node){ .type = NODE_TYPE_FUNCTION, .node.func_identifier = "sin" }); break;
        case KEYSTROKE_COS:               insert_node(expr, &(Node){ .type = NODE_TYPE_FUNCTION, .node.func_identifier = "cos" }); break;
        case KEYSTROKE_TAN:               insert_node(expr, &(Node){ .type = NODE_TYPE_FUNCTION, .node.func_identifier = "tan" }); break;
        case KEYSTROKE_LN:                insert_node(expr, &(Node){ .type = NODE_TYPE_FUNCTION, .node.func_identifier = "ln" }); break;
        case KEYSTROKE_LOG10:             insert_node(expr, &(Node){ .type = NODE_TYPE_FUNCTION, .node.func_identifier = "log" }); break;
        case KEYSTROKE_PI:                insert_node(expr, &(Node){ .type = NODE_TYPE_VARIABLE, .node.var_identifier = "pi" }); break;
        case KEYSTROKE_ENTER: {
            Expression expressions[256];
            SyntaxToken tokens[256];
            size_t token_count = to_tokens(expr, tokens, 256);

            Parser parser;
            init_parser(&parser, tokens, token_count);
            size_t root_index = parse(&parser, expressions, 256);
            double result = evaluate(expressions, root_index);

            // for now since all results are numbers we make the output an expression with a single number node
            TokenExpression result_expr;
            init_token_expression(&result_expr);
            Node result_node = {
                .type = NODE_TYPE_NUMBER
            };
            snprintf(result_node.node.num_str, sizeof(result_node.node.num_str), "%g", result);
            insert_node(&result_expr, &result_node);

            // circular buffer for history. [0] is the oldest, [history_size - 1] is the most recent. if overflow, we overwrite starting from [0]
            // current_history_index points to the most recent expression in the history
            size_t index = calculator_page_data.history_size == 0 ? 0 : (calculator_page_data.history_latest + 1) % CALCULATOR_PAGE_MAX_HISTORY_SIZE;
            calculator_page_data.history[index] = (ExpressionResult){
                .expression = *expr,
                .result = result_expr,
                .expression_dimensions = calculate_dimensions(expr, CALCULATOR_PAGE_EXPRESSION_FONT_SCALE),
                .result_dimensions = calculate_dimensions(&result_expr, CALCULATOR_PAGE_EXPRESSION_FONT_SCALE)
            };

            calculator_page_data.history_latest = index;
            if (calculator_page_data.history_size < CALCULATOR_PAGE_MAX_HISTORY_SIZE) {
                calculator_page_data.history_size++;
            }

            reset_token_expression(expr);
            break;
        }
        case KEYSTROKE_BACKSPACE: backspace_node(expr); break;
        case KEYSTROKE_CLEAR: {
            // clear the current expression. if clear clear the whole history as well
            if (expr->size == 0) {
                clear_display(&manager->display);
                clear_history();
            }
            reset_token_expression(expr);
            break;
        }
        case KEYSTROKE_LEFT: shift_cursor(expr, CURSOR_DIRECTION_LEFT); break;
        case KEYSTROKE_RIGHT: shift_cursor(expr, CURSOR_DIRECTION_RIGHT); break;
        default: break;
    }

    printf("Current expression: ");
    output_token_expression(expr);
    printf("Current cursors: node_index = %zu, char_index = %zu\n", expr->node_index, expr->char_index);
    printf("\n");
    render_expressions(&manager->display);
}