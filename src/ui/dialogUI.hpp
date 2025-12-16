#pragma once
// === SFML Libraries ===
#include <SFML/Graphics/Color.hpp>  // Provides the glow color passed to the location header helper.
#include <SFML/Graphics/RenderTarget.hpp>  // Provides sf::RenderTarget parameter used by every draw function.
// === Header Files ===
#include "core/game.hpp"  // Accesses Game for dialogue state, boxes, and resources rendered by the UI.

class Game;

void drawDialogueUI(Game& game, sf::RenderTarget& target, bool skipConfirmation = false, float* outAlpha = nullptr);

void drawLocationBox(Game& game, sf::RenderTarget& target, float uiAlphaFactor, const sf::Color& glowColor);
