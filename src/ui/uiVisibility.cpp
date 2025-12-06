#include "uiVisibility.hpp"
#include "../core/game.hpp"
#include <algorithm>

UiVisibility computeUiVisibility(Game& game, UiElementMask elements) {
    UiVisibility visibility{};

    if (elements == static_cast<UiElementMask>(UiElement::None))
        return visibility;

    if (game.uiFadeInActive) {
        float fadeProgress = std::min(1.f, game.uiFadeClock.getElapsedTime().asSeconds() / game.uiFadeInDuration);
        visibility.alphaFactor = fadeProgress;

        if (fadeProgress >= 1.f) {
                game.uiFadeInActive = false;

                if (game.startGonadDialoguePending) {
                    game.startGonadDialoguePending = false;
                    game.currentDialogue = &gonad;
                    game.dialogueIndex = 0;
                    game.visibleText.clear();
                    game.charIndex = 0;
                    game.typewriterClock.restart();
                    game.introDialogueFinished = false;
                    game.state = GameState::Dialogue;
                    game.currentLocation = Locations::findById(game.locations, LocationId::Gonad);
                }
        }
    }
    else if (game.introDialogueFinished) {
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