#pragma once
// === C++ Libraries ===
#include <string>            // Stores source text and segment substrings for coloring.
#include <vector>            // Returns dynamic lists of ColoredTextSegment results.
// === SFML Libraries ===
#include <SFML/Graphics.hpp>  // Provides sf::Color used for each segment.
// === Header Files ===
#include "helper/colorHelper.hpp"  // Supplies default palette colors for fallback segments.

// Describes text and the color it should be rendered in for a segment.
struct ColoredTextSegment {
    std::string text;
    sf::Color color = ColorHelper::Palette::Normal;
};

// Splits a string into colored segments based on tokens/names that should change color.
std::vector<ColoredTextSegment> buildColoredSegments(const std::string& text);
