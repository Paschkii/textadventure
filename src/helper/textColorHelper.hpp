#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "helper/colorHelper.hpp"

struct ColoredTextSegment {
    std::string text;
    sf::Color color = ColorHelper::Palette::Normal;
};

std::vector<ColoredTextSegment> buildColoredSegments(const std::string& text);
