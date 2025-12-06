#include "dialogDrawElements.hpp"
#include "dialogUI.hpp"
#include "uiEffects.hpp"
#include "rendering/textColorHelper.hpp"
#include "rendering/textLayout.hpp"
#include "story/textStyles.hpp"

namespace {
    constexpr float kNameBoxTextOffset = 20.f;
    constexpr float kTextBoxPadding = 20.f;
    constexpr unsigned int kTextCharacterSize = 28;
}

namespace dialogDraw {
    void drawDialogueFrames(
        Game& game
        , sf::RenderTarget& target
        , float uiAlphaFactor
        , float glowElapsedSeconds
    )
    {
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
            game.textBox.getPosition(),
            game.textBox.getSize(),
            glowColor,
            2.f
        );

        uiEffects::drawGlowFrame(
            target,
            game.uiFrame,
            game.nameBox.getPosition(),
            game.nameBox.getSize(),
            glowColor,
            2.f
        );

        uiEffects::drawGlowFrame(
            target,
            game.uiFrame,
            game.locationBox.getPosition(),
            game.locationBox.getSize(),
            glowColor,
            2.f
        );

        sf::Color frameColor = ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, uiAlphaFactor);
        game.uiFrame.draw(target, game.nameBox, frameColor);
        game.uiFrame.draw(target, game.textBox, frameColor);
        game.uiFrame.draw(target, game.locationBox, frameColor);
    }

    void drawSpeakerName(
        sf::RenderTarget& target
        , Game& game
        , const TextStyles::SpeakerStyle& info
        , float uiAlphaFactor
    )
    {
        sf::Text nameText{game.resources.uiFont, "", kTextCharacterSize};
        if (info.name.empty())
            return;

        nameText.setFillColor(ColorHelper::applyAlphaFactor(info.color, uiAlphaFactor));
        nameText.setString(info.name);

        auto namePos = game.nameBox.getPosition();
        nameText.setPosition({ namePos.x + kNameBoxTextOffset, namePos.y + kNameBoxTextOffset });
        target.draw(nameText);
    }

    void drawDialogueText(
        sf::RenderTarget& target
        , Game& game
        , const std::string& textToDraw
        , float uiAlphaFactor
    )
    {
        auto textPos = game.textBox.getPosition();
        sf::Vector2f basePos{ textPos.x + kTextBoxPadding, textPos.y + kTextBoxPadding };

        auto segments = buildColoredSegments(textToDraw);
        float maxWidth = game.textBox.getSize().x - (kTextBoxPadding * 2.f);
        auto cursorPos = drawColoredSegments(target, game.resources.uiFont, segments, basePos, kTextCharacterSize, maxWidth, uiAlphaFactor);

        (void)cursorPos;
    }

    void drawNameInput(
        sf::RenderTarget& target
        , Game& game
        , float uiAlphaFactor
    )
    {
        if (game.cursorBlinkClock.getElapsedTime().asSeconds() >= game.cursorBlinkInterval) {
            game.cursorVisible = !game.cursorVisible;
            game.cursorBlinkClock.restart();
        }

        auto textPos = game.textBox.getPosition();
        sf::Vector2f inputPos{ textPos.x +kTextBoxPadding, textPos.y + kTextBoxPadding + 40.f };

        sf::Text inputText{ game.resources.uiFont, "", kTextCharacterSize };
        inputText.setFillColor(ColorHelper::applyAlphaFactor(sf::Color::White, uiAlphaFactor));
        inputText.setString(game.nameInput);

        std::string nameWithCursor = game.nameInput;
        nameWithCursor.push_back(' ');
        inputText.setString(game.nameInput);

        inputText.setPosition(inputPos);
        target.draw(inputText);

        if (game.cursorVisible) {
            sf::Text cursorText{ game.resources.uiFont, "_", kTextCharacterSize };
            cursorText.setFillColor(ColorHelper::applyAlphaFactor(sf::Color::White, uiAlphaFactor));

            auto cursorDrawPos = inputText.findCharacterPos(game.nameInput.size());
            cursorText.setPosition(cursorDrawPos);
            target.draw(cursorText);
        }
    }

    void drawReturnPrompt(
        sf::RenderTarget& target
        , Game & game
        , float uiAlphaFactor
        , bool isTyping
    )
    {
        if (isTyping)
            return;

        float returnPosWidth = game.textBox.getSize().x + 300.f;
        float returnPosHeight = game.textBox.getSize().y + 450.f;
        game.returnSprite->setPosition({ returnPosWidth, returnPosHeight });
        if (game.returnBlinkClock.getElapsedTime().asSeconds() >= game.returnBlinkInterval) {
            game.returnVisible = !game.returnVisible;
            game.returnBlinkClock.restart();
        }
        if (game.returnVisible) {
            sf::Color c = game.returnSprite->getColor();
            c.a = 0;
            game.returnSprite->setColor(c);
            c.a = static_cast<uint8_t>(255.f * uiAlphaFactor);
            game.returnSprite->setColor(c);
            target.draw(*game.returnSprite);
        }
    }
}