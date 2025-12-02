#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <utility>
#include <vector>

struct ColoredTextSegment {
    std::string text;
    sf::Color color = sf::Color::White;
};

std::vector<ColoredTextSegment> buildColoredSegments(const std::string& text);

sf::Vector2f drawColoredSegments(
    sf::RenderTarget& target,
    const sf::Font& font,
    const std::vector<ColoredTextSegment>& segments,
    sf::Vector2f startPos,
    unsigned int characterSize,
    float maxWidth,
    float alphaFactor = 1.f);