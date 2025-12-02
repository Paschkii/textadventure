#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "textColorHelper.hpp"

sf::Vector2f drawColoredSegments(
    sf::RenderTarget& target,
    const sf::Font& font,
    const std::vector<ColoredTextSegment>& segments,
    sf::Vector2f startPos,
    unsigned int characterSize,
    float maxWidth,
    float alphaFactor = 1.f);