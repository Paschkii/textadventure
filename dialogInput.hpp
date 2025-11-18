#pragma once
#include "dialogueLine.hpp"
#include "game.hpp"
#include "speaker.hpp"

struct EnterAction {
    bool confirmName = false;
    bool skipToEnd = false;
    bool nextLine = false;
};

inline EnterAction processEnter(
    bool askingName,
    const std::string& nameInput,
    const DialogueLine& currentLine,
    std::size_t dialogueIndex,
    std::size_t dialogueCount,
    std::size_t charIndex
) {
    EnterAction action;

    if (askingName) {
        if (!nameInput.empty())
            action.confirmName = true;
        return action;
    }

    if (charIndex < currentLine.text.size()) {
        action.skipToEnd = true;
        return action;
    }

    if (dialogueIndex + 1 < dialogueCount) {
        action.nextLine = true;
    }

    return action;
}

inline void waitForEnter(Game& game, const DialogueLine& line) {
    auto* dialog = game.currentDialogue;
    std::size_t count = dialog ? dialog->size() : 0;
    std::string processed = line.text;

    EnterAction action = processEnter(
        game.askingName,
        game.nameInput,
        line,
        game.dialogueIndex,
        count,
        game.charIndex
    );

    if (action.confirmName) {
        game.playerName = game.nameInput;
        playerDisplayName = game.playerName;
        game.askingName = false;
        game.dialogueIndex++;
        game.visibleText.clear();
        game.charIndex = 0;
        game.typewriterClock.restart();
        return;
    }

    if (action.skipToEnd) {
        game.visibleText = line.text;
        game.charIndex = line.text.size();
        return;
    }

    if (line.triggersNameInput) {
        game.askingName = true;
        game.nameInput.clear();
        return;
    }

    if (action.nextLine) {
        game.dialogueIndex++;
        game.visibleText.clear();
        game.charIndex = 0;
        game.typewriterClock.restart();
        return;
    }

    if(processed.find("{player}") != std::string::npos) {
        processed.replace(processed.find("{player}"), 8, game.playerName);
    }

    game.visibleText = "";
    game.charIndex = 0;
    game.currentProcessedLine = processed;
}

inline std::string injectSpeakerNames(const std::string& text, const Game& game) {
    std::string out = text;

    if (!game.playerName.empty()) {
        const std::string token = "{player}";
        std::size_t pos = 0;
        while ((pos = out.find(token, pos)) != std::string::npos) {
            out.replace(pos, token.size(), game.playerName);
            pos += game.playerName.size();
        }
    }

    return out;
}