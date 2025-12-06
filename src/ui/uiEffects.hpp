#pragma once
#include <initializer_list>
#include <SFML/Graphics.hpp>

class NineSliceBox;

namespace uiEffects {
    float computeFlickerAlpha(
        float elapsedSeconds,
        float baseAlpha,
        float amplitude,
        std::initializer_list<float> frequencies,
        float phase = 0.f
    );

    sf::Color computeGlowColor(
        const sf::Color& baseColor,
        float elapsedSeconds,
        float uiAlphaFactor,
        float baseAlpha,
        float amplitude,
        std::initializer_list<float> frequencies,
        float phase = 0.f
    );

    void drawGlowFrame(
        sf::RenderTarget& target,
        const NineSliceBox& frame,
        sf::Vector2f position,
        sf::Vector2f size,
        const sf::Color& glowColor,
        float scaleOffset = 2.f
    );

    void drawGlowFrame(
        sf::RenderTarget& target,
        const NineSliceBox& frame,
        sf::Vector2f position,
        sf::Vector2f size,
        const sf::Color& baseColor,
        float elapsedSeconds,
        float uiAlphaFactor,
        float baseAlpha,
        float amplitude,
        std::initializer_list<float> frequencies,
        float phase = 0.f,
        float scaleOffset = 2.f
    );


}