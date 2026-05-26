#pragma once
#include "drivers/st7789/st7789.h"

enum class UIPageType {
    Main
};

struct UIPage {
    UIPageType type;

    UIPage(UIPageType type);
    virtual ~UIPage() = default;
    virtual void draw(ST7789& display) = 0;
};

struct MainCalculatorPage : UIPage {
    MainCalculatorPage();

    void draw(ST7789& display) override;
};