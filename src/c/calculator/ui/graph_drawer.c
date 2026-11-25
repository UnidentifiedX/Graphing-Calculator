#include "calculator/ui/graph_drawer.h"
#include "st7789/st7789.h"
#include "calculator/evaluator/token_expression.h"
#include "core/evaluator.h"
#include <math.h>
#include <stdbool.h>

static const uint16_t TICK_LENGTH = 4;
static const uint16_t TICK_WIDTH  = 1;
static const uint16_t AXIS_WIDTH  = 1;

static const uint16_t MAX_VERTICAL_JUMP_PIXELS = ST7789_HEIGHT / 3;
static const uint16_t PIXEL_SIZE = 2;

// Choose a nice tick interval: 1, 2, 5, 10, 20, 50, etc.
static double calculate_tick_interval(double range, uint16_t pixel_span) {
    if (range <= 0 || pixel_span == 0) return 1.0;

    // Aim for approximately 60 pixels between major ticks.
    double raw_interval = range * 60.0 / pixel_span;

    double magnitude = pow(10.0, floor(log10(raw_interval)));
    double normalized = raw_interval / magnitude;

    if (normalized <= 1.0)
        return magnitude;
    else if (normalized <= 2.0)
        return 2.0 * magnitude;
    else if (normalized <= 5.0)
        return 5.0 * magnitude;
    else
        return 10.0 * magnitude;
}


// (x1, y1) is top-left; (x2, y2) is bottom-right.
void draw_axes(ST7789* display,
    uint16_t x1, uint16_t y1,
    uint16_t x2, uint16_t y2,
    double x_min, double x_max,
    double y_min, double y_max
) {
    if (x_max <= x_min || y_max <= y_min || x2 <= x1 || y2 <= y1)
        return;

    double dx = (x_max - x_min) / (double)(x2 - x1);
    double dy = (y_max - y_min) / (double)(y2 - y1);

    uint16_t width = x2 - x1;
    uint16_t height = y2 - y1;

    bool y_axis_visible = (x_min <= 0 && x_max >= 0);
    bool x_axis_visible = (y_min <= 0 && y_max >= 0);

    uint16_t y_axis_x = x1;
    uint16_t x_axis_y = y2;

    if (y_axis_visible) y_axis_x = x1 + (uint16_t)lround((0.0 - x_min) / dx);
    if (x_axis_visible) x_axis_y = y2 - (uint16_t)lround((0.0 - y_min) / dy);

    // Draw axes.
    if (y_axis_visible) {
        st7789_draw_vertical_line(display, y_axis_x, y1, height, AXIS_WIDTH, COLOR565(0, 0, 0), LINE_STYLE_SOLID, COLOR565(255, 255, 255));
    }

    if (x_axis_visible) {
        st7789_draw_horizontal_line(display, x1, x_axis_y, width, AXIS_WIDTH, COLOR565(0, 0, 0), LINE_STYLE_SOLID, COLOR565(255, 255, 255));
    }

    // If an axis isn't visible, put its ticks on the nearest border.
    uint16_t tick_x_axis = x_axis_visible ? x_axis_y : (y_min > 0 ? y2 : y1);
    uint16_t tick_y_axis = y_axis_visible ? y_axis_x : (x_min > 0 ? x1 : x2);

    // Calculate major tick intervals.
    double x_tick_interval = 1;
    double y_tick_interval = 1;

    uint16_t tick_color = COLOR565(0, 0, 0);
    uint16_t background = COLOR565(255, 255, 255);

    // Draw X-axis ticks.
    double first_x_tick = ceil(x_min / x_tick_interval) * x_tick_interval;

    for (double x = first_x_tick; x <= x_max + x_tick_interval * 1e-9; x += x_tick_interval) {
        uint16_t px = x1 + (uint16_t)lround((x - x_min) / dx);

        if (px > x2) continue;

        uint16_t start_y;
        if (!x_axis_visible) {
            // Ticks point inward from the graph border.
            start_y = (tick_x_axis == y1) ? y1 : y2 - TICK_LENGTH;
        } else {
            // Ticks are centred on the axis.
            start_y = tick_x_axis >= y1 + TICK_LENGTH / 2 ? tick_x_axis - TICK_LENGTH / 2 : y1;

            if (start_y + TICK_LENGTH - 1 > y2)
                start_y = y2 - TICK_LENGTH;
        }

        st7789_draw_vertical_line(display, px, start_y, TICK_LENGTH, TICK_WIDTH, tick_color, LINE_STYLE_SOLID, background);
    }

    // Draw Y-axis ticks.
    double first_y_tick = ceil(y_min / y_tick_interval) * y_tick_interval;

    for (double y = first_y_tick; y <= y_max + y_tick_interval * 1e-9; y += y_tick_interval) {
        uint16_t py = y2 - (uint16_t)lround((y - y_min) / dy);

        if (py < y1 || py > y2) continue;

        uint16_t start_x;

        if (!y_axis_visible) {
            // Ticks point inward from the graph border.
            start_x = (tick_y_axis == x1) ? x1 : x2 - TICK_LENGTH;
        } else {
            // Ticks are centred on the axis.
            start_x = tick_y_axis >= x1 + TICK_LENGTH / 2 ? tick_y_axis - TICK_LENGTH / 2 : x1;

            if (start_x + TICK_LENGTH - 1 > x2)
                start_x = x2 - TICK_LENGTH;
        }

        st7789_draw_horizontal_line(
            display, start_x, py, TICK_LENGTH,
            TICK_WIDTH, tick_color,
            LINE_STYLE_SOLID, background
        );
    }
}

// (x1, y1) is the top-left corner of the graphing area, (x2, y2) is the bottom-right corner of the graphing area
void plot_expression(ST7789* display, TokenExpression* expr, 
                        uint16_t x1, uint16_t y1, 
                        uint16_t x2, uint16_t y2,
                        double x_min, double x_max,
                        double y_min, double y_max,
                        uint16_t color) {
    double dx = (x_max - x_min) / (double)(x2 - x1);
    double dy = (y_max - y_min) / (double)(y2 - y1);

    Variable variable = {.name = "x", .value = 0};
    VariableEnvironment env = { .variables = &variable, .count = 1 };

    uint16_t prev_x_pixel = 0, prev_y_pixel = 0;
    double prev_y_value = 0;
    bool prev_defined = false;   // false at start and after NaN

    for (uint16_t x_pixel = x1; x_pixel < x2; x_pixel++) {
        variable.value = x_min + (x_pixel - x1) * dx;
        double y_value = evaluate_token_expression(expr, &env);

        // NaN (e.g. sqrt(-1), 0/0): nothing to draw, break the line
        if (isnan(y_value)) {
            prev_defined = false;
            continue;
        }

        bool in_range = (y_value >= y_min && y_value <= y_max);

        // clamp in double space FIRST, then convert (works for +/-inf too)
        double y_clamped = y_value < y_min ? y_min
                         : y_value > y_max ? y_max
                         : y_value;
        uint16_t y_pixel = y2 - (uint16_t)((y_clamped - y_min) / dy);

        if (in_range) {
            st7789_fill_rect(display, x_pixel, y_pixel, PIXEL_SIZE, PIXEL_SIZE, color);
        }

        if (prev_defined) {
            bool prev_in_range = (prev_y_value >= y_min && prev_y_value <= y_max);

            bool draw_segment;
            if (in_range && prev_in_range) {
                // both on screen: a huge jump here probably means an asymptote
                double jump_px = fabs(y_value - prev_y_value) / dy;
                draw_segment = (jump_px <= MAX_VERTICAL_JUMP_PIXELS);
            } else if (in_range || prev_in_range) { 
                draw_segment = true; // one on screen
            } else { 
                draw_segment = false; // both not on screen
            }

            if (draw_segment) {
                uint16_t vertical_jump = prev_y_pixel > y_pixel
                                    ? prev_y_pixel - y_pixel
                                    : y_pixel - prev_y_pixel;
                if (vertical_jump > PIXEL_SIZE) {
                    st7789_draw_vertical_line(display, prev_x_pixel,
                        prev_y_pixel > y_pixel ? y_pixel : prev_y_pixel,
                        vertical_jump, PIXEL_SIZE, color,
                        LINE_STYLE_SOLID, COLOR565(255, 255, 255));
                }
            }
        }

        prev_x_pixel = x_pixel;
        prev_y_pixel = y_pixel;
        prev_y_value = y_value;
        prev_defined = true;
    }
}