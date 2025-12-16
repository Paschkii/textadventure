#pragma once

// === SFML Libraries ===
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

// === Header Files ===
#include "core/game.hpp"

namespace ui::genderSelection {

bool handleEvent(Game& game, const sf::Event& event);
void draw(Game& game, sf::RenderTarget& target, float uiAlphaFactor);
void start(Game& game);
void stop(Game& game);

} // namespace ui::genderSelection
