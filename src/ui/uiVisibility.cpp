#include "uiVisibility.hpp"
#include "../core/game.hpp"
#include <algorithm>

UiVisibility computeUiVisibility(Game& game, UiElementMask elements) {
    UiVisibility visibility{};

    if (elements == static_cast<UiElementMask>(UiElement::None))
        return visibility;

    if (game.introDialogueFinished) {
        if (game.uiFadeOutActive) {
            float fadeProgress = std::min(1.f, game.uiFadeClock.getElapsedTime().asSeconds() / game.uiFadeOutDuration);
            visibility.alphaFactor = 1.f - fadeProgress;

            if (fadeProgress >= 1.f) {
                game.uiFadeOutActive = false;
                visibility.hidden = true;

                if (!game.backgroundFadeInActive && !game.backgroundVisible) {
                    game.backgroundFadeInActive = true;
                    game.backgroundFadeClock.restart();
                    visibility.backgroundFadeTriggered = true;
                }
            }
        }
        else {
            visibility.hidden = true;
        }
    }

    return visibility;
}