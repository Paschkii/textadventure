#include "confirmationUI.hpp"
#include "../core/game.hpp"
#include "rendering/colorHelper.hpp"
#include "story/textStyles.hpp"
#include <SFML/Window/Mouse.hpp>

namespace {
    constexpr float kButtonHeight = 44.f;
    constexpr float kButtonWidth = 140.f;
    constexpr float kButtonPadding = 18.f;
    constexpr unsigned int kButtonTextSize = 22;

    sf::FloatRect computeYesBounds(const Game& game) {
        auto boxPos = game.textBox.getPosition();
        auto boxSize = game.textBox.getSize();
        float y = boxPos.y + boxSize.y - kButtonHeight - kButtonPadding;
        float x = boxPos.x + kButtonPadding;
        return { { x, y }, { kButtonWidth, kButtonHeight } };
    }

    sf::FloatRect computeNoBounds(const Game& game) {
        auto boxPos = game.textBox.getPosition();
        auto boxSize = game.textBox.getSize();
        float y = boxPos.y + boxSize.y - kButtonHeight - kButtonPadding;
        float x = boxPos.x + boxSize.x - kButtonPadding - kButtonWidth;
        return { { x, y }, { kButtonWidth, kButtonHeight } };
    }

    void drawButton(
        Game& game,
        sf::RenderTarget& target,
        const sf::FloatRect& bounds,
        const std::string& label,
        bool isPrimary,
        bool isHovered,
        float uiAlphaFactor
    ) {
        sf::Color baseColor = TextStyles::UI::PanelDark;
        sf::Color outlineColor = ColorHelper::Palette::Amber;

        if (isHovered)
            baseColor = isPrimary ? ColorHelper::Palette::Green : sf::Color::Red;

        baseColor = ColorHelper::applyAlphaFactor(baseColor, uiAlphaFactor * (isHovered ? 0.9f : 0.7f));
        outlineColor = ColorHelper::applyAlphaFactor(outlineColor, uiAlphaFactor);

        sf::RectangleShape buttonShape({ bounds.size.x, bounds.size.y });
        buttonShape.setPosition({ bounds.position.x, bounds.position.y });
        buttonShape.setFillColor(baseColor);
        buttonShape.setOutlineThickness(2.f);
        buttonShape.setOutlineColor(outlineColor);
        target.draw(buttonShape);

        sf::Text text{ game.resources.uiFont, label, kButtonTextSize };
        text.setFillColor(ColorHelper::applyAlphaFactor(sf::Color::White, uiAlphaFactor));
        auto textBounds = text.getLocalBounds();
        text.setOrigin({ textBounds.position.x + (textBounds.size.x / 2.f), textBounds.position.y + (textBounds.size.y / 2.f) });
        text.setPosition({ bounds.position.x + (bounds.size.x / 2.f), bounds.position.y + (bounds.size.y / 2.f) });
        target.draw(text);
    }
}

void showConfirmationPrompt(
    Game& game,
    const std::string& message,
    std::function<void(Game&)> onConfirm,
    std::function<void(Game&)> onCancel
) {
    game.confirmationPrompt.active = true;
    game.confirmationPrompt.message = message;
    game.confirmationPrompt.onConfirm = std::move(onConfirm);
    game.confirmationPrompt.onCancel = std::move(onCancel);
    game.visibleText = message;
    game.charIndex = message.size();
}

void hideConfirmationPrompt(Game& game) {
    game.confirmationPrompt.active = false;
    game.confirmationPrompt.message.clear();
    game.confirmationPrompt.yesBounds = {};
    game.confirmationPrompt.noBounds = {};
}

void drawConfirmationPrompt(Game& game, sf::RenderTarget& target, float uiAlphaFactor) {
    if (!game.confirmationPrompt.active)
        return;

    game.confirmationPrompt.yesBounds = computeYesBounds(game);
    game.confirmationPrompt.noBounds = computeNoBounds(game);

    auto mousePos = game.window.mapPixelToCoords(sf::Mouse::getPosition(game.window));
    bool yesHovered = game.confirmationPrompt.yesBounds.contains(mousePos);
    bool noHovered = game.confirmationPrompt.noBounds.contains(mousePos);

    drawButton(game, target, game.confirmationPrompt.yesBounds, "Yes", true, yesHovered, uiAlphaFactor);
    drawButton(game, target, game.confirmationPrompt.noBounds, "No", false, noHovered, uiAlphaFactor);
}

bool handleConfirmationEvent(Game& game, const sf::Event& event) {
    if (!game.confirmationPrompt.active)
        return false;

    if (event.is<sf::Event::MouseButtonReleased>()) {
        const auto* button = event.getIf<sf::Event::MouseButtonReleased>();
        if (button->button != sf::Mouse::Button::Left)
            return true;

        sf::Vector2f clickPos = game.window.mapPixelToCoords(button->position);
        if (game.confirmationPrompt.yesBounds.contains(clickPos)) {
            if (game.confirmSound) {
                game.confirmSound->stop();
                game.confirmSound->play();
            }
            hideConfirmationPrompt(game);
            if (game.confirmationPrompt.onConfirm)
                game.confirmationPrompt.onConfirm(game);
        }
        else if (game.confirmationPrompt.noBounds.contains(clickPos)) {
            if (game.rejectSound) {
                game.rejectSound->stop();
                game.rejectSound->play();
            }
            hideConfirmationPrompt(game);
            if (game.confirmationPrompt.onCancel)
                game.confirmationPrompt.onCancel(game);
        }
        return true;
    }

    // Confirmation UI should only be controllable via mouse clicks.

    return false;
}