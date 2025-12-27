#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

class Game;

namespace ui::battle {

void update(Game& game, sf::Time dt);
bool handleEvent(Game& game, const sf::Event& event);
void draw(Game& game, sf::RenderTarget& target);

} // namespace ui::battle
