#pragma once
// === C++ Libraries ===
#include <functional>  // Stores callbacks triggered when the prompt is confirmed or canceled.
#include <string>      // Holds the message text shown inside the modal.
// === SFML Libraries ===
#include <SFML/Graphics/RenderTarget.hpp>  // Draws the prompt box and text onto the window.
#include <SFML/Window/Event.hpp>           // Processes keyboard/mouse input for the prompt.
#include <SFML/System/Clock.hpp>          // Tracks blink/fade timing for the prompt’s UI effects.

class Game;

// Tracks the state of a yes/no modal and routes confirm/cancel callbacks.
struct ConfirmationPrompt {
    bool active = false;
    std::string message;
    sf::FloatRect yesBounds{};
    sf::FloatRect noBounds{};
    std::function<void(Game&)> onConfirm;
    std::function<void(Game&)> onCancel;
    int keyboardSelection = -1; // 0 = yes, 1 = no, -1 = none
};

// Shows the prompt with the given message and callbacks.
void showConfirmationPrompt(
    Game& game,
    const std::string& message,
    std::function<void(Game&)> onConfirm,
    std::function<void(Game&)> onCancel
);

// Hides the modal and clears any queued callbacks.
void hideConfirmationPrompt(Game& game);

// Renders the prompt box with fading so it sits above the regular UI.
void drawConfirmationPrompt(Game& game, sf::RenderTarget& target, float uiAlphaFactor);

// Processes keyboard/mouse input for the active prompt.
bool handleConfirmationEvent(Game& game, const sf::Event& event);
