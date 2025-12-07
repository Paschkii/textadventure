#include "dialogUI.hpp"
#include <algorithm>
#include "story/dialogInput.hpp"
#include "story/storyIntro.hpp"
#include "dialogDrawElements.hpp"
#include "story/textStyles.hpp"
#include "rendering/colorHelper.hpp"
#include "confirmationUI.hpp"
#include "uiVisibility.hpp"

namespace {
    UiVisibility computeDialogueVisibility(Game& game) {
        UiElementMask visibilityMask =
            UiElement::TextBox
            | UiElement::NameBox
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

void drawLocationBox(Game& game, sf::RenderTarget& target, float uiAlphaFactor) {
    if (!game.currentLocation)
        return;

    constexpr unsigned int kLocationTextSize = 32;
    constexpr float kLocationOutlineThickness = 1.f;

    sf::Text locationName{ game.resources.uiFont, game.currentLocation->name, kLocationTextSize };
    locationName.setFillColor(ColorHelper::applyAlphaFactor(game.currentLocation->color, uiAlphaFactor));
    locationName.setOutlineColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, uiAlphaFactor));
    locationName.setOutlineThickness(kLocationOutlineThickness);

    auto boxPos = game.locationBox.getPosition();
    auto boxSize = game.locationBox.getSize();
    sf::Vector2f boxCenter{ boxPos.x + (boxSize.x / 2.f), boxPos.y + (boxSize.y / 2.f) };

    auto textBounds = locationName.getLocalBounds();
    sf::Vector2f origin{
        textBounds.position.x + (textBounds.size.x / 2.f),
        textBounds.position.y + (textBounds.size.y / 2.f)
    };
    locationName.setOrigin(origin);
    locationName.setPosition(boxCenter);

    target.draw(locationName);
}

void drawDialogueUI(Game& game, sf::RenderTarget& target) {

    UiVisibility visibility = computeDialogueVisibility(game);

    if (visibility.hidden)
        return;

    float uiAlphaFactor = visibility.alphaFactor;
    float glowElapsedSeconds = game.uiGlowClock.getElapsedTime().asSeconds();

    bool showLocationBox = game.currentDialogue != &intro;

    dialogDraw::drawDialogueFrames(game, target, uiAlphaFactor, glowElapsedSeconds, showLocationBox);

    if (showLocationBox)
        drawLocationBox(game, target, uiAlphaFactor);

    if (!game.currentDialogue || game.dialogueIndex >= game.currentDialogue->size())
        return;

    const auto& line = (*game.currentDialogue)[game.dialogueIndex];
    std::string fullText = injectSpeakerNames(line.text, game);

    float delay = 0.02f;
    bool isTyping = false;
    if (!game.confirmationPrompt.active)
        isTyping = updateTypewriter(game, fullText, delay);

    TextStyles::SpeakerStyle info = TextStyles::speakerStyle(line.speaker);
    dialogDraw::drawSpeakerName(target, game, info, uiAlphaFactor);

    std::string textToDraw = game.visibleText;
    if (game.askingName) {
        auto newlinePos = textToDraw.find('\n');
        if (newlinePos != std::string::npos)
            textToDraw = textToDraw.substr(0, newlinePos + 1);
    }

    dialogDraw::drawDialogueText(target, game, textToDraw, uiAlphaFactor);

    if (game.confirmationPrompt.active) {
        drawConfirmationPrompt(game, target, uiAlphaFactor);
        return;
    }

    if (game.askingName)
        dialogDraw::drawNameInput(target, game, uiAlphaFactor);

    dialogDraw::drawReturnPrompt(target, game, uiAlphaFactor, isTyping);
}