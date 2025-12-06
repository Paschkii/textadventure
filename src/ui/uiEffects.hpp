#pragma once
#include <initializer_list>

namespace uiEffects {
    float computeFlickerAlpha(
        float elapsedSeconds,
        float baseAlpha,
        float amplitude,
        std::initializer_list<float> frequencies,
        float phase = 0.f
    );
}