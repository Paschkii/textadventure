// === C++ Libraries ===
#include <algorithm>  // Uses std::max when sizing the popup and button layout.
// === SFML Libraries ===
#include <SFML/Window/Mouse.hpp>     // Reads mouse position when updating hover/click state.
#include <SFML/Window/Keyboard.hpp>  // Handles key scans for confirm/cancel shortcuts.
// === Header Files ===
#include "../core/game.hpp"          // Pulls in Game/UI state mutated by the prompt helpers.
#include "confirmationUI.hpp"        // Declares show/hide/draw/handle functions implemented here.
#include "helper/colorHelper.hpp"    // Applies shared palette colors to the popup visuals.
#include "story/textStyles.hpp"      // Uses TextStyles::UI colors when rendering text/button outlines.
#include "helper/textColorHelper.hpp"// Breaks the message into colored segments.
#include "rendering/textLayout.hpp"  // Reuses the tutorial line-spacing helper.
#include "ui/popupStyle.hpp"         // Draws the standardized popup frame.

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
        // placeholder; final position is computed in drawConfirmationPrompt using popup layout
        return {};
    }

    void drawButton(
        Game& game,
        sf::RenderTarget& target,
        const sf::FloatRect& bounds,
        const std::string& label,
        bool isPrimary,
        bool isActive,
        float uiAlphaFactor
    ) {
        sf::Color baseColor = TextStyles::UI::PanelDark;
        sf::Color outlineColor = ColorHelper::Palette::Amber;

        if (isActive)
            baseColor = isPrimary ? ColorHelper::Palette::Green : ColorHelper::Palette::SoftRed;

        baseColor = ColorHelper::applyAlphaFactor(baseColor, uiAlphaFactor * (isActive ? 0.9f : 0.7f));
        outlineColor = ColorHelper::applyAlphaFactor(outlineColor, uiAlphaFactor);

        RoundedRectangleShape buttonShape({ bounds.size.x, bounds.size.y }, bounds.size.y * 0.5f, 20);
        buttonShape.setPosition({ bounds.position.x, bounds.position.y });
        buttonShape.setFillColor(baseColor);
        buttonShape.setOutlineThickness(2.f);
        buttonShape.setOutlineColor(outlineColor);
        target.draw(buttonShape);

        sf::Text text{ game.resources.uiFont, label, kButtonTextSize };
        text.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, uiAlphaFactor));
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
    game.confirmationPrompt.keyboardSelection = -1;
}

void drawConfirmationPrompt(Game& game, sf::RenderTarget& target, float uiAlphaFactor) {
    if (!game.confirmationPrompt.active)
        return;

    float popupPadding = 14.f;
    float popupWidth = 520.f;
    float popupHeight = (kButtonHeight * 2.f) + (popupPadding * 2.f);
    float winW = static_cast<float>(game.window.getSize().x);
    float winH = static_cast<float>(game.window.getSize().y);
    float margin = winW * 0.05f;
    float popupX = winW - margin - popupWidth;
    float popupY = winH * 0.05f;

    sf::Text message{ game.resources.uiFont, game.confirmationPrompt.message, 20 };
    message.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, uiAlphaFactor));
    auto msgBounds = message.getLocalBounds();

    popupWidth = std::max(popupWidth, msgBounds.size.x + popupPadding * 2.f + kButtonWidth + popupPadding * 2.f);
    popupHeight = std::max(popupHeight, msgBounds.size.y + popupPadding * 2.f);
    popupX = std::max(8.f, popupX);
    popupY = std::max(8.f, popupY);

    sf::FloatRect popupBounds{ { popupX, popupY }, { popupWidth, popupHeight } };
    ui::popup::drawPopupFrame(target, popupBounds, uiAlphaFactor);

    float msgX = popupX + popupPadding;
    float msgY = popupY + popupPadding;
    float buttonsX = popupX + popupWidth - popupPadding - kButtonWidth;
    float textEndX = buttonsX - popupPadding * 0.5f;
    float maxTextWidth = std::max(0.f, textEndX - msgX);
    auto segments = buildColoredSegments(game.confirmationPrompt.message);
    drawColoredSegments(
        target,
        game.resources.uiFont,
        segments,
        { msgX, msgY },
        20,
        maxTextWidth,
        uiAlphaFactor,
        ui::popup::kLineSpacingMultiplier
    );

    // buttons on right stacked vertically
    float yesY = popupY + popupPadding;
    float noY = yesY + kButtonHeight + 8.f;
    game.confirmationPrompt.yesBounds = { { buttonsX, yesY }, { kButtonWidth, kButtonHeight } };
    game.confirmationPrompt.noBounds = { { buttonsX, noY }, { kButtonWidth, kButtonHeight } };

    auto mousePos = game.window.mapPixelToCoords(sf::Mouse::getPosition(game.window));
    bool yesHovered = game.confirmationPrompt.yesBounds.contains(mousePos);
    bool noHovered = game.confirmationPrompt.noBounds.contains(mousePos);

    bool yesActive = yesHovered || game.confirmationPrompt.keyboardSelection == 0;
    bool noActive = noHovered || game.confirmationPrompt.keyboardSelection == 1;

    drawButton(game, target, game.confirmationPrompt.yesBounds, "Yes", true, yesActive, uiAlphaFactor);
    drawButton(game, target, game.confirmationPrompt.noBounds, "No", false, noActive, uiAlphaFactor);
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

    if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        if (key->scancode == sf::Keyboard::Scan::Up) {
            game.confirmationPrompt.keyboardSelection = 0;
            return true;
        }
        if (key->scancode == sf::Keyboard::Scan::Down) {
            game.confirmationPrompt.keyboardSelection = 1;
            return true;
        }
        if (key->scancode == sf::Keyboard::Scan::Enter || key->scancode == sf::Keyboard::Scan::Enter) {
            if (game.confirmationPrompt.keyboardSelection == 0) {
                if (game.confirmSound) {
                    game.confirmSound->stop();
                    game.confirmSound->play();
                }
                hideConfirmationPrompt(game);
                if (game.confirmationPrompt.onConfirm)
                    game.confirmationPrompt.onConfirm(game);
                return true;
            }
            if (game.confirmationPrompt.keyboardSelection == 1) {
                if (game.rejectSound) {
                    game.rejectSound->stop();
                    game.rejectSound->play();
                }
                hideConfirmationPrompt(game);
                if (game.confirmationPrompt.onCancel)
                    game.confirmationPrompt.onCancel(game);
                return true;
            }
        }
    }

    return false;
}
