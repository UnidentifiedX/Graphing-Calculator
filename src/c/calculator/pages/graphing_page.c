#include "pages.h"
#include "st7789/st7789.h"
#include "calculator/keystrokes.h"
#include "calculator/pages/calculator_page.h"
#include "calculator/ui/expression_drawer.h"
#include "calculator/ui/cursor.h"

static const uint16_t GRAPHING_PAGE_CURSOR_THICKNESS      = 1;

static const uint16_t GRAPHING_PAGE_EXPRESSION_FONT_COLOR = COLOR565(0, 0, 0); // black
static const uint16_t GRAPHING_PAGE_EXPRESSION_BG_COLOR   = COLOR565(255, 255, 255); // white
static const uint16_t GRAPHING_PAGE_EXPRESSION_FONT_SCALE = 2;
static const uint16_t GRAPHING_PAGE_LEFT_MARGIN           = 5; // margin from the color label box to the expression
static const uint16_t GRAPHING_PAGE_RIGHT_MARGIN          = 5; // margin from the right edge of the screen to the expression

static const uint16_t SEPARATOR_LINE_MARGIN               = 10; // leave some margin between the separator line and expressions
static const uint16_t SEPARATOR_LINE_THICKNESS            = 1; // thickness of the separator line
static const uint16_t LINE_COLOR_BOX_WIDTH                = 5; // width of the color label box

#define GRAPHING_PAGE_MAX_PLOTS 3

static void keystroke_to_action(PageManager* manager, Keystroke key);
static void render_expressions(ST7789* display);

static uint16_t line_colors[GRAPHING_PAGE_MAX_PLOTS] = {
    COLOR565(255, 0, 0), // Red
    COLOR565(0, 255, 0), // Green
    COLOR565(0, 0, 255), // Blue
    // COLOR565(255, 255, 0), // Yellow
    // COLOR565(255, 0, 255)  // Magenta
};

typedef struct {
    TokenExpression expression;
    ExpressionDimensions expression_dimensions;
} PlottingExpression;

typedef struct {
    PlottingExpression plotting_expressions[GRAPHING_PAGE_MAX_PLOTS];
    size_t current_selected_plot_index;

    Cursor cursor;
} GraphingPageData;

static GraphingPageData graphing_page_data = {
    .current_selected_plot_index = 0,
    .cursor = {0}
};

static void init(Page* self, PageManager* manager) {
    ST7789* display = &manager->display;
    clear_display(display, GRAPHING_PAGE_EXPRESSION_BG_COLOR);
    init_cursor(&graphing_page_data.cursor, display, GRAPHING_PAGE_EXPRESSION_FONT_SCALE * FONT_HEIGHT_6x8, GRAPHING_PAGE_CURSOR_THICKNESS);
    render_expressions(display);
    render_top_bar(display);
    enable_cursor(&graphing_page_data.cursor);
}

static void on_key(Page* self, PageManager* manager, Keystroke key) {
    keystroke_to_action(manager, key);
}

static void on_tick(Page* self, PageManager* manager) {
    toggle_cursor(&graphing_page_data.cursor);
}

Page graphing_page = {
    .init = init,
    .on_key = on_key,
    .on_tick = on_tick,
    .data = &graphing_page_data
};

// The page should look like a list of plottable expressions from the top of the screen to the bottom
// Each expression has its own colour which is a nice little box/label on the left of the expression
static void render_expressions(ST7789* display) {
    // temporarily disable the cursor
    bool was_cursor_enabled = graphing_page_data.cursor.enabled;
    disable_cursor(&graphing_page_data.cursor);

    // render all plotting expressions from the top of the screen to the bottom
    uint16_t y_offset = TOP_BAR_HEIGHT; // starting offset from the top of the screen

    for (size_t i = 0; i < GRAPHING_PAGE_MAX_PLOTS; i++) {
        y_offset += SEPARATOR_LINE_MARGIN; // add margin before each expression

        PlottingExpression* plot_expression = &graphing_page_data.plotting_expressions[i];

        uint16_t expression_height = 0;

        if (plot_expression->expression.size == 0) {
            expression_height = GRAPHING_PAGE_EXPRESSION_FONT_SCALE * FONT_HEIGHT_6x8; // default height if the expression is empty
        } else {
            if (i == graphing_page_data.current_selected_plot_index) { // means we are rendering the currently selected expression. need to recalculate its dimensions
                plot_expression->expression_dimensions = calculate_dimensions(&plot_expression->expression, GRAPHING_PAGE_EXPRESSION_FONT_SCALE);
            }
            expression_height = plot_expression->expression_dimensions.ascent + plot_expression->expression_dimensions.descent;
        }

        draw_expression(display,
            &plot_expression->expression,
            GRAPHING_PAGE_LEFT_MARGIN + LINE_COLOR_BOX_WIDTH, // leave space for the color label box
            y_offset + expression_height, // baseline_y
            plot_expression->expression_dimensions,
            GRAPHING_PAGE_EXPRESSION_FONT_SCALE,
            GRAPHING_PAGE_EXPRESSION_FONT_COLOR,
            GRAPHING_PAGE_EXPRESSION_BG_COLOR,
            i == graphing_page_data.current_selected_plot_index, // don't draw cursor for now
            &graphing_page_data.cursor
        );

        // draw colour label box
        st7789_fill_rect(display, 0, y_offset, LINE_COLOR_BOX_WIDTH, expression_height, line_colors[i]);
        y_offset += expression_height;
        y_offset += SEPARATOR_LINE_MARGIN; // add margin after each expression
        // draw separator line
        st7789_draw_horizontal_line(display, 
            GRAPHING_PAGE_LEFT_MARGIN,
            y_offset,
            ST7789_WIDTH - GRAPHING_PAGE_LEFT_MARGIN - GRAPHING_PAGE_RIGHT_MARGIN,
            SEPARATOR_LINE_THICKNESS,
            GRAPHING_PAGE_EXPRESSION_FONT_COLOR,
            LINE_STYLE_DOTTED,
            GRAPHING_PAGE_EXPRESSION_BG_COLOR
        );
        y_offset += SEPARATOR_LINE_THICKNESS; // add the height of the separator line
    }

    // restore cursor state
    if (was_cursor_enabled) {
        enable_cursor(&graphing_page_data.cursor);
    }
}

static void keystroke_to_action(PageManager* manager, Keystroke key) {
    TokenExpression* expr = &graphing_page_data.plotting_expressions[graphing_page_data.current_selected_plot_index].expression;

    switch (key) {
        case KEYSTROKE_QUIT: set_page(manager, &calculator_page); return;
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
        case KEYSTROKE_SQRT:              insert_node(expr, &(Node){ .type = NODE_TYPE_FUNCTION, .node.func_identifier = "sqrt" }); break;
        case KEYSTROKE_NRT:               insert_node(expr, &(Node){ .type = NODE_TYPE_FUNCTION, .node.func_identifier = "nrt" }); break;
        case KEYSTROKE_FRAC:              insert_node(expr, &(Node){ .type = NODE_TYPE_FUNCTION, .node.func_identifier = "frac" }); break;
        case KEYSTROKE_ABS:               insert_node(expr, &(Node){ .type = NODE_TYPE_FUNCTION, .node.func_identifier = "abs" }); break;
        case KEYSTROKE_POW:               insert_node(expr, &(Node){ .type = NODE_TYPE_FUNCTION, .node.func_identifier = "pow" }); break;
        case KEYSTROKE_X:                 insert_node(expr, &(Node){ .type = NODE_TYPE_VARIABLE, .node.var_identifier = "x" }); break;
        case KEYSTROKE_PI:                insert_node(expr, &(Node){ .type = NODE_TYPE_VARIABLE, .node.var_identifier = "pi" }); break;
        case KEYSTROKE_E:                 insert_node(expr, &(Node){ .type = NODE_TYPE_VARIABLE, .node.var_identifier = "e" }); break;
        case KEYSTROKE_ENTER: {
            break;
        }
        case KEYSTROKE_BACKSPACE: backspace_node(expr); break;
        case KEYSTROKE_CLEAR: {
            // clear the current expression
            if (expr->size == 0) {
                clear_display(&manager->display, GRAPHING_PAGE_EXPRESSION_BG_COLOR);
            }
            reset_token_expression(expr);
            break;
        }
        case KEYSTROKE_LEFT: shift_cursor(expr, CURSOR_DIRECTION_LEFT); break;
        case KEYSTROKE_RIGHT: shift_cursor(expr, CURSOR_DIRECTION_RIGHT); break;
        case KEYSTROKE_DOWN: {
            if (graphing_page_data.current_selected_plot_index < GRAPHING_PAGE_MAX_PLOTS - 1) {
                graphing_page_data.current_selected_plot_index++;
            }
            break;
        }
        case KEYSTROKE_UP: {
            if (graphing_page_data.current_selected_plot_index > 0) {
                graphing_page_data.current_selected_plot_index--;
            }
            break;
        }
        default: return;
    }

    clear_display(&manager->display, GRAPHING_PAGE_EXPRESSION_BG_COLOR);
    render_expressions(&manager->display);
}