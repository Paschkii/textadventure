#pragma once
#include <SFML/Graphics/RenderTarget.hpp>

class Game;

bool introTitleDropComplete(const Game& game);
void triggerIntroTitleExit(Game & game);
void drawIntroTitle(Game& game, sf::RenderTarget& target);