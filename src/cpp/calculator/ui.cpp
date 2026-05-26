#include "ui.h"
#include "drivers/st7789/fonts.h"

constexpr uint16_t HEADER_BACKGROUND_COLOR = color565(128, 128, 128);
constexpr uint16_t BACKGROUND_COLOR = color565(255, 255, 255);

UIPage::UIPage(UIPageType type) : type(type) {}

MainCalculatorPage::MainCalculatorPage() : UIPage(UIPageType::Main) {}

void MainCalculatorPage::draw(ST7789& display) {
    display.fillScreen(BACKGROUND_COLOR);
    display.fillRect(0, 0, ST7789::WIDTH, 28, HEADER_BACKGROUND_COLOR); // Header background
    display.drawString(10, 10, "NORMAL FLOAT AUTO a+bi DEGREE", color565(0, 0, 0), 1, HEADER_BACKGROUND_COLOR); // Draw page title
    display.drawString(10, 50, "3.14159265", color565(0, 0, 0), 2);

    display.drawHorizontalLine(0, 90, ST7789::WIDTH, 1, color565(0, 0, 0), LineStyle::Solid); // Separator line
    display.drawHorizontalLine(0, 100, ST7789::WIDTH, 2, color565(0, 0, 0), LineStyle::Dashed); // Separator line
    display.drawHorizontalLine(0, 110, ST7789::WIDTH, 3, color565(0, 0, 0), LineStyle::Dotted); // Separator line
}