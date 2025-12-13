#include "uiEffects.hpp"
#include <algorithm>
#include <cmath>
#include "helper/colorHelper.hpp"
#include "ui/nineSliceBox.hpp"

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

    sf::Color computeGlowColor(
        const sf::Color& baseColor,
        float elapsedSeconds,
        float uiAlphaFactor,
        float baseAlpha,
        float amplitude,
        std::initializer_list<float> frequencies,
        float phase
    )
    {
        float glowAlpha = computeFlickerAlpha(
            elapsedSeconds,
            baseAlpha,
            amplitude,
            frequencies,
            phase
        );

        sf::Color glowColor = baseColor;
        glowColor.a = static_cast<uint8_t>(glowAlpha);
        glowColor.a = static_cast<uint8_t>(static_cast<float>(glowColor.a) * uiAlphaFactor);

        return glowColor;
    }

    void drawGlowFrame(
        sf::RenderTarget& target,
        const NineSliceBox& frame,
        sf::Vector2f position,
        sf::Vector2f size,
        const sf::Color& glowColor,
        float scaleOffset
    )
    {
        frame.drawScaled(target, position, size, glowColor, scaleOffset);
    }

    void drawGlowFrame(
        sf::RenderTarget& target,
        const NineSliceBox& frame,
        sf::Vector2f position,
        sf::Vector2f size,
        const sf::Color baseColor,
        float elapsedSeconds,
        float uiAlphaFactor,
        float baseAlpha,
        float amplitude,
        std::initializer_list<float> frequencies,
        float phase,
        float scaleOffset
    )
    {
        auto glowColor = computeGlowColor(
            baseColor,
            elapsedSeconds,
            uiAlphaFactor,
            baseAlpha,
            amplitude,
            frequencies,
            phase
        );

        drawGlowFrame(target, frame, position, size, glowColor, scaleOffset);
    }
}