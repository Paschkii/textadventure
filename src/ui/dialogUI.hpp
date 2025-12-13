#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include "core/game.hpp"

class Game;

void drawDialogueUI(Game& game, sf::RenderTarget& target, bool skipConfirmation = false, float* outAlpha = nullptr);

void drawLocationBox(Game& game, sf::RenderTarget& target, float uiAlphaFactor);
