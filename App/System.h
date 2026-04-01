#pragma once

#include <atomic>
#include <cstdint>

class System {
public:
    static void init();
    static uint32_t ticks();
    static void tick();

private:
    static std::atomic<uint32_t> _ticks;
};