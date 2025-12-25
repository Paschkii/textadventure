#pragma once
#include <algorithm>  // Used for removing icons and clamping values.
#include <cctype>
// === Header Files ===
#include "core/game.hpp"              // Accesses Game state manipulated while handling dialogue.
#include "core/itemActivation.hpp"
#include "dialogueLine.hpp"           // Uses DialogueLine metadata processed on Enter.
#include "textStyles.hpp"             // Formats speaker names and checks speaker IDs.
#include "ui/confirmationUI.hpp"      // Shows the name-confirmation modal.
#include "ui/quizUI.hpp"              // References quiz controls triggered mid-dialogue.
#include "ui/brokenWeaponPreview.hpp"  // Controls the broken weapon popup shown during Perigonal dialogue.
#include "helper/healingPotion.hpp"    // Starts the potion timer that restores health.
#include "mapTutorial.hpp"            // Defines Tory Tailor map tutorial steps.
#include "story/quests.hpp"           // Knows which quest should fire at each dialogue line.

// Collects the actions triggered by pressing Enter during dialogue or name entry.
struct EnterAction {
    bool confirmName = false;
    bool skipToEnd = false;
    bool nextLine = false;
};

namespace {
constexpr std::size_t kBrokenWeaponPreviewLineIndex = 32;
constexpr std::size_t kWillFigsidLineIndex = 33;
constexpr std::size_t kHealingPotionLineIndex = 4;
constexpr float kWeaponForgingFadeDuration = 0.8f;
constexpr float kWeaponForgingSleepDuration = 5.f;
}

inline constexpr std::size_t kBlacksmithSelectionLineIndex = 19;
inline constexpr std::size_t kBlacksmithRestLineIndex = 20;
inline constexpr std::size_t kBlacksmithPlayerLineIndex = 21;
inline constexpr std::size_t kMapAcquisitionLineIndex = 5;
inline constexpr std::size_t kMapTutorialStartLineIndex = StoryIntro::MapTutorial::kStartIndex;
inline constexpr std::size_t kMapTutorialEndLineIndex = StoryIntro::MapTutorial::kEndIndex;
inline constexpr int kMenuMapTabIndex = 2;                     // Tab index used when opening the map through the menu.
inline constexpr int kMenuUmbraTabIndex = 4;                   // Tab index dedicated to the Umbra Ossea chart.
constexpr char kInventoryArrowLineText[] = "You can open your inventory through this menu button."; // StoryTeller line that introduces the menu button.

inline void removeBrokenWeaponIcons(Game& game) {
    auto& icons = game.itemController.icons();
    icons.erase(
        std::remove_if(icons.begin(), icons.end(), [&](const core::ItemIcon& icon) {
            const sf::Texture& texture = icon.sprite.getTexture();
            return &texture == &game.resources.weaponHolmabirBroken
                || &texture == &game.resources.weaponKattkavarBroken
                || &texture == &game.resources.weaponStiggedinBroken;
        }),
        icons.end()
    );
}

inline void openBlacksmithWeaponSelection(Game& game, const std::string& processed) {
    if (game.state == GameState::WeaponSelection)
        return;
    game.visibleText = processed;
    game.charIndex = processed.size();
    game.currentProcessedLine = processed;
    game.typewriterClock.restart();
    game.state = GameState::WeaponSelection;
    game.hoveredWeaponIndex = -1;
    game.weaponItemAdded = false;
    game.forgedWeaponName.clear();
}

inline void startWeaponForgingRest(Game& game) {
    auto& forging = game.weaponForging;
    if (forging.phase != Game::WeaponForgingState::Phase::Idle)
        return;
    forging.phase = Game::WeaponForgingState::Phase::FadingOut;
    forging.clock.restart();
    forging.alpha = 0.f;
    forging.autoAdvancePending = true;
    game.forgedWeaponPopupActive = false;
    if (game.forgeSound && game.forgeSound->getStatus() == sf::Sound::Status::Playing)
        game.forgeSound->stop();
}

inline void giveForgedWeapon(Game& game) {
    if (game.weaponItemAdded)
        return;
    if (game.selectedWeaponIndex < 0 || static_cast<std::size_t>(game.selectedWeaponIndex) >= game.weaponOptions.size())
        return;
    const auto& tex = game.weaponOptions[game.selectedWeaponIndex].texture;
    auto makeItemKey = [](std::string value) {
        std::string key;
        for (char ch : value) {
            if (std::isspace(static_cast<unsigned char>(ch)))
                continue;
            key.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return key;
    };
    std::string key = makeItemKey(game.weaponOptions[game.selectedWeaponIndex].displayName);
    game.itemController.addItem(tex, key);
    game.weaponItemAdded = true;
    core::itemActivation::activateItem(game, key);
}

inline void giveMapItem(Game& game) {
    if (game.mapItemCollected)
        return;
    game.itemController.addItem(game.resources.mapGlandular, "map_glandular");
    game.mapItemCollected = true;
}

// Replaces placeholder tokens with the correct speaker names (e.g., player input).
inline std::string injectSpeakerNames(const std::string& text, const Game& game);

inline void endMapTutorial(Game& game) {
    if (!game.mapTutorialActive)
        return;
    game.mapTutorialActive = false;
    game.mapTutorialAwaitingOk = false;
    game.mapTutorialHighlight.reset();
    game.mapTutorialAnchorNormalized = { 0.5f, 0.5f };
    game.mapTutorialPopupBounds = {};
    game.mapTutorialOkBounds = {};
    game.mapTutorialOkHovered = false;
    if (game.menuActive)
        game.menuActive = false;
    game.menuHoveredTab = -1;
}

inline void updateMapTutorialState(Game& game) {
    if (!game.mapTutorialActive)
        return;
    if (auto stepIndex = StoryIntro::MapTutorial::stepIndexFor(game.dialogueIndex)) {
        const auto& step = StoryIntro::MapTutorial::step(*stepIndex);
        game.mapTutorialHighlight = step.highlightLocation;
        game.mapTutorialAnchorNormalized = step.popupAnchorNormalized;
        game.mapTutorialAwaitingOk = true;
        if (game.currentDialogue && game.dialogueIndex < game.currentDialogue->size()) {
            game.currentProcessedLine = injectSpeakerNames((*game.currentDialogue)[game.dialogueIndex].text, game);
        }
    }
    else {
        endMapTutorial(game);
    }
}

inline void startMapTutorial(Game& game) {
    if (game.mapTutorialActive)
        return;
    game.mapTutorialActive = true;
    game.mapTutorialAwaitingOk = true;
    game.menuActive = true;
    game.menuActiveTab = 2;
    game.menuHoveredTab = -1;
    game.mouseMapHover.reset();
    game.keyboardMapHover.reset();
    game.menuButtonUnlocked = true;
    game.menuButtonAlpha = 1.f;
    game.menuButtonFadeActive = false;
    updateMapTutorialState(game);
}


inline void openMenuMapFromDialogue(Game& game) {
    game.menuActive = true;
    game.menuActiveTab = kMenuMapTabIndex;
    game.menuHoveredTab = -1;
    game.mouseMapHover.reset();
    game.keyboardMapHover.reset();
    game.menuButtonUnlocked = true;
    game.menuButtonAlpha = 1.f;
    game.menuButtonFadeActive = false;
    game.menuButtonHovered = false;
    game.menuMapPopup.reset();
    game.mapInteractionUnlocked = true;
}

inline void openMenuUmbraMapFromDialogue(Game& game) {
    game.menuActive = true;
    game.menuActiveTab = kMenuUmbraTabIndex;
    game.menuHoveredTab = -1;
    game.mouseMapHover.reset();
    game.keyboardMapHover.reset();
    game.menuButtonUnlocked = true;
    game.menuButtonAlpha = 1.f;
    game.menuButtonFadeActive = false;
    game.menuButtonHovered = false;
    game.menuMapPopup.reset();
    game.mapInteractionUnlocked = false;
    game.pendingReturnToMenuMap = true;
    game.umbraMapGlowActive = true;
    game.umbraMapGlowClock.restart();
}

inline void triggerQuestAction(Game& game, const std::optional<std::string>& questName, bool start) {
    if (!questName)
        return;
    if (auto questDefinition = Story::questNamed(*questName)) {
        if (start)
            game.startQuest(*questDefinition);
        else
            game.completeQuest(*questDefinition);
    }
}

inline void handleDialogueLineActions(Game& game, const DialogueLine& line) {
    using Action = DialogueLineAction;
    if (dialogueLineHasAction(line.actions, Action::StartsQuest))
        triggerQuestAction(game, line.questStart, true);
    if (dialogueLineHasAction(line.actions, Action::CompletesQuest))
        triggerQuestAction(game, line.questComplete, false);
    if (dialogueLineHasAction(line.actions, Action::OpensMapFromMenu)) {
        openMenuMapFromDialogue(game);
        game.pendingReturnToMenuMap = true;
    }
    if (dialogueLineHasAction(line.actions, Action::OpensUmbraMapFromMenu)) {
        openMenuUmbraMapFromDialogue(game);
    }
    if (dialogueLineHasAction(line.actions, Action::StartsSeminiferousTeleport)) {
        game.pendingTeleportToSeminiferous = true;
        game.pendingTeleportToGonad = false;
    }
}


inline EnterAction processEnter(
    bool askingName,
    const std::string& nameInput,
    const DialogueLine& currentLine,
    std::size_t dialogueIndex,
    std::size_t dialogueCount,
    std::size_t charIndex,
        std::size_t processedLength
) {
    // Determine which Enter-driven actions should fire right now.
    EnterAction action;

    if (askingName) {
        if (!nameInput.empty())
            action.confirmName = true;
        return action;
    }

    if (charIndex < processedLength) {
        action.skipToEnd = true;
        return action;
    }

    if (dialogueIndex + 1 < dialogueCount) {
        action.nextLine = true;
    }

    return action;
}

// Detects whether the provided dialogue pointer references one of the dragon branches.
inline bool isDragonDialogue(const std::vector<DialogueLine>* dlg) {
    return dlg == &firedragon
        || dlg == &waterdragon
        || dlg == &earthdragon
        || dlg == &airdragon;
}

inline std::optional<std::size_t> quizQuestionIndexFor(const std::vector<DialogueLine>* dlg) {
    if (!dlg)
        return std::nullopt;
    for (std::size_t index = 0; index < dlg->size(); ++index) {
        if (dialogueLineHasAction((*dlg)[index].actions, DialogueLineAction::StartsQuiz)) {
            std::size_t questionIndex = index + 1;
            if (questionIndex < dlg->size())
                return questionIndex;
            break;
        }
    }
    return std::nullopt;
}

// Begins the timed quiz intro sequence triggered during dragon dialogue.
inline void startQuizIntroSequence(Game& game, std::size_t questionIndex) {
    auto& quiz = game.quiz;
    if (quiz.intro.active)
        return;

    // Start the intro fade/clocks so the quiz transition can animate.
    quiz.intro.active = true;
    quiz.intro.clock.restart();
    // Record the question that triggered this intro and the source dialogue.
    quiz.intro.questionIndex = questionIndex;
    quiz.intro.targetLocation = game.currentLocation ? game.currentLocation->id : LocationId::Gonad;
    quiz.intro.dialogue = game.currentDialogue;
    quiz.quizAutoStarted = false;
    quiz.questionStartSuppressed = true;
    // Mute the current location music so the quiz audio can take over.
    quiz.locationMusicVolumeBeforeQuiz = game.audioManager.hasLocationMusic()
        ? game.audioManager.locationMusicVolume()
        : 100.f;
    quiz.locationMusicMuted = true;
    game.audioManager.setLocationMusicVolume(0.f);
    game.audioManager.cancelLocationMusicFade();
    if (game.quizStartSound) {
        game.quizStartSound->stop();
        game.quizStartSound->play();
    }
}

// Starts the final cheer sequence once a dragon quiz concludes.
inline void maybeTriggerFinalCheer(Game& game) {
    auto& quiz = game.quiz;
    if (quiz.finalCheerTriggered)
        return;
    if (!isDragonDialogue(game.currentDialogue))
        return;
    if (game.dialogueIndex != kDragonFinalCheerLine)
        return;

    quiz.finalCheerTriggered = true;
    quiz.finalCheerActive = true;
    quiz.finalCheerClock.restart();
    stopQuestionAudio(game);
    if (game.quizEndSound) {
        game.quizEndSound->stop();
        game.quizEndSound->play();
    }
    if (game.audioManager.hasLocationMusic() && quiz.locationMusicMuted) {
        game.audioManager.setLocationMusicVolume(quiz.locationMusicVolumeBeforeQuiz);
        quiz.locationMusicMuted = false;
    }
    game.audioManager.cancelLocationMusicFade();
}

// Advances the dialogue index and handles dragon quiz transitions as if Enter was pressed.
inline bool advanceDialogueLine(Game& game) {
    game.stopTypingSound();
    const DialogueLine* completedLine = nullptr;
    if (game.currentDialogue && game.dialogueIndex < game.currentDialogue->size())
        completedLine = &(*game.currentDialogue)[game.dialogueIndex];
    game.dialogueIndex++;
    game.visibleText.clear();
    game.charIndex = 0;
    game.typewriterClock.restart();
    if (game.enterSound) {
        game.enterSound->stop();
        game.enterSound->play();
    }
    if (isDragonDialogue(game.currentDialogue)) {
        if (game.currentDialogue && game.dialogueIndex < game.currentDialogue->size()) {
            const auto& nextLine = (*game.currentDialogue)[game.dialogueIndex];
            if (dialogueLineHasAction(nextLine.actions, DialogueLineAction::StartsQuiz)) {
                if (auto questionIndex = quizQuestionIndexFor(game.currentDialogue))
                    startQuizIntroSequence(game, *questionIndex);
            }
        }
        maybeTriggerFinalCheer(game);
    }

    if (game.currentDialogue == &blacksmith) {
        game.forgedWeaponPopupActive = (game.dialogueIndex == kBlacksmithPlayerLineIndex);
    }
    else {
        game.forgedWeaponPopupActive = false;
    }

    if (game.currentDialogue == &gonad_part_two) {
        game.mapItemPopupActive = (game.dialogueIndex == kMapAcquisitionLineIndex);
        if (game.dialogueIndex == kMapAcquisitionLineIndex)
            giveMapItem(game);
        if (game.dialogueIndex == kMapTutorialStartLineIndex)
            startMapTutorial(game);
        if (game.mapTutorialActive) {
            if (game.dialogueIndex > kMapTutorialEndLineIndex)
                endMapTutorial(game);
            else
                updateMapTutorialState(game);
        }
    }
    else {
        game.mapItemPopupActive = false;
        endMapTutorial(game);
    }

    if (game.currentDialogue == &blacksmith
        && game.dialogueIndex == kBlacksmithPlayerLineIndex)
    {
        giveForgedWeapon(game);
    }

    if (game.currentDialogue == &perigonal) {
        if (game.dialogueIndex == kBrokenWeaponPreviewLineIndex) {
            // Broken Weapon Popup
            ui::brokenweapon::showPreview(game);
            if (!game.brokenWeaponsStored) {
                game.itemController.addItem(game.resources.weaponHolmabirBroken, "weapon_holmabir_broken");
                game.itemController.addItem(game.resources.weaponKattkavarBroken, "weapon_kattkavar_broken");
                game.itemController.addItem(game.resources.weaponStiggedinBroken, "weapon_stiggedin_broken");
                game.brokenWeaponsStored = true;
            }
        }
        else if (game.dialogueIndex == kWillFigsidLineIndex) {
            ui::brokenweapon::hidePreview(game);
        }
        if (game.dialogueIndex < game.currentDialogue->size()) {
            const auto& currentLine = (*game.currentDialogue)[game.dialogueIndex];
            if (currentLine.text == kInventoryArrowLineText) {
                game.inventoryArrowActive = true;
                game.inventoryTutorialPending = true;
                game.inventoryTutorialPopupActive = false;
                game.inventoryTutorialCompleted = false;
                game.inventoryArrowBlinkClock.restart();
                game.inventoryArrowVisible = true;
                if (!game.menuButtonUnlocked) {
                    game.menuButtonUnlocked = true;
                    game.menuButtonAlpha = 0.f;
                    game.menuButtonFadeActive = true;
                    game.menuButtonFadeClock.restart();
                }
            }
        }
    }
    if (completedLine)
        handleDialogueLineActions(game, *completedLine);
    return true;
}

// Keeps the final-cheer animation from being skipped until the delay passes.
inline bool shouldBlockFinalCheerAdvance(Game& game) {
    auto& quiz = game.quiz;
    if (!quiz.finalCheerActive)
        return false;
    if (quiz.finalCheerClock.getElapsedTime().asSeconds() >= kFinalCheerDelay) {
        quiz.finalCheerActive = false;
        return false;
    }
    return true;
}

// Handles Enter presses while dialog is advancing, managing quizzes and scene transitions.
inline bool waitForEnter(Game& game, const DialogueLine& line) {
    if (game.introDialogueFinished)
        return false;
    if (game.holdMapDialogue)
        return false;

    bool transitioned = false;
    // Helper to switch to another dialogue sequence while resetting text state.
    auto startDialogue = [&](const std::vector<DialogueLine>* nextDialogue) {
        game.holdMapDialogue = false;
        game.currentDialogue = nextDialogue;
        game.dialogueIndex = 0;
        game.visibleText.clear();
        game.charIndex = 0;
        game.typewriterClock.restart();
        transitioned = true;
    };

    // Cache dialogue pointer and size for Enter processing.
    auto* dialog = game.currentDialogue;
    std::size_t count = dialog ? dialog->size() : 0;
    std::string processed = injectSpeakerNames(line.text, game);

    // Determine what the Enter key should do next for this line.
    EnterAction action = processEnter(
        game.askingName,
        game.nameInput,
        line,
        game.dialogueIndex,
        count,
        game.charIndex,
        processed.size()
    );
    // Prevent Enter from skipping the final cheer during its delay.
    if (action.nextLine && shouldBlockFinalCheerAdvance(game))
        return true;
    // Block advances that happen while the quiz intro is still animating.
    if (action.nextLine
        && isDragonDialogue(game.currentDialogue)
        && game.currentDialogue
        && game.dialogueIndex < game.currentDialogue->size()
        && dialogueLineHasAction((*game.currentDialogue)[game.dialogueIndex].actions, DialogueLineAction::StartsQuiz)
        && game.quiz.intro.active)
        return true;
    if (game.inventoryTutorialPopupActive && game.menuActive)
        return true;
    if (game.inventoryArrowActive
        && game.currentDialogue == &perigonal
        && line.text == kInventoryArrowLineText
        && !game.inventoryTutorialCompleted)
        return true;
    if (game.healingPotionActive)
        return true;

    if (game.mapTutorialActive
        && game.dialogueIndex >= kMapTutorialStartLineIndex
        && game.dialogueIndex <= kMapTutorialEndLineIndex
        && game.mapTutorialAwaitingOk)
    {
        game.currentProcessedLine = processed;
        return true;
    }

    if (game.weaponForging.phase != Game::WeaponForgingState::Phase::Idle)
        return true;

    bool startingHealingPotion = game.currentDialogue == &perigonal
        && game.dialogueIndex == kHealingPotionLineIndex
        && action.nextLine
        && !game.healingPotionReceived;
    if (startingHealingPotion) {
        if (helper::healingPotion::start(game))
            return true;
    }

    // Detect when we are replaying the return-to-map dialogue after teleporting.
    bool isReturnToMapDialogue = game.transientReturnToMap && game.currentDialogue == &game.transientDialogue;

    // Guard against fogging the map dialogue when nothing should change yet.
    bool mapDialogueFinished = game.state == GameState::MapSelection
        && !isReturnToMapDialogue
        && game.charIndex >= processed.size()
        && !action.nextLine
        && !action.skipToEnd
        && !line.triggersNameInput
        && !game.askingName;
    if (mapDialogueFinished)
        return false;

    // Ask the player to confirm their typed name before continuing.
    if (action.confirmName) {
        game.stopTypingSound();
        game.askingName = false;
        
        showConfirmationPrompt(
            game,
            "Is \"" + game.nameInput + "\" correct?",
            [](Game& confirmedGame) {
                confirmedGame.playerName = confirmedGame.nameInput;
                playerDisplayName = confirmedGame.playerName;
                confirmedGame.beginSessionTimer();
                confirmedGame.dialogueIndex++;
                confirmedGame.visibleText.clear();
                confirmedGame.charIndex = 0;
                confirmedGame.typewriterClock.restart();
            },
            [](Game& retryGame) {
                retryGame.nameInput.clear();
                retryGame.visibleText.clear();
                retryGame.charIndex = 0;
                retryGame.typewriterClock.restart();
            }
        );
        return true;
    }

    // Immediately reveal the rest of the current dialogue line.
    if (action.skipToEnd) {
        game.stopTypingSound();
        game.visibleText = processed;
        game.charIndex = processed.size();
        return true;
    }

    // Manage Enter while the quiz feedback text is still on screen.
    if (game.quiz.feedbackActive && game.currentDialogue == &game.quiz.feedbackDialogue) {
        // Reveal the rest of the feedback line if Enter is pressed early.
        if (game.charIndex < processed.size()) {
            game.stopTypingSound();
            game.visibleText = processed;
            game.charIndex = processed.size();
            return true;
        }

        // Advance to the next feedback line if the player pressed Enter.
        if (action.nextLine) {
            game.stopTypingSound();
            game.dialogueIndex++;
            game.visibleText.clear();
            game.charIndex = 0;
            game.typewriterClock.restart();
            if (game.enterSound) {
                game.enterSound->stop();
                game.enterSound->play();
            }
            return true;
        }

        // Returns from the feedback screen to the active quiz question.
        auto resumeQuiz = [&]() {
            game.quiz.feedbackActive = false;
            // Keep the last feedback text visible until the next reply.
            if (game.visibleText.empty() && game.dialogueIndex < game.currentDialogue->size())
                game.visibleText = (*game.currentDialogue)[game.dialogueIndex].text;
            game.charIndex = game.visibleText.size();
            game.quiz.hoveredIndex = -1;
            game.state = GameState::Quiz;
            game.quiz.active = true;
            game.currentDialogue = &game.quiz.feedbackDialogue;
            game.dialogueIndex = game.quiz.feedbackDialogue.size();
            beginQuestionAudio(game);
        };

        // Regenerate the current quiz question if the retry joker was used.
        if (game.quiz.pendingRetry) {
            game.quiz.pendingRetry = false;
            if (game.quiz.pendingSillyReplacement) {
                game.quiz.questions[game.quiz.currentQuestion] = std::move(*game.quiz.pendingSillyReplacement);
                game.quiz.pendingSillyReplacement.reset();
            } else {
                regenerateCurrentQuestion(game);
            }
            resumeQuiz();
            return true;
        }

        // Move to the next quiz question after feedback.
        if (game.quiz.pendingQuestionAdvance) {
            game.quiz.pendingQuestionAdvance = false;
            game.quiz.currentQuestion++;
            resumeQuiz();
            return true;
        }

        // Finish the quiz, resume dialogue, and trigger the final cheer if needed.
        if (game.quiz.pendingFinish) {
            game.quiz.pendingFinish = false;
            game.quiz.feedbackActive = false;
            game.quiz.feedbackDialogue.clear();
            game.quiz.active = false;
            game.state = GameState::Dialogue;
            game.currentDialogue = game.quiz.quizDialogue;
            game.dialogueIndex = game.quiz.questionIndex + 3;
            game.visibleText.clear();
            game.charIndex = 0;
            game.typewriterClock.restart();
            game.quiz.pendingSuccess = true;
            maybeTriggerFinalCheer(game);
            return true;
        }
    }

    // Activate name-entry mode when the dialogue line requests it.
    if (line.triggersNameInput) {
        game.askingName = true;
        game.nameInput.clear();
        return true;
    }

    // Returns the quiz question line index for dragon dialogue sequences.
    auto questionIndexForDialogue = [&](const std::vector<DialogueLine>* dlg) -> std::optional<std::size_t> {
        if (!dlg || !isDragonDialogue(dlg))
            return std::nullopt;
        return quizQuestionIndexFor(dlg);
    };

    // During normal dialogue, pressing Enter at the quiz trigger line launches the quiz.
    if (game.state == GameState::Dialogue) {
        if (auto qIdx = questionIndexForDialogue(game.currentDialogue)) {
            if (game.dialogueIndex == *qIdx && action.nextLine) {
                if (game.currentLocation && !game.quiz.quizAutoStarted) {
                    startQuiz(game, game.currentLocation->id, *qIdx);
                    beginQuestionAudio(game);
                }
                return true;
            }
        }
    }

    // Advance to the next dialogue line when Enter is pressed at the end of the current text.
    if (game.currentDialogue == &blacksmith
        && game.dialogueIndex == kBlacksmithSelectionLineIndex
        && action.nextLine
        && game.state == GameState::Dialogue
        && !game.confirmationPrompt.active)
    {
        openBlacksmithWeaponSelection(game, processed);
        return true;
    }

    if (game.currentDialogue == &blacksmith
        && game.dialogueIndex == kBlacksmithRestLineIndex
        && action.nextLine
        && game.selectedWeaponIndex >= 0)
    {
        startWeaponForgingRest(game);
        return true;
    }

    if (action.nextLine)
        return advanceDialogueLine(game);

    if (!action.skipToEnd)
        handleDialogueLineActions(game, line);

    // If the player is still naming the weapon, stay in weapon-selection mode.
    if (game.currentDialogue == &weapon && game.selectedWeaponIndex < 0) {
        game.stopTypingSound();
        game.visibleText = processed;
        game.charIndex = processed.size();
        game.state = GameState::WeaponSelection;
        game.hoveredWeaponIndex = -1;
        return true;
    }

    game.stopTypingSound();
    bool keepReturnDialogueVisible = isReturnToMapDialogue && !action.nextLine;

    game.visibleText = keepReturnDialogueVisible ? processed : "";
    game.charIndex = processed.size();
    game.currentProcessedLine = processed;

    // Handle the queued quiz success payoff once feedback returns to dialogue.
    if (game.quiz.pendingSuccess && game.quiz.quizDialogue == game.currentDialogue) {
        game.quiz.pendingSuccess = false;
        completeQuizSuccess(game);
        return true;
    }

    // During the final encounter, wait for dialogue to finish before launching end options.
    if (game.finalEncounterActive && !action.nextLine && game.currentDialogue == &game.transientDialogue) {
        game.stopTypingSound();
        game.visibleText.clear();
        game.charIndex = processed.size();
        startFinalChoice(game);
        return true;
    }

    // Helper that starts the end-sequence fade once the final dialogue finishes.
    auto triggerEndSequence = [&]() {
        game.finalEndingPending = false;
        game.recordSessionRanking();
        game.uiFadeOutActive = true;
        game.uiFadeClock.restart();
        game.endSequenceController.start();
    };

    // Trigger the end sequence once the transient dialogue has played through.
    if (game.finalEndingPending
        && game.currentDialogue == &game.transientDialogue
        && game.dialogueIndex + 1 >= game.currentDialogue->size()) {
        game.stopTypingSound();
        triggerEndSequence();
        return true;
    }

    // Handle transitions between the fixed dialogue pools once a sequence completes.
    if (game.currentDialogue == &intro && !game.introDialogueFinished) {
        game.introDialogueFinished = true;
        game.audioManager.stopIntroDialogueMusic();
        game.pendingPerigonalDialogue = true;
        game.queuedBackgroundTexture = &game.resources.backgroundPetrigonal;
        game.uiFadeOutActive = true;
        game.uiFadeClock.restart();
        // Clear any lingering speaker/text so no portrait/name shows during the transition.
        game.currentDialogue = nullptr;
        game.lastSpeaker.reset();
        game.visibleText.clear();
        game.currentProcessedLine.clear();
        game.charIndex = 0;
    }
    else if (game.currentDialogue == &perigonal) {
        game.introDialogueFinished = true;
        game.pendingGonadPartOneDialogue = true;
        game.queuedBackgroundTexture = &game.resources.backgroundGonad;
        game.uiFadeOutActive = true;
        game.uiFadeClock.restart();
        // Clear any lingering speaker/text so no portrait/name shows during the transition.
        game.currentDialogue = nullptr;
        game.lastSpeaker.reset();
        game.visibleText.clear();
        game.currentProcessedLine.clear();
        game.charIndex = 0;
    }
    else if (game.currentDialogue == &gonad_part_one) {
        game.introDialogueFinished = true;
        game.pendingBlacksmithDialogue = true;
        game.queuedBackgroundTexture = &game.resources.backgroundBlacksmith;
        game.uiFadeOutActive = true;
        game.uiFadeClock.restart();
        game.currentDialogue = nullptr;
        game.lastSpeaker.reset();
        game.visibleText.clear();
        game.currentProcessedLine.clear();
        game.charIndex = 0;
    }
    else if (game.currentDialogue == &blacksmith) {
        game.introDialogueFinished = true;
        game.pendingGonadPartTwoDialogue = true;
        game.queuedBackgroundTexture = &game.resources.backgroundGonad;
        game.uiFadeOutActive = true;
        game.uiFadeClock.restart();
        game.currentDialogue = nullptr;
        game.lastSpeaker.reset();
        game.visibleText.clear();
        game.currentProcessedLine.clear();
        game.charIndex = 0;
    }
    else if (game.currentDialogue == &gonad_part_two) {
        bool finishedFinalLine = game.dialogueIndex + 1 >= game.currentDialogue->size();
        if (finishedFinalLine && !game.forcedDestinationSelection) {
            game.state = GameState::Dialogue;
            game.selectedWeaponIndex = -1;
            if (auto location = Locations::findById(game.locations, LocationId::Gonad))
                game.setCurrentLocation(location, false);
            game.beginForcedDestinationSelection();
        }
    }
    else if (isReturnToMapDialogue) {
        if (game.pendingTeleportToSeminiferous) {
            game.pendingTeleportToSeminiferous = false;
            game.transientReturnToMap = false;
            game.pendingReturnToMenuMap = false;
            game.menuActive = false;
            game.menuActiveTab = -1;
            game.menuHoveredTab = -1;
            game.mapInteractionUnlocked = false;
            game.menuMapPopup.reset();
            game.beginTeleport(LocationId::Seminiferous);
            return true;
        }
        if (game.pendingTeleportToGonad) {
            game.pendingTeleportToGonad = false;
            game.transientReturnToMap = false;
            game.pendingReturnToMenuMap = false;
            game.menuActive = false;
            game.menuActiveTab = -1;
            game.menuHoveredTab = -1;
            game.mapInteractionUnlocked = false;
            game.menuMapPopup.reset();
            game.beginTeleport(LocationId::Gonad);
            return true;
        }

        if (!game.pendingReturnToMenuMap)
            game.state = GameState::MapSelection;
        else
            game.pendingReturnToMenuMap = false;
        game.transientReturnToMap = false;
        game.keyboardMapHover.reset();
        game.mouseMapHover.reset();

        if (keepReturnDialogueVisible) {
            game.holdMapDialogue = true;
            return true;
        }

        game.currentDialogue = nullptr;
        game.visibleText.clear();
        game.charIndex = 0;
    }

    // If startDialogue was called (we transitioned to a new dialogue), play enter sound.
    if (transitioned && game.enterSound) {
        game.enterSound->stop();
        game.enterSound->play();
    }

    return true;
}

inline void updateWeaponForging(Game& game) {
    auto& forging = game.weaponForging;
    switch (forging.phase) {
        case Game::WeaponForgingState::Phase::Idle:
            return;
        case Game::WeaponForgingState::Phase::FadingOut: {
            float progress = std::min(1.f, forging.clock.getElapsedTime().asSeconds() / kWeaponForgingFadeDuration);
            forging.alpha = progress;
            if (progress >= 1.f) {
                forging.phase = Game::WeaponForgingState::Phase::Sleeping;
                forging.clock.restart();
                if (!game.forgeSound)
                    game.forgeSound.emplace(game.resources.forgeSound);
                else
                    game.forgeSound->setBuffer(game.resources.forgeSound);
                game.forgeSound->setLooping(true);
                game.forgeSound->play();
            }
            break;
        }
        case Game::WeaponForgingState::Phase::Sleeping: {
            forging.alpha = 1.f;
            if (forging.clock.getElapsedTime().asSeconds() >= kWeaponForgingSleepDuration) {
                forging.phase = Game::WeaponForgingState::Phase::FadingIn;
                forging.clock.restart();
                if (game.forgeSound && game.forgeSound->getStatus() == sf::Sound::Status::Playing)
                    game.forgeSound->stop();
            }
            break;
        }
        case Game::WeaponForgingState::Phase::FadingIn: {
            float progress = std::min(1.f, forging.clock.getElapsedTime().asSeconds() / kWeaponForgingFadeDuration);
            forging.alpha = 1.f - progress;
            if (progress >= 1.f) {
                forging.phase = Game::WeaponForgingState::Phase::Idle;
                forging.alpha = 0.f;
                if (game.forgeSound && game.forgeSound->getStatus() == sf::Sound::Status::Playing)
                    game.forgeSound->stop();
                if (forging.autoAdvancePending) {
                    forging.autoAdvancePending = false;
                    advanceDialogueLine(game);
                }
            }
            break;
        }
    }
}

inline float weaponForgingOverlayAlpha(const Game& game) {
    return game.weaponForging.phase == Game::WeaponForgingState::Phase::Idle
        ? 0.f
        : game.weaponForging.alpha;
}

inline bool weaponForgingOverlayVisible(const Game& game) {
    return game.weaponForging.phase != Game::WeaponForgingState::Phase::Idle;
}

// Replace tokens such as {player} or {weapon} with dynamic names before displaying text.
inline std::string injectSpeakerNames(const std::string& text, const Game& game) {
    std::string out = text;

    // Helper that returns the display name of the currently selected weapon (or placeholder).
    auto selectedWeaponName = [&]() -> std::string {
        if (game.selectedWeaponIndex >= 0 && game.selectedWeaponIndex < static_cast<int>(game.weaponOptions.size()))
            return game.weaponOptions[game.selectedWeaponIndex].displayName;
        return "your weapon";
    };
    auto forgedWeaponName = [&]() -> std::string {
        if (!game.forgedWeaponName.empty())
            return game.forgedWeaponName;
        return selectedWeaponName();
    };

    // Fetches the name of the most recently completed location for {lastLocation}.
    auto lastLocationName = [&]() -> std::string {
        if (!game.lastCompletedLocation)
            return "";
        if (auto loc = Locations::findById(game.locations, *game.lastCompletedLocation))
            return loc->name;
        return "";
    };

    // Replaces every occurrence of a placeholder token with the provided value.
    auto replaceToken = [&](const std::string& token, const std::string& value) {
        if (value.empty())
            return;

        std::size_t pos = 0;
        while ((pos = out.find(token, pos)) != std::string::npos) {
            out.replace(pos, token.size(), value);
            pos += value.size();
        }
    };

    // Expand each known placeholder with its current runtime value.
    replaceToken("{player}", game.playerName);
    replaceToken("{playerName}", game.playerName);
    replaceToken("{fireDragon}", TextStyles::speakerStyle(TextStyles::SpeakerId::FireDragon).name);
    replaceToken("{waterDragon}", TextStyles::speakerStyle(TextStyles::SpeakerId::WaterDragon).name);
    replaceToken("{earthDragon}", TextStyles::speakerStyle(TextStyles::SpeakerId::EarthDragon).name);
    replaceToken("{airDragon}", TextStyles::speakerStyle(TextStyles::SpeakerId::AirDragon).name);
    replaceToken("{lastDragonName}", game.lastDragonName);
    replaceToken("{weapon}", selectedWeaponName());
    replaceToken("{lastLocation}", lastLocationName());
    replaceToken("{weaponName}", forgedWeaponName());
    auto otherGender = (game.playerGender == Game::DragonbornGender::Male)
        ? Game::DragonbornGender::Female
        : Game::DragonbornGender::Male;
    auto otherName = (otherGender == Game::DragonbornGender::Male)
        ? "Asha Scale"
        : "Ember Scale";
    auto otherSubject = (otherGender == Game::DragonbornGender::Male) ? "he" : "she";
    auto otherObject = (otherGender == Game::DragonbornGender::Male) ? "him" : "her";
    auto otherPossessive = (otherGender == Game::DragonbornGender::Male) ? "his" : "her";
    auto otherPossessivePronoun = (otherGender == Game::DragonbornGender::Male) ? "his" : "hers";
    auto otherSibling = (otherGender == Game::DragonbornGender::Male) ? "brother" : "sister";
    auto otherSiblingName = (otherGender == Game::DragonbornGender::Male) ? "Asha" : "Ember";
    auto ownPossessive = (otherGender == Game::DragonbornGender::Male) ? "her" : "his";

    StoryIntro::refreshDynamicDragonbornTokens(
        game.playerName,
        otherName,
        otherSubject,
        otherObject,
        otherPossessive,
        otherPossessivePronoun,
        otherSibling,
        otherSiblingName,
        ownPossessive
    );
    replaceToken("{dragonbornName}", otherName);
    replaceToken("{dragonbornSubject}", otherSubject);
    replaceToken("{dragonbornObject}", otherObject);
    replaceToken("{dragonbornPossessive}", otherPossessive);
    replaceToken("{dragonbornPossessivePronoun}", otherPossessivePronoun);
    replaceToken("{dragonbornSibling}", otherSibling);
    replaceToken("{dragonbornSiblingName}", otherSiblingName);
    replaceToken("{dragonbornOwnPossessive}", ownPossessive);
    return out;
}
