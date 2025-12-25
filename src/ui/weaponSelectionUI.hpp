#pragma once
// === SFML Libraries ===
#include <SFML/Graphics/RenderTarget.hpp>  // Supplies the target for drawing weapons and overlays.
#include <SFML/Window/Event.hpp>           // Processes the mouse/keyboard events handled by the UI.

struct Game;

void drawWeaponSelectionUI(Game& game, sf::RenderTarget& target);
void handleWeaponSelectionEvent(Game& game, const sf::Event& event);
