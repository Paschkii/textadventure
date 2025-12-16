#pragma once
// === C++ Libraries ===
#include <algorithm>  // Uses std::clamp when modulating overlay alpha values.
#include <cstdint>    // Reads/writes std::uint8_t when computing overlay colors.
// === Header Files ===
#include "core/game.hpp"                   // Supplies Game state, resources, and controllers for rendering.
#include "helper/colorHelper.hpp"          // Supplies ColorHelper::Palette colors used while drawing text.
#include "ui/dialogUI.hpp"                 // Draws the dialogue UI during most GameState modes.
#include "ui/introScreen.hpp"              // Renders the intro screen when GameState is IntroScreen.
#include "ui/introTitle.hpp"               // Handles the intro title overlay used before dialogue.
#include "ui/mapSelectionUI.hpp"           // Displays the map UI and popup when in map selection.
#include "ui/quizUI.hpp"                   // Draws the quiz UI when GameState is Quiz.
#include "ui/rankingUI.hpp"                // Renders the leaderboard once the ending finishes.
#include "ui/weaponSelectionUI.hpp"        // Renders weapon selection graphics when active.

// Returns the current overlay opacity driven by the end-sequence controller.
inline float endOverlayAlpha(const Game& game) {
    return game.endSequenceController.overlayAlpha();
}

// Returns the current text opacity the end-screen should use based on its fade state.
inline float endTextAlpha(const Game& game) {
    return game.endSequenceController.textAlpha();
}

// Draws the "THE END" overlay once the ending fade has progressed enough.
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

// Returns the opacity of the teleport mask while the sequence runs.
inline float teleportOverlayAlpha(const Game& game) {
    return game.teleportController.overlayAlpha();
}

// Renders the fullscreen teleport fade using the overlay alpha.
inline void drawTeleportOverlay(Game& game) {
    float alphaFactor = teleportOverlayAlpha(game);
    if (alphaFactor <= 0.f)
        return;

    sf::Vector2u size = game.window.getSize();
    sf::RectangleShape overlay({ static_cast<float>(size.x), static_cast<float>(size.y) });
    overlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(255.f * std::clamp(alphaFactor, 0.f, 1.f))));
    game.window.draw(overlay);
}

// Chooses the correct UI screens based on the current GameState and adds overlays.
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
    const auto& entries = game.rankingManager.entries();
    int highlightIndex = -1;
    if (game.lastRecordedRank > 0 && game.lastRecordedRank <= static_cast<int>(entries.size()))
        highlightIndex = game.lastRecordedRank - 1;
    ui::ranking::drawOverlay(game.rankingOverlay, game.window, game.resources.uiFont, entries, highlightIndex, game.playerName);
}
