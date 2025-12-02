#pragma once
#include "dialogUI.hpp"
#include "game.hpp"
#include "introScreen.hpp"
#include "introTitle.hpp"
#include "mapSelectionUI.hpp"
#include "quizUI.hpp"
#include "weaponSelectionUI.hpp"

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
            drawWeaponSelectionUI(game, game.window);
            break;
        case GameState::Quiz:
            drawQuizUI(game, game.window);
            break;
    }
}