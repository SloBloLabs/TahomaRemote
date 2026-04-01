#pragma once

#include <string_view>
#include <cstdint>
#include <cmath>

inline void stripSlash(char* out, std::string_view in) {
    char c;
    size_t i = 0;
    for(size_t j = 0; j < in.length(); ++j) {
        c = in.at(j);
        if(c != '\\') {
            out[i++] = c;
        }
    }
    out[i] = 0;
}

template<typename T>
static T quantize(T value, float inputRange, float outputRange) {
    float delta = inputRange / outputRange;
    float k = floorf(value / delta);
    //T qValue = k * delta;
    //return qValue;
    return k;
}

inline void HSVtoRGB(float H, float S, float V, float &R, float &G, float &B) {
    if(H > 360 || H < 0 || S > 1 || S < 0 || V > 1 || V < 0) {
        return;
    }
    
    float C = V * S;
    float X = C * (1 - fabsf(fmodf(H / 60.f, 2.f) - 1));
    float m = V - C;
    float r, g, b;

    if(H >= 0 && H < 60) {
        r = C, g = X, b = 0;
    } else if(H >= 60 && H < 120) {
        r = X, g = C, b = 0;
    } else if(H >= 120 && H < 180) {
        r = 0, g = C, b = X;
    } else if(H >= 180 && H < 240) {
        r = 0, g = X, b = C;
    } else if(H >= 240 && H < 300) {
        r = X, g = 0, b = C;
    } else {
        r = C, g = 0, b = X;
    }
    
    R = (r + m);
    G = (g + m);
    B = (b + m);
}