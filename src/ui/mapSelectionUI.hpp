#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Event.hpp>
#include <optional>
#include <string>

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
};

std::optional<MapPopupRenderData> drawMapSelectionUI(Game& game, sf::RenderTarget& target);
void drawMapSelectionPopup(Game& game, sf::RenderTarget& target, const MapPopupRenderData& popup);
void handleMapSelectionEvent(Game& game, const sf::Event& event);
