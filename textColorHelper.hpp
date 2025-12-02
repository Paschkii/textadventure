#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

struct ColoredTextSegment {
    std::string text;
    sf::Color color = sf::Color::White;
};

std::vector<ColoredTextSegment> buildColoredSegments(const std::string& text);