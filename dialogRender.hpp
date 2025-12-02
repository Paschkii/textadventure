#pragma once
#include "dialogUI.hpp"
#include "game.hpp"
#include "introScreen.hpp"
#include "introTitle.hpp"

inline void renderDialogue(Game& game) {
    if (game.showingIntroScreen) {
        renderIntroScreen(game);
        return;
    }

    drawIntroTitle(game, game.window);
    drawDialogueUI(game, game.window);