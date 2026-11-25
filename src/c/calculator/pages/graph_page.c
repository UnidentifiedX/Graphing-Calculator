#include "graph_page.h"
#include "st7789/st7789.h"
#include "calculator/keystrokes.h"
#include "calculator/pages/graphing_page.h"
#include "calculator/pages/calculator_page.h"
#include "calculator/ui/graph_drawer.h"

static const uint16_t GRAPH_PAGE_EXPRESSION_BG_COLOR = COLOR565(255, 255, 255); // white

static void keystroke_to_action(PageManager* manager, Keystroke key);
static void plot_expressions(ST7789* display);

typedef struct {
    double x_min;
    double x_max;
    double y_min;
    double y_max;
} GraphPageData;

static GraphPageData graph_page_data = {
    .x_min = -10.0,
    .x_max = 10.0,
    .y_min = -10.0,
    .y_max = 10.0
};

static void init(Page* self, PageManager* manager) {
    ST7789* display = &manager->display;
    clear_display(display, GRAPH_PAGE_EXPRESSION_BG_COLOR, true);
    plot_expressions(display);
}

static void on_key(Page* self, PageManager* manager, Keystroke key) {
    keystroke_to_action(manager, key);
}

static void on_tick(Page* self, PageManager* manager) {
    
}

Page graph_page = {
    .init = init,
    .on_key = on_key,
    .on_tick = on_tick,
    .data = &graph_page_data
};

static void keystroke_to_action(PageManager* manager, Keystroke key) {
    switch (key) {
        case KEYSTROKE_QUIT: set_page(manager, &calculator_page); return;
        case KEYSTROKE_GRAPH: set_page(manager, &graphing_page); return;
    }
}

static void plot_expressions(ST7789* display) {
    clear_display(display, GRAPH_PAGE_EXPRESSION_BG_COLOR, true);
    draw_axes(display, 0, 0, ST7789_WIDTH, ST7789_HEIGHT, graph_page_data.x_min, graph_page_data.x_max, graph_page_data.y_min, graph_page_data.y_max);

    for (size_t i = 0; i < GRAPHING_PAGE_MAX_PLOTS; i++) {
        PlottingExpression* expression = &((GraphingPageData*)graphing_page.data)->plotting_expressions[i];

        if (expression->expression.size > 0) {
            plot_expression(display, &expression->expression, 0, 0, ST7789_WIDTH, ST7789_HEIGHT,
                            graph_page_data.x_min, graph_page_data.x_max,
                            graph_page_data.y_min, graph_page_data.y_max,
                            line_colors[i]);
        }
    }
}