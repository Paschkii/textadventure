#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

struct Game;

void drawWeaponSelectionUI(Game& game, sf::RenderTarget& target);
void handleWeaponSelectionEvent(Game& game, const sf::Event& event);