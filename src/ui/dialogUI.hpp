#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include "core/game.hpp"

class Game;

void drawDialogueUI(Game& game, sf::RenderTarget& target);

void drawLocationBox(Game& game, sf::RenderTarget& target);