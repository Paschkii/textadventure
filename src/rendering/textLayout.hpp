#pragma once
// === C++ Libraries ===
#include <vector>                 // Stores the list of ColoredTextSegment inputs to draw.
// === SFML Libraries ===
#include <SFML/Graphics.hpp>      // Provides the font, RenderTarget, Text, and Color helpers for drawing.
// === Header Files ===
#include "helper/textColorHelper.hpp"  // Defines ColoredTextSegment used by drawColoredSegments.

// Draws colored text segments with wrapping/alpha handling and returns the new cursor.
sf::Vector2f drawColoredSegments(
    sf::RenderTarget& target,
    const sf::Font& font,
    const std::vector<ColoredTextSegment>& segments,
    sf::Vector2f startPos,
    unsigned int characterSize,
    float maxWidth,
    float alphaFactor = 1.f);
