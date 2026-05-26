#include <cmath>
#include <iostream>
#include "constants.h"

float abs(float x) {
    return x < 0 ? -x : x;
}

float exp(float x) { // TODO: minimax for better performance
    int k = static_cast<int>(x * INV_LN2 + (x >= 0? 0.5f : -0.5f));
    float r = x - k * LN2;
    float result = 1 + r * (1 + r * (0.5f + r * (1.0f/6 + r * (1.0f/24 + r * (1.0f/120 + r * 1.0f/720))))); // Taylor expansion

    return std::ldexp(result, k); // result * 2^k
}

float sqrt(float a) {
    if (a < 0) {
        std::cerr << "Cannot take square root of a negative number\n";
        std::exit(1);
    }
    
    float x = a * 0.5f;
    for (int i = 0; i < 10; i++)
        x = 0.5f * (x + a / x);
    return x;
}