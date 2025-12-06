#include "uiEffects.hpp"
#include <algorithm>
#include <cmath>

namespace uiEffects {
    float computeFlickerAlpha(
        float elapsedSeconds,
        float baseAlpha,
        float amplitude,
        std::initializer_list<float> frequencies,
        float phase
    )
    {
        if (frequencies.size() == 0)
            return std::clamp(baseAlpha, 0.f, 255.f);

        float combined = 0.f;
        for (float frequency : frequencies) {
            combined += std::sin((elapsedSeconds * frequency) + phase);
        }
        combined /= static_cast<float>(frequencies.size());

        float alpha = baseAlpha + (combined * amplitude);
        return std::clamp(alpha, 0.f, 255.f);
    }
}