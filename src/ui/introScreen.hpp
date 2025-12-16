#pragma once
// === C++ Libraries ===
#include <string>  // Holds the body/prompt strings passed to renderIntroScreen.
// === SFML Libraries ===
#include <SFML/Graphics.hpp>  // Provides sf::Text and related drawables used when rendering.

class Game;

void renderIntroScreen(Game& game);
