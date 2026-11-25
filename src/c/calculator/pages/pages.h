#pragma once

#include <stdbool.h>
#include "st7789/st7789.h"
#include "calculator/keystrokes.h"
#include "calculator/evaluator/token_expression.h"
#include "core/evaluator.h"

static const uint16_t TOP_BAR_HEIGHT = 30;
static const uint16_t TOP_BAR_TOP_MARGIN = 2;
static const uint16_t TOP_BAR_LEFT_MARGIN = 2;

typedef struct Page Page;

typedef struct {
    Page* current_page;
    ST7789 display;
} PageManager;

typedef struct Page {
    void (*init)(Page* self, PageManager* manager);
    void (*on_key)(Page* self, PageManager* manager, Keystroke key);
    void (*on_tick)(Page* self, PageManager* manager);
    void* data;
} Page;

void init_page_manager(PageManager* manager);
void set_page(PageManager* manager, Page* page);
void render_top_bar(ST7789* display);
void clear_display(ST7789* display, uint16_t bg_color, bool clear_top_bar);