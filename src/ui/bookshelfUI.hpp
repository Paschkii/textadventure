#pragma once
// === SFML Libraries ===
#include <SFML/Graphics.hpp>

// === Header Files ===
#include "core/game.hpp"

namespace ui::bookshelf {
    // Prepares a new seating of books and shows the quest state overlay.
    void enter(Game& game);
    // Handles input events received while the bookshelf quest is active.
    bool handleEvent(Game& game, const sf::Event& event);
    // Draws the bookshelf, the books, and the status text.
    void draw(Game& game, sf::RenderTarget& target);
} // namespace ui::bookshelf
