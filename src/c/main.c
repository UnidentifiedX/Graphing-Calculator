#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "calculator/pages/pages.h"
#include "calculator/pages/calculator_page.h"
#include "virtual_keypad/virtual_keypad.h"

int main()
{
    stdio_init_all();

    // while(!stdio_usb_connected()) {
    //     sleep_ms(100);
    // }

    PageManager page_manager;
    init_page_manager(&page_manager);
    set_page(&page_manager, &calculator_page);

    uint64_t last_tick = time_us_64();
    while (true) {
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            Keystroke key = char_to_keystroke((char)c);

            if (key != KEYSTROKE_INVALID) {
                page_manager.current_page->on_key(page_manager.current_page, &page_manager, key);
            }
        }

        uint64_t now = time_us_64();
        if (now - last_tick >= 600000) { // 600 ms
            last_tick = now;
            page_manager.current_page->on_tick(page_manager.current_page, &page_manager);
        }
    }
}