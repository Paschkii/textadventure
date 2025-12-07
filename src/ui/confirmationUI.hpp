#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <functional>
#include <string>
#include <SFML/Window/Event.hpp>

class Game;

struct ConfirmationPrompt {
    bool active = false;
    std::string message;
    sf::FloatRect yesBounds{};
    sf::FloatRect noBounds{};
    std::function<void(Game&)> onConfirm;
    std::function<void(Game&)> onCancel;
};

void showConfirmationPrompt(
    Game& game,
    const std::string& message,
    std::function<void(Game&)> onConfirm,
    std::function<void(Game&)> onCancel
);

void hideConfirmationPrompt(Game& game);

void drawConfirmationPrompt(Game& game, sf::RenderTarget& target, float uiAlphaFactor);

bool handleConfirmationEvent(Game& game, const sf::Event& event);