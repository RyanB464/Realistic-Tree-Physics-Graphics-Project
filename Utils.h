#pragma once
#include <random>

namespace Utils {
    std::mt19937& rng();
    float angleJitter();
    float lengthJitter();
}
