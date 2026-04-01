#pragma once

class RGBLed {
public:
    enum Code {
        None = -1,
        BTN0 = 3,
        BTN1 = 4,
        BTN2 = 7,
        BTN3 = 8,
        BTN4 = 2,
        BTN5 = 1,
        BTN6 = 0,
        BTN7 = 6,
        BTN8 = 5,
        SLIDER = 9,
        NUM_LEDS
    };
};