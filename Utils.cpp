#include "Utils.h"

namespace {
    std::mt19937 rngEngine(12345);
    std::uniform_real_distribution<float> angleDist(-25.0f, 25.0f);
    std::uniform_real_distribution<float> lengthDist(0.8f, 1.2f);
}

namespace Utils {
    std::mt19937& rng() {
        return rngEngine;
    }

    float angleJitter() {
        return angleDist(rngEngine);
    }

    float lengthJitter() {
        return lengthDist(rngEngine);
    }
}
