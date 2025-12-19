#pragma once

#include <SFML/Graphics/RenderTarget.hpp>  // Draws the preview popup during dialogue.

struct Game;

namespace ui {
namespace brokenweapon {
void showPreview(Game& game);
void hidePreview(Game& game);
void updatePreview(Game& game);
void drawPreview(Game& game, sf::RenderTarget& target);
}
}
