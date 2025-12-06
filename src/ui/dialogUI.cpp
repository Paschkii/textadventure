#include "dialogUI.hpp"
#include <algorithm>
#include "story/dialogInput.hpp"
#include "dialogDrawElements.hpp"
#include "story/textStyles.hpp"
#include "uiVisibility.hpp"

namespace {
    UiVisibility computeDialogueVisibility(Game& game) {
        UiElementMask visibilityMask =
            UiElement::TextBox
            | UiElement::NameBox
            | UiElement::LocationBox
            | UiElement::IntroTitle;

        return computeUiVisibility(game, visibilityMask);
    }

    bool updateTypewriter(Game& game, const std::string& fullText, float delay) {
        bool isTyping = !game.askingName && game.charIndex < fullText.size();

    if (isTyping) {
            game.startTypingSound();
            if (game.typewriterClock.getElapsedTime().asSeconds() >= delay) {
                game.visibleText += fullText[game.charIndex];
                game.charIndex++;
                game.typewriterClock.restart();
            }
        }
        else {
            game.stopTypingSound();
        }

        return isTyping;
    }
}

void drawLocationBox(Game&, sf::RenderTarget&) {
    // TODO: Implement location box rendering.
}

void drawDialogueUI(Game& game, sf::RenderTarget& target) {

    UiVisibility visibility = computeDialogueVisibility(game);

    if (visibility.hidden)
        return;

    float uiAlphaFactor = visibility.alphaFactor;
    float glowElapsedSeconds = game.uiGlowClock.getElapsedTime().asSeconds();

    dialogDraw::drawDialogueFrames(game, target, uiAlphaFactor, glowElapsedSeconds);
    drawLocationBox(game, target);

    if (!game.currentDialogue || game.dialogueIndex >= game.currentDialogue->size())
        return;

    const auto& line = (*game.currentDialogue)[game.dialogueIndex];
    std::string fullText = injectSpeakerNames(line.text, game);

    float delay = 0.02f;
    bool isTyping = updateTypewriter(game, fullText, delay);

    TextStyles::SpeakerStyle info = TextStyles::speakerStyle(line.speaker);
    dialogDraw::drawSpeakerName(target, game, info, uiAlphaFactor);

    std::string textToDraw = game.visibleText;
    if (game.askingName) {
        auto newlinePos = textToDraw.find('\n');
        if (newlinePos != std::string::npos)
            textToDraw = textToDraw.substr(0, newlinePos + 1);
    }

    dialogDraw::drawDialogueText(target, game, textToDraw, uiAlphaFactor);

    if (game.askingName)
        dialogDraw::drawNameInput(target, game, uiAlphaFactor);
        dialogDraw::drawReturnPrompt(target, game, uiAlphaFactor, isTyping);
}