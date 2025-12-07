#pragma once
#include "dialogueLine.hpp"
#include "core/game.hpp"
#include "textStyles.hpp"
#include "ui/confirmationUI.hpp"

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

inline void waitForEnter(Game& game, const DialogueLine& line) {
    if (game.introDialogueFinished)
        return;

    auto startDialogue = [&](const std::vector<DialogueLine>* nextDialogue) {
        game.currentDialogue = nextDialogue;
        game.dialogueIndex = 0;
        game.visibleText.clear();
        game.charIndex = 0;
        game.typewriterClock.restart();
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
        return;
    }

    if (action.skipToEnd) {
        game.stopTypingSound();
        game.visibleText = processed;
        game.charIndex = processed.size();
        return;
    }

    if (line.triggersNameInput) {
        game.askingName = true;
        game.nameInput.clear();
        return;
    }

    if (action.nextLine) {
        game.stopTypingSound();
        game.dialogueIndex++;
        game.visibleText.clear();
        game.charIndex = 0;
        game.typewriterClock.restart();
        return;
    }

    if (game.currentDialogue == &weapon && game.selectedWeaponIndex < 0) {
        game.stopTypingSound();
        game.visibleText = processed;
        game.charIndex = processed.size();
        game.state = GameState::WeaponSelection;
        game.hoveredWeaponIndex = -1;
        return;
    }

    game.stopTypingSound();
    game.visibleText = "";
    game.charIndex = processed.size();
    game.currentProcessedLine = processed;

    if (game.currentDialogue == &intro && !game.introDialogueFinished) {
        game.introDialogueFinished = true;
        game.uiFadeOutActive = true;
        game.uiFadeClock.restart();
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
    }
}

inline std::string injectSpeakerNames(const std::string& text, const Game& game) {
    std::string out = text;

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

    return out;
}