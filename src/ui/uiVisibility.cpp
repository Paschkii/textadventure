// === C++ Libraries ===
#include <algorithm>  // Uses std::min when computing fade progress for UI visibility.
// === Header Files ===
#include "uiVisibility.hpp"  // Declares visibility helpers implemented in this translation unit.
#include "../core/game.hpp"  // Reads/updates Game fade/timer flags used by the visibility logic.

UiVisibility computeUiVisibility(Game& game, UiElementMask elements) {
    UiVisibility visibility{};

    if (elements == static_cast<UiElementMask>(UiElement::None))
        return visibility;

    if (game.endSequenceController.isActive() || game.endSequenceController.isScreenVisible()) {
        visibility.alphaFactor = 0.f;
        visibility.hidden = true;
        return visibility;
    }

    if (game.uiFadeInActive) {
        float fadeProgress = std::min(1.f, game.uiFadeClock.getElapsedTime().asSeconds() / game.uiFadeInDuration);
        visibility.alphaFactor = fadeProgress;

        if (fadeProgress >= 1.f) {
            game.uiFadeInActive = false;

            auto startIntroDialogue = [&]() {
                game.currentDialogue = &intro;
                game.dialogueIndex = 0;
                game.visibleText.clear();
                game.charIndex = 0;
                game.typewriterClock.restart();
                game.introDialogueFinished = false;
                game.state = GameState::Dialogue;
                game.currentProcessedLine.clear();
                game.askingName = false;
                game.nameInput.clear();
                game.audioManager.startIntroDialogueMusic();
            };

            auto startPerigonalDialogue = [&]() {
                game.currentDialogue = &perigonal;
                game.dialogueIndex = 0;
                game.visibleText.clear();
                game.charIndex = 0;
                game.typewriterClock.restart();
                game.introDialogueFinished = false;
                game.state = GameState::Dialogue;
                game.currentProcessedLine.clear();
                game.askingName = false;
                game.nameInput.clear();
                if (auto location = Locations::findById(game.locations, LocationId::Perigonal))
                    game.setCurrentLocation(location, false);
            };

            auto startGonadDialogue = [&]() {
                game.currentDialogue = &gonad;
                game.dialogueIndex = 0;
                game.visibleText.clear();
                game.charIndex = 0;
                game.typewriterClock.restart();
                game.introDialogueFinished = false;
                game.state = GameState::Dialogue;
                game.setCurrentLocation(Locations::findById(game.locations, LocationId::Gonad), false);
                game.currentProcessedLine.clear();
                game.askingName = false;
                game.nameInput.clear();
            };

            if (game.pendingIntroDialogue) {
                game.pendingIntroDialogue = false;
                startIntroDialogue();
            }
            else if (game.pendingPerigonalDialogue) {
                game.pendingPerigonalDialogue = false;
                startPerigonalDialogue();
            }
            else if (game.pendingGonadDialogue) {
                game.pendingGonadDialogue = false;
                startGonadDialogue();
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
                // Clear any lingering dialogue visuals (e.g., Tory Tailor portrait/name)
                // before the next dialogue fades back in.
                game.lastSpeaker.reset();
                game.visibleText.clear();
                game.currentProcessedLine.clear();

                if (game.queuedBackgroundTexture) {
                    game.setBackgroundTexture(*game.queuedBackgroundTexture);
                    game.queuedBackgroundTexture = nullptr;
                    visibility.backgroundFadeTriggered = true;
                }
                else if (!game.backgroundFadeInActive && !game.backgroundVisible) {
                    game.backgroundFadeInActive = true;
                    game.backgroundFadeClock.restart();
                    visibility.backgroundFadeTriggered = true;
                }
                if ((game.pendingPerigonalDialogue || game.pendingGonadDialogue) && !game.uiFadeInActive) {
                    game.introDialogueFinished = false;
                    game.uiFadeInActive = true;
                    game.uiFadeClock.restart();
                }
            }
        }
        else {
            visibility.hidden = true;
        }
    }

    return visibility;
}
