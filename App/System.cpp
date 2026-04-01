#include "System.h"
#include "main.h"

std::atomic<uint32_t> System::_ticks{0};

void System::init() {
    // Configure and enable Systick timer including interrupt
    SysTick_Config((SystemCoreClock / 1000) - 1);
}

void System::tick() {
    ++_ticks;
}

uint32_t System::ticks() {
    return _ticks.load();
}
