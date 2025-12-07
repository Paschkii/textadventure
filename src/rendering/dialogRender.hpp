#pragma once
#include "ui/dialogUI.hpp"
#include "core/game.hpp"
#include "ui/introScreen.hpp"
#include "ui/introTitle.hpp"
#include "ui/mapSelectionUI.hpp"
#include "ui/quizUI.hpp"
#include "ui/weaponSelectionUI.hpp"

inline void renderGame(Game& game) {
    switch (game.state) {
        case GameState::IntroScreen:
            renderIntroScreen(game);
            break;
        case GameState::IntroTitle:
            drawIntroTitle(game, game.window);
            drawDialogueUI(game, game.window);
            break;
        case GameState::Dialogue:
            drawIntroTitle(game, game.window);
            drawDialogueUI(game, game.window);
            break;
        case GameState::MapSelection:
            drawMapSelectionUI(game, game.window);
            break;
        case GameState::WeaponSelection:
            drawDialogueUI(game, game.window);
            drawWeaponSelectionUI(game, game.window);
            break;
        case GameState::Quiz:
            drawQuizUI(game, game.window);
            break;
    }
}