#pragma once
// === Header Files ===
#include "core/game.hpp"              // Accesses Game state manipulated while handling dialogue.
#include "dialogueLine.hpp"           // Uses DialogueLine metadata processed on Enter.
#include "textStyles.hpp"             // Formats speaker names and checks speaker IDs.
#include "ui/confirmationUI.hpp"      // Shows the name-confirmation modal.
#include "ui/quizUI.hpp"              // References quiz controls triggered mid-dialogue.

// Collects the actions triggered by pressing Enter during dialogue or name entry.
struct EnterAction {
    bool confirmName = false;
    bool skipToEnd = false;
    bool nextLine = false;
};

// Replaces placeholder tokens with the correct speaker names (e.g., player input).
inline std::string injectSpeakerNames(const std::string& text, const Game& game);


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
    game.dialogueIndex++;
    game.visibleText.clear();
    game.charIndex = 0;
    game.typewriterClock.restart();
    if (game.enterSound) {
        game.enterSound->stop();
        game.enterSound->play();
    }
    if (isDragonDialogue(game.currentDialogue)) {
        if (game.dialogueIndex == kDragonQuizIntroLine)
            startQuizIntroSequence(game, kDragonQuizQuestionLine);
        maybeTriggerFinalCheer(game);
    }
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
        && game.dialogueIndex == kDragonQuizIntroLine
        && game.quiz.intro.active)
        return true;

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
        if (!dlg)
            return std::nullopt;
        if (isDragonDialogue(dlg))
            return kDragonQuizQuestionLine;
        return std::nullopt;
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
    if (action.nextLine)
        return advanceDialogueLine(game);

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
        game.pendingGonadDialogue = true;
        game.uiFadeOutActive = true;
        game.uiFadeClock.restart();
        // Clear any lingering speaker/text so no portrait/name shows during the transition.
        game.currentDialogue = nullptr;
        game.lastSpeaker.reset();
        game.visibleText.clear();
        game.currentProcessedLine.clear();
        game.charIndex = 0;
    }
    else if (game.currentDialogue == &gonad) {
        startDialogue(&weapon);
        game.selectedWeaponIndex = -1;
    }
    else if (game.currentDialogue == &weapon) {
        startDialogue(&dragon);
        game.state = GameState::Dialogue;
    }
    else if (game.currentDialogue == &dragon) {
        startDialogue(&destination);
        // After the dragon dialogue we switch to the map selection state
        game.state = GameState::MapSelection;
    }
    else if (isReturnToMapDialogue) {
        if (game.pendingTeleportToGonad) {
            game.pendingTeleportToGonad = false;
            game.transientReturnToMap = false;
            game.beginTeleport(LocationId::Gonad);
            return true;
        }

        game.state = GameState::MapSelection;
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

// Replace tokens such as {player} or {weapon} with dynamic names before displaying text.
inline std::string injectSpeakerNames(const std::string& text, const Game& game) {
    std::string out = text;

    // Helper that returns the display name of the currently selected weapon (or placeholder).
    auto selectedWeaponName = [&]() -> std::string {
        if (game.selectedWeaponIndex >= 0 && game.selectedWeaponIndex < static_cast<int>(game.weaponOptions.size()))
            return game.weaponOptions[game.selectedWeaponIndex].displayName;
        return "your weapon";
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
    return out;
}
