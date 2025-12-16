// === C++ Libraries ===
#include <algorithm>  // Uses std::min and std::clamp when sizing portraits and layout helpers.
#include <vector>     // Builds temporary sets of texts/colors when drawing speaker names.
// === Header Files ===
#include "dialogDrawElements.hpp"  // Declares the draw helpers implemented in this file.
#include "dialogUI.hpp"            // Coordinates drawing order with the main dialogue UI.
#include "uiEffects.hpp"           // Computes glow colors for the dialogue frames.
#include "helper/textColorHelper.hpp"  // Breaks speaker names into colored segments when rendering.
#include "helper/colorHelper.hpp"  // Applies palette colors for outlines, text, and frames.
#include "rendering/textLayout.hpp"  // Lays out multi-line colored text segments inside boxes.
#include "story/textStyles.hpp"    // Retrieves speaker styles for portraits and name labels.

namespace {
    constexpr float kTextBoxPadding = 20.f;
    constexpr unsigned int kTextCharacterSize = 28;
    constexpr unsigned int kNameCharacterSize = kTextCharacterSize - 4;
    constexpr float kNameVerticalNudge = -10.f;
    constexpr float kPortraitPadding = 10.f;

    const sf::Texture* portraitForSpeaker(const Game& game, const std::string& speakerName) {
        using TextStyles::SpeakerId;

        switch (TextStyles::speakerFromName(speakerName)) {
            case SpeakerId::StoryTeller:
            case SpeakerId::NoNameNPC:
                return &game.resources.portraitStoryTeller;
            case SpeakerId::VillageElder:
                return &game.resources.portraitVillageElder;
            case SpeakerId::VillageWanderer:
                return &game.resources.portraitVillageWanderer;
            case SpeakerId::MasterBates:
                return &game.resources.portraitMasterBates;
            case SpeakerId::NoahBates:
                return &game.resources.portraitNoahBates;
            case SpeakerId::Player:
                if (game.playerGender == Game::DragonbornGender::Female)
                    return &game.resources.portraitDragonbornFemale;
                return &game.resources.portraitDragonbornMale;
            case SpeakerId::FireDragon:
                return &game.resources.portraitFireDragon;
            case SpeakerId::WaterDragon:
                return &game.resources.portraitWaterDragon;
            case SpeakerId::AirDragon:
                return &game.resources.portraitAirDragon;
            case SpeakerId::EarthDragon:
                return &game.resources.portraitEarthDragon;
            case SpeakerId::Unknown:
            default:
                return nullptr;
        }
    }

    void drawSpeakerPortrait(
        sf::RenderTarget& target,
        const sf::RectangleShape& nameBox,
        const sf::Text& nameText,
        const sf::Texture& texture,
        float uiAlphaFactor
    ) {
        // Compute the area above the speaker name inside the name box.
        auto boxPos = nameBox.getPosition();
        auto boxSize = nameBox.getSize();

        const auto textBounds = nameText.getLocalBounds();
        float nameBaseline = nameText.getPosition().y;
        float textOriginY = textBounds.position.y + textBounds.size.y;
        float textTop = nameBaseline - textOriginY;

        float portraitAreaTop = boxPos.y + kPortraitPadding;
        float portraitAreaBottom = textTop - kPortraitPadding;

        float portraitAreaHeight = portraitAreaBottom - portraitAreaTop;
        float portraitAreaWidth = boxSize.x - (kPortraitPadding * 2.f);

        if (portraitAreaWidth <= 0.f || portraitAreaHeight <= 0.f)
            return;

        sf::Sprite portrait{ texture };
        auto texSize = texture.getSize();
        float scaleX = portraitAreaWidth / static_cast<float>(texSize.x);
        float scaleY = portraitAreaHeight / static_cast<float>(texSize.y);
        float scale = std::min(scaleX, scaleY);
        portrait.setScale({ scale, scale });

        auto localBounds = portrait.getLocalBounds();
        portrait.setOrigin({
            localBounds.position.x + (localBounds.size.x / 2.f),
            localBounds.position.y + (localBounds.size.y / 2.f)
        });

        float centerX = boxPos.x + (boxSize.x / 2.f);
        float centerY = portraitAreaTop + (portraitAreaHeight / 2.f);
        portrait.setPosition({ centerX, centerY });

        sf::Color color = portrait.getColor();
        color.a = static_cast<std::int8_t>(255.f * uiAlphaFactor);
        portrait.setColor(color);

        target.draw(portrait);
    }
}

namespace dialogDraw {
    void drawDialogueFrames(
        Game& game
        , sf::RenderTarget& target
        , float uiAlphaFactor
        , const sf::Color& glowColor
    )
    {
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

        sf::Color frameColor = game.frameColor(uiAlphaFactor);
        game.uiFrame.draw(target, game.nameBox, frameColor);
        game.uiFrame.draw(target, game.textBox, frameColor);
    }

    void drawSpeakerName(
        sf::RenderTarget& target
        , Game& game
        , const TextStyles::SpeakerStyle& info
        , float uiAlphaFactor
    )
    {
        sf::Text nameText{game.resources.uiFont, "", kNameCharacterSize};
        if (info.name.empty())
            return;

        auto drawNameParts = [&](const std::vector<std::pair<std::string, sf::Color>>& parts, float baseY) {
            std::vector<sf::Text> texts;
            texts.reserve(parts.size());
            float totalWidth = 0.f;
            for (const auto& part : parts) {
                sf::Text t{ game.resources.uiFont, part.first, kNameCharacterSize };
                t.setFillColor(ColorHelper::applyAlphaFactor(part.second, uiAlphaFactor));
                auto b = t.getLocalBounds();
                totalWidth += b.size.x;
                texts.push_back(std::move(t));
            }

            auto namePos = game.nameBox.getPosition();
            auto nameSize = game.nameBox.getSize();
            float marginY = nameSize.y * 0.05f;
            float x = namePos.x + (nameSize.x / 2.f) - (totalWidth / 2.f);
            float y = baseY + kNameVerticalNudge;

            for (auto& t : texts) {
                auto b = t.getLocalBounds();
                t.setOrigin({ b.position.x, b.position.y + b.size.y });
                t.setPosition({ x, y });
                target.draw(t);
                x += b.size.x;
            }
        };

        nameText.setFillColor(ColorHelper::applyAlphaFactor(info.color, uiAlphaFactor));
        nameText.setString(info.name);

        auto namePos = game.nameBox.getPosition();
        auto nameSize = game.nameBox.getSize();
        auto bounds = nameText.getLocalBounds();

        // Bottom-center the name with a 5% vertical inset from the lower edge
        float marginY = nameSize.y * 0.05f;
        nameText.setOrigin({ bounds.position.x + (bounds.size.x / 2.f), bounds.position.y + bounds.size.y });
        float x = namePos.x + (nameSize.x / 2.f);
        float baseY = namePos.y + nameSize.y - marginY;
        nameText.setPosition({ x, baseY });

        // Draw portrait using the un-nudged name position so its area stays stable.
        if (const sf::Texture* portraitTex = portraitForSpeaker(game, info.name)) {
            drawSpeakerPortrait(target, game.nameBox, nameText, *portraitTex, uiAlphaFactor);
        }

        // Apply visual nudge to bring the name closer to the portrait.
        nameText.setPosition({ x, baseY + kNameVerticalNudge });


        if (info.name == "Noah Bates") {
            drawNameParts({
                { "Noah ", ColorHelper::Palette::PurpleBlue },
                { "Bates", ColorHelper::Palette::DarkPurple }
            }, baseY);
        }
        else {
            target.draw(nameText);
        }
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
        inputText.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, uiAlphaFactor));
        inputText.setString(game.nameInput);

        std::string nameWithCursor = game.nameInput;
        nameWithCursor.push_back(' ');
        inputText.setString(game.nameInput);

        inputText.setPosition(inputPos);
        target.draw(inputText);

        if (game.cursorVisible) {
            sf::Text cursorText{ game.resources.uiFont, "_", kTextCharacterSize };
            cursorText.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, uiAlphaFactor));

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

    void drawBoxHeader(
        Game& game,
        sf::RenderTarget& target,
        const sf::RectangleShape& box,
        const std::string& label,
        float uiAlphaFactor
    )
    {
        sf::Text title{ game.resources.uiFont, label, 18 };
        title.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, uiAlphaFactor));

        auto bounds = title.getLocalBounds();
        title.setOrigin({
            bounds.position.x + (bounds.size.x / 2.f),
            bounds.position.y + bounds.size.y
        });

        auto pos = box.getPosition();
        auto size = box.getSize();
        title.setPosition({ pos.x + (size.x / 2.f), pos.y - 6.f });

        target.draw(title);
    }

    void drawItemIcons(Game& game, sf::RenderTarget& target, float uiAlphaFactor) {
        for (auto& item : game.itemController.icons()) {
            sf::Color color = item.sprite.getColor();
            color.a = static_cast<std::uint8_t>(255.f * uiAlphaFactor);
            item.sprite.setColor(color);
            target.draw(item.sprite);
        }
    }
}
