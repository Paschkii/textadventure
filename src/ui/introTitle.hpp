#pragma once
// === SFML Libraries ===
#include <SFML/Graphics/RenderTarget.hpp>  // Supplies the target used when drawing the title overlay.
#include <SFML/Window/Event.hpp>            // Needed for intro title menu input handling.

class Game;

void triggerIntroTitleExit(Game & game);
bool handleIntroTitleEvent(Game& game, const sf::Event& event);
void drawIntroTitle(Game& game, sf::RenderTarget& target);
