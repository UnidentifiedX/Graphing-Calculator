#include "pages.h"
#include "st7789/st7789.h"

static const uint8_t ST7789_PIN_CS    = 1;
static const uint8_t ST7789_PIN_SCK   = 2;
static const uint8_t ST7789_PIN_MOSI  = 3;

static const uint8_t ST7789_PIN_RESET = 8;
static const uint8_t ST7789_PIN_DC    = 9;
static const uint8_t ST7789_PIN_PWM   = 10;

#define TOP_BAR_BG_COLOR color565(64, 64, 64)

void init_page_manager(PageManager* manager) {
    manager->display = (ST7789){
        ._cs = ST7789_PIN_CS,
        ._dc = ST7789_PIN_DC,
        ._reset = ST7789_PIN_RESET,
        ._sck = ST7789_PIN_SCK,
        ._mosi = ST7789_PIN_MOSI,
        ._pwm = ST7789_PIN_PWM
    };

    st7789_init(&manager->display);
}

void set_page(PageManager* manager, Page* page) {
    manager->current_page = page;
    if (page->init) {
        page->init(page, manager);
    }
}

// shi used for info about the calc
void render_top_bar(ST7789* display) {
    st7789_fill_rect(display, 0, 0, ST7789_WIDTH, TOP_BAR_HEIGHT, TOP_BAR_BG_COLOR); // background
    st7789_draw_string(display, TOP_BAR_LEFT_MARGIN, TOP_BAR_TOP_MARGIN, "NORMAL FLOAT AUTO READ RADIAN MP", color565(255, 255, 255), 1, TOP_BAR_BG_COLOR); // info text
}