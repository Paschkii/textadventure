#pragma once
#include "ui/dialogUI.hpp"
#include "core/game.hpp"
#include "ui/introScreen.hpp"
#include "ui/introTitle.hpp"
#include "ui/mapSelectionUI.hpp"
#include "ui/quizUI.hpp"
#include "ui/weaponSelectionUI.hpp"
#include "helper/colorHelper.hpp"
#include <algorithm>
#include <cstdint>

inline float endOverlayAlpha(const Game& game) {
    if (!game.endSequenceActive && !game.endScreenVisible)
        return 0.f;

    if (game.endFadeOutActive) {
        return std::min(1.f, game.endClock.getElapsedTime().asSeconds() / game.endFadeOutDuration);
    }
    return 1.f;
}

inline float endTextAlpha(const Game& game) {
    if (game.endFadeInActive) {
        return std::min(1.f, game.endClock.getElapsedTime().asSeconds() / game.endFadeInDuration);
    }
    if (game.endScreenVisible)
        return 1.f;
    return 0.f;
}

inline void drawEndScreen(Game& game) {
    float overlayA = endOverlayAlpha(game);
    if (overlayA <= 0.f)
        return;

    sf::Vector2u size = game.window.getSize();
    sf::RectangleShape overlay({ static_cast<float>(size.x), static_cast<float>(size.y) });
    overlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(255.f * std::clamp(overlayA, 0.f, 1.f))));
    game.window.draw(overlay);

    float textAlpha = endTextAlpha(game);
    if (textAlpha <= 0.f)
        return;

    std::string text = "THE END";
    sf::Text back{ game.resources.titleFontExtrude, text, 120 };
    sf::Text front{ game.resources.titleFont, text, 120 };
    back.setFillColor(ColorHelper::Palette::TitleBack);
    sf::Color frontColor = ColorHelper::Palette::TitleAccent;
    frontColor.a = static_cast<std::uint8_t>(frontColor.a * textAlpha);
    back.setFillColor(ColorHelper::applyAlphaFactor(back.getFillColor(), textAlpha));
    front.setFillColor(frontColor);

    auto centerOrigin = [](sf::Text& t) {
        auto b = t.getLocalBounds();
        t.setOrigin({ b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f });
    };
    centerOrigin(back);
    centerOrigin(front);

    float cx = static_cast<float>(size.x) * 0.5f;
    float cy = static_cast<float>(size.y) * 0.5f;
    back.setPosition({ cx, cy });
    front.setPosition({ cx, cy });

    game.window.draw(back);
    game.window.draw(front);
}

inline float teleportOverlayAlpha(const Game& game) {
    switch (game.teleportPhase) {
        case Game::TeleportPhase::FadeOut:
            return std::min(1.f, game.teleportClock.getElapsedTime().asSeconds() / game.teleportFadeOutDuration);
        case Game::TeleportPhase::Cooldown:
            return 1.f;
        case Game::TeleportPhase::FadeIn:
            return 1.f - std::min(1.f, game.teleportClock.getElapsedTime().asSeconds() / game.teleportFadeInDuration);
        case Game::TeleportPhase::None:
        default:
            return 0.f;
    }
}

inline void drawTeleportOverlay(Game& game) {
    float alphaFactor = teleportOverlayAlpha(game);
    if (alphaFactor <= 0.f)
        return;

    sf::Vector2u size = game.window.getSize();
    sf::RectangleShape overlay({ static_cast<float>(size.x), static_cast<float>(size.y) });
    overlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(255.f * std::clamp(alphaFactor, 0.f, 1.f))));
    game.window.draw(overlay);
}

inline void renderGame(Game& game) {
    switch (game.state) {
        case GameState::IntroScreen:
            renderIntroScreen(game);
            break;
        case GameState::Dialogue:
            drawIntroTitle(game, game.window);
            drawDialogueUI(game, game.window);
            break;
        case GameState::IntroTitle:
            drawIntroTitle(game, game.window);
            drawDialogueUI(game, game.window);
            break;
        case GameState::MapSelection: {
            // Keep the location/name/text boxes visible while showing the map
            float mapUiAlpha = 1.f;
            auto popup = drawMapSelectionUI(game, game.window);
            drawDialogueUI(game, game.window, true, &mapUiAlpha);
            if (popup)
                drawMapSelectionPopup(game, game.window, *popup);
            if (game.confirmationPrompt.active)
                drawConfirmationPrompt(game, game.window, mapUiAlpha);
            break;
        }
        case GameState::WeaponSelection:
            drawWeaponSelectionUI(game, game.window);
            // Draw dialogue UI afterward so confirmation popups sit above weapon sprites.
            drawDialogueUI(game, game.window);
            break;
        case GameState::Quiz:
            drawDialogueUI(game, game.window);
            drawQuizUI(game, game.window);
            break;
        case GameState::FinalChoice:
            drawDialogueUI(game, game.window);
            drawFinalChoiceUI(game, game.window);
            break;
    }

    drawTeleportOverlay(game);
    drawEndScreen(game);
}
