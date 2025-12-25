#pragma once
// === SFML Libraries ===
#include <SFML/Graphics/RenderTarget.hpp>  // Draws the menu button and overlay when active.
#include <SFML/Window/Event.hpp>            // Handles input when the menu is open or toggled.

class Game;

namespace ui::menu {
// Processes SFML events for the menu button/overlay and prevents underlying UI interaction when active.
bool handleEvent(Game& game, const sf::Event& event);

// Draws the menu button overlay and the modal menu when open.
void draw(Game& game, sf::RenderTarget& target);

} // namespace ui::menu
