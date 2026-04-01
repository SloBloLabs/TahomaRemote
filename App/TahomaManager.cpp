#include "TahomaManager.h"
#include <cstdlib>

std::span<Shutter*> TahomaManager::getChangedShutters() {
    static std::array<Shutter*, MAX_SHUTTER_SIZE> result; // static or reuse buffer
    size_t count = 0;

    for(auto& shutter : _shutters) {
        if(shutter.isChanged()) {
            result[count++] = &shutter;
        }
    }
    return std::span<Shutter*>(result.data(), count);
}