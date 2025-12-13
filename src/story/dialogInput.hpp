#pragma once
#include "dialogueLine.hpp"
#include "core/game.hpp"
#include "textStyles.hpp"
#include "ui/confirmationUI.hpp"
#include "ui/quizUI.hpp"

struct EnterAction {
    bool confirmName = false;
    bool skipToEnd = false;
    bool nextLine = false;
};

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

inline bool isDragonDialogue(const std::vector<DialogueLine>* dlg) {
    return dlg == &firedragon
        || dlg == &waterdragon
        || dlg == &earthdragon
        || dlg == &airdragon;
}

inline void startQuizIntroSequence(Game& game, std::size_t questionIndex) {
    auto& quiz = game.quiz;
    if (quiz.intro.active)
        return;

    quiz.intro.active = true;
    quiz.intro.clock.restart();
    quiz.intro.questionIndex = questionIndex;
    quiz.intro.targetLocation = game.currentLocation ? game.currentLocation->id : LocationId::Gonad;
    quiz.intro.dialogue = game.currentDialogue;
    quiz.quizAutoStarted = false;
    quiz.questionStartSuppressed = true;
    quiz.locationMusicVolumeBeforeQuiz = game.locationMusic ? game.locationMusic->getVolume() : 100.f;
    quiz.locationMusicMuted = true;
    if (game.locationMusic) {
        game.locationMusic->setVolume(0.f);
        game.locationMusicFade.active = false;
    }
    if (game.quizStartSound) {
        game.quizStartSound->stop();
        game.quizStartSound->play();
    }
}

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
    if (game.locationMusic && quiz.locationMusicMuted) {
        game.locationMusic->setVolume(quiz.locationMusicVolumeBeforeQuiz);
        quiz.locationMusicMuted = false;
    }
    game.locationMusicFade.active = false;
}

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

inline bool waitForEnter(Game& game, const DialogueLine& line) {
    if (game.introDialogueFinished)
        return false;
    if (game.holdMapDialogue)
        return false;

    bool transitioned = false;
    auto startDialogue = [&](const std::vector<DialogueLine>* nextDialogue) {
        game.holdMapDialogue = false;
        game.currentDialogue = nextDialogue;
        game.dialogueIndex = 0;
        game.visibleText.clear();
        game.charIndex = 0;
        game.typewriterClock.restart();
        transitioned = true;
    };

    auto* dialog = game.currentDialogue;
    std::size_t count = dialog ? dialog->size() : 0;
    std::string processed = injectSpeakerNames(line.text, game);

    EnterAction action = processEnter(
        game.askingName,
        game.nameInput,
        line,
        game.dialogueIndex,
        count,
        game.charIndex,
        processed.size()
    );
    if (action.nextLine && shouldBlockFinalCheerAdvance(game))
        return true;
    if (action.nextLine
        && isDragonDialogue(game.currentDialogue)
        && game.dialogueIndex == kDragonQuizIntroLine
        && game.quiz.intro.active)
        return true;

    bool isReturnToMapDialogue = game.transientReturnToMap && game.currentDialogue == &game.transientDialogue;

    bool mapDialogueFinished = game.state == GameState::MapSelection
        && !isReturnToMapDialogue
        && game.charIndex >= processed.size()
        && !action.nextLine
        && !action.skipToEnd
        && !line.triggersNameInput
        && !game.askingName;
    if (mapDialogueFinished)
        return false;

    if (action.confirmName) {
        game.stopTypingSound();
        game.askingName = false;
        
        showConfirmationPrompt(
            game,
            "Is \"" + game.nameInput + "\" correct?",
            [](Game& confirmedGame) {
                confirmedGame.playerName = confirmedGame.nameInput;
                playerDisplayName = confirmedGame.playerName;
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

    if (action.skipToEnd) {
        game.stopTypingSound();
        game.visibleText = processed;
        game.charIndex = processed.size();
        return true;
    }

    if (game.quiz.feedbackActive && game.currentDialogue == &game.quiz.feedbackDialogue) {
        if (game.charIndex < processed.size()) {
            game.stopTypingSound();
            game.visibleText = processed;
            game.charIndex = processed.size();
            return true;
        }

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

        if (game.quiz.pendingRetry) {
            game.quiz.pendingRetry = false;
            regenerateCurrentQuestion(game);
            resumeQuiz();
            return true;
        }

        if (game.quiz.pendingQuestionAdvance) {
            game.quiz.pendingQuestionAdvance = false;
            game.quiz.currentQuestion++;
            resumeQuiz();
            return true;
        }

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

    if (line.triggersNameInput) {
        game.askingName = true;
        game.nameInput.clear();
        return true;
    }

    auto questionIndexForDialogue = [&](const std::vector<DialogueLine>* dlg) -> std::optional<std::size_t> {
        if (!dlg)
            return std::nullopt;
        if (isDragonDialogue(dlg))
            return kDragonQuizQuestionLine;
        return std::nullopt;
    };

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
        if (isDragonDialogue(game.currentDialogue)) {
            if (game.dialogueIndex == kDragonQuizIntroLine)
                startQuizIntroSequence(game, kDragonQuizQuestionLine);
            maybeTriggerFinalCheer(game);
        }
        return true;
    }

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

    if (game.quiz.pendingSuccess && game.quiz.quizDialogue == game.currentDialogue) {
        game.quiz.pendingSuccess = false;
        completeQuizSuccess(game);
        return true;
    }

    if (game.finalEncounterActive && !action.nextLine && game.currentDialogue == &game.transientDialogue) {
        game.stopTypingSound();
        game.visibleText.clear();
        game.charIndex = processed.size();
        startFinalChoice(game);
        return true;
    }

    auto triggerEndSequence = [&]() {
        game.finalEndingPending = false;
        game.endSequenceActive = true;
        game.endFadeOutActive = true;
        game.endFadeInActive = false;
        game.endScreenVisible = false;
        game.endClock.restart();
    };

    if (game.finalEndingPending
        && game.currentDialogue == &game.transientDialogue
        && game.dialogueIndex + 1 >= game.currentDialogue->size()) {
        game.stopTypingSound();
        triggerEndSequence();
        return true;
    }

    if (game.currentDialogue == &intro && !game.introDialogueFinished) {
        game.introDialogueFinished = true;
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

inline std::string injectSpeakerNames(const std::string& text, const Game& game) {
    std::string out = text;

    auto selectedWeaponName = [&]() -> std::string {
        if (game.selectedWeaponIndex >= 0 && game.selectedWeaponIndex < static_cast<int>(game.weaponOptions.size()))
            return game.weaponOptions[game.selectedWeaponIndex].displayName;
        return "your weapon";
    };

    auto lastLocationName = [&]() -> std::string {
        if (!game.lastCompletedLocation)
            return "";
        if (auto loc = Locations::findById(game.locations, *game.lastCompletedLocation))
            return loc->name;
        return "";
    };

    auto replaceToken = [&](const std::string& token, const std::string& value) {
        if (value.empty())
            return;

        std::size_t pos = 0;
        while ((pos = out.find(token, pos)) != std::string::npos) {
            out.replace(pos, token.size(), value);
            pos += value.size();
        }
    };

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
