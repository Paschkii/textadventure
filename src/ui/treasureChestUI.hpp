#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

#include "rendering/locations.hpp"

struct Game;

namespace ui {
namespace treasureChest {

bool handleEvent(Game& game, const sf::Event& event);
void update(Game& game, float deltaSeconds);
void draw(Game& game, sf::RenderTarget& target);
void prepare(Game& game, LocationId location);

} // namespace treasureChest
} // namespace ui
