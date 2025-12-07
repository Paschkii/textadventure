#include "dialogUI.hpp"
#include <algorithm>
#include <cstdint>
#include <optional>
#include "story/dialogInput.hpp"
#include "story/storyIntro.hpp"
#include "dialogDrawElements.hpp"
#include "story/textStyles.hpp"
#include "rendering/colorHelper.hpp"
#include "confirmationUI.hpp"
#include "uiEffects.hpp"
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

    std::optional<std::size_t> highlightedDragonIndex(const Game& game) {
        if (game.currentDialogue != &dragon)
            return std::nullopt;

        std::size_t index = game.dialogueIndex;

        if (index >= 1 && index <= 3)
            return 0;
        if (index >= 5 && index <= 7)
            return 1;
        if (index >= 9 && index <= 11)
            return 2;
        if (index >= 13 && index <= 15)
            return 3;

        return std::nullopt;
    }

    void drawDragonShowcase(Game& game, sf::RenderTarget& target, float uiAlphaFactor, float glowElapsedSeconds) {
        if (game.dragonPortraits.empty())
            return;

        sf::Color glowColor = uiEffects::computeGlowColor(
            ColorHelper::Palette::BlueLight,
            glowElapsedSeconds,
            uiAlphaFactor,
            140.f,
            30.f,
            { 25.f, 41.f }
        );

        uiEffects::drawGlowFrame(
            target,
            game.uiFrame,
            game.weaponPanel.getPosition(),
            game.weaponPanel.getSize(),
            glowColor,
            2.f
        );

        sf::Color frameColor = ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, uiAlphaFactor);
        game.uiFrame.draw(target, game.weaponPanel, frameColor);

        constexpr float labelSize = 22.f;
        constexpr float outlinePadding = 6.f;

        auto highlighted = highlightedDragonIndex(game);

        for (std::size_t i = 0; i < game.dragonPortraits.size(); ++i) {
            auto& portrait = game.dragonPortraits[i];

            float scaleMultiplier = (highlighted && *highlighted == i) ? 1.1f : 1.f;
            portrait.sprite.setScale({ portrait.baseScale * scaleMultiplier, portrait.baseScale * scaleMultiplier });
            portrait.sprite.setPosition(portrait.centerPosition);

            sf::Color spriteColor = portrait.sprite.getColor();
            spriteColor.a = static_cast<std::uint8_t>(255.f * uiAlphaFactor);
            portrait.sprite.setColor(spriteColor);
            target.draw(portrait.sprite);

            if (highlighted && *highlighted == i) {
                auto bounds = portrait.sprite.getGlobalBounds();
                sf::RectangleShape outline({ bounds.size.x + (outlinePadding * 2.f), bounds.size.y + (outlinePadding * 2.f) });
                outline.setPosition({ bounds.position.x - outlinePadding, bounds.position.y - outlinePadding });
                outline.setFillColor(sf::Color::Transparent);
                outline.setOutlineColor(ColorHelper::applyAlphaFactor(sf::Color::White, uiAlphaFactor));
                outline.setOutlineThickness(3.f);
                target.draw(outline);
            }

            sf::Text label{ game.resources.titleFont, portrait.displayName, static_cast<unsigned int>(labelSize) };
            label.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, uiAlphaFactor));

            auto labelBounds = label.getLocalBounds();
            label.setOrigin({ labelBounds.position.x + (labelBounds.size.x / 2.f), labelBounds.position.y });
            label.setPosition(portrait.labelPosition);
            target.draw(label);
        }
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

    if (game.currentDialogue == &dragon)
        drawDragonShowcase(game, target, uiAlphaFactor, glowElapsedSeconds);

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