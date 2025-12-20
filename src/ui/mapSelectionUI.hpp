#pragma once
// === C++ Libraries ===
#include <optional>  // Returns LocationId selections from keyboard/mouse helpers.
#include <string>    // Stores text shown inside popups describing each location.
// === SFML Libraries ===
#include <SFML/Graphics/RenderTarget.hpp>  // Draws the map and popup overlays.
#include <SFML/Graphics/Rect.hpp>             // Represents map and icon bounds.
#include <SFML/Window/Event.hpp>              // Processes map-specific mouse and keyboard events.

struct Game;

struct MapPopupRenderData {
    std::string title;
    std::string shortDesc;
    std::string residentTitle;
    std::string residentDesc;
    float posX = 0.f;
    float posY = 0.f;
    sf::FloatRect iconBounds{};
    sf::FloatRect mapBounds{};
    float winW = 0.f;
    float winH = 0.f;
    sf::Vector2f cursorPos{0.f, 0.f};
};

std::optional<MapPopupRenderData> drawMapSelectionUI(Game& game, sf::RenderTarget& target);
void drawMapSelectionPopup(Game& game, sf::RenderTarget& target, const MapPopupRenderData& popup);
void handleMapSelectionEvent(Game& game, const sf::Event& event, const sf::View* viewOverride = nullptr);
