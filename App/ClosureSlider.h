#pragma once

#include <cstdint>
#include "utils.h"

class ClosureSlider {
public:
    void init();
    
    inline uint16_t value() {
        return quantize(_value, 4096.f, 101.f);
    }

private:
    volatile uint16_t _value;
};