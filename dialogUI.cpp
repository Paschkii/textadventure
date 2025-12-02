#include "dialogUI.hpp"
#include <algorithm>
#include <cmath>
#include "colorHelper.hpp"
#include "game.hpp"
#include "speaker.hpp"
#include "textColorHelper.hpp"
#include "textLayout.cpp"

namespace {
    constexpr float kNameBoxTextOffset = 20.f;
    constexpr float kTextBoxPadding = 20.f;
    constexpr unsigned int kTextCharacterSize = 28;

    sf::Color applyAlpha(sf::Color color, float factor) {
        color.a = static_cast<std::uint8_t>(static_cast<float>(color.a) * factor);
        return color;
    }
}

void drawLocationBox(Game&, sf::RenderTarget&) {
    // TODO: Implement location box rendering.
}

void drawDialogueUI(Game& game, sf::RenderTarget& target) {
    float uiAlphaFactor = 1.f;
    bool hideUI = false;

    if (game.introDialogueFinished) {
        if (game.uiFadeOutActive) {
            float fadeProgress = std::min(1.f, game.uiFadeClock.getElapsedTime().asSeconds() / game.uiFadeOutDuration);
            uiAlphaFactor = 1.f - fadeProgress;

            if (fadeProgress >= 1.f) {
                game.uiFadeOutActive = false;
                hideUI = true;
                if (!game.backgroundFadeInActive && !game.backgroundVisible) {
                    game.backgroundFadeInActive = true;
                    game.backgroundFadeClock.restart();
                }
            }
        } else {
            hideUI = true;
        }
    }

    if (hideUI)
        return;

    float t = game.uiGlowClock.getElapsedTime().asSeconds();
    float flicker = (std::sin(t * 25.f) + std::sin(t * 41.f)) * 0.25f;

    float alpha = 140.f + flicker * 30.f;

    sf::Color glowColor = ColorHelper::UI::PanelBlueLight;
    glowColor.a = static_cast<std::uint8_t>(std::clamp(alpha, 0.f, 255.f));
    glowColor = applyAlpha(glowColor, uiAlphaFactor);

    game.uiFrame.drawScaled(
        target,
        game.textBox.getPosition(),
        game.textBox.getSize(),
        glowColor,
        2.f
    );

    game.uiFrame.drawScaled(
        target,
        game.nameBox.getPosition(),
        game.nameBox.getSize(),
        glowColor,
        2.f
    );

    sf::Color frameColor = applyAlpha(ColorHelper::UI::PanelBlueDark, uiAlphaFactor);
    game.uiFrame.draw(target, game.nameBox, frameColor);
    game.uiFrame.draw(target, game.textBox, frameColor);

    drawLocationBox(game, target);

    if (!game.currentDialogue || game.dialogueIndex >= game.currentDialogue->size())
        return;

    const auto& line = (*game.currentDialogue)[game.dialogueIndex];

    std::string fullText = injectSpeakerNames(line.text, game);

    float delay = 0.02f;
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

    auto info = getSpeakerInfo(line.speaker);

    sf::Text nameText{game.resources.uiFont, "", kTextCharacterSize};

    if (!info.name.empty()) {
        nameText.setFillColor(applyAlpha(info.color, uiAlphaFactor));
        nameText.setString(info.name);

        auto namePos = game.nameBox.getPosition();
        nameText.setPosition({ namePos.x + kNameBoxTextOffset, namePos.y + kNameBoxTextOffset });
        target.draw(nameText);
    }

    auto textPos = game.textBox.getPosition();
    sf::Vector2f basePos{ textPos.x + kTextBoxPadding, textPos.y + kTextBoxPadding };

    std::string textToDraw = game.visibleText;
    if (game.askingName) {
        auto newlinePos = textToDraw.find('\n');
        if (newlinePos != std::string::npos)
            textToDraw = textToDraw.substr(0, newlinePos + 1);
    }

    auto segments = buildColoredSegments(textToDraw);
    float maxWidth = game.textBox.getSize().x - (kTextBoxPadding * 2.f);
    auto cursorPos = drawColoredSegments(target, game.resources.uiFont, segments, basePos, kTextCharacterSize, maxWidth, uiAlphaFactor);

    (void)cursorPos;

    if (game.askingName) {
        if (game.cursorBlinkClock.getElapsedTime().asSeconds() >= game.cursorBlinkInterval) {
            game.cursorVisible = !game.cursorVisible;
            game.cursorBlinkClock.restart();
        }

        sf::Vector2f inputPos{ textPos.x + kTextBoxPadding, textPos.y + kTextBoxPadding + 40.f };

        sf::Text inputText{game.resources.uiFont, "", kTextCharacterSize};
        inputText.setFillColor(applyAlpha(sf::Color::White, uiAlphaFactor));
        inputText.setString(game.nameInput);

        std::string nameWithCursor = game.nameInput;
        nameWithCursor.push_back(' ');
        inputText.setString(nameWithCursor);

        inputText.setPosition(inputPos);
        target.draw(inputText);

        if (game.cursorVisible) {
            sf::Text cursorText{game.resources.uiFont, "_", kTextCharacterSize};
            cursorText.setFillColor(applyAlpha(sf::Color::White, uiAlphaFactor));
            auto cursorDrawPos = inputText.findCharacterPos(game.nameInput.size());
            cursorText.setPosition(cursorDrawPos);
            target.draw(cursorText);
        }
    }

    if (!isTyping) {
        float returnPosWidth = game.textBox.getSize().x + 300.f;
        float returnPosHeight = game.textBox.getSize().y + 450.f;
        game.returnSprite.setPosition({ returnPosWidth, returnPosHeight });
        if (game.returnBlinkClock.getElapsedTime().asSeconds() >= game.returnBlinkInterval) {
            game.returnVisible = !game.returnVisible;
            game.returnBlinkClock.restart();
        }
        if (game.returnVisible) {
            sf::Color c = game.returnSprite.getColor();
            c.a = 0;
            game.returnSprite.setColor(c);
            c.a = static_cast<std::uint8_t>(255.f * uiAlphaFactor);
            game.returnSprite.setColor(c);
            target.draw(game.returnSprite);
        }
    }
}