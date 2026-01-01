// === C++ Libraries ===
#include <algorithm>  // Uses std::min and std::clamp when sizing portraits and layout helpers.
#include <cstdint>    // Needed for std::uint8_t when tinting sprites.
#include <vector>     // Builds temporary sets of texts/colors when drawing speaker names.
// === Header Files ===
#include "dialogDrawElements.hpp"  // Declares the draw helpers implemented in this file.
#include "dialogUI.hpp"            // Coordinates drawing order with the main dialogue UI.
#include "uiEffects.hpp"           // Computes glow colors for the dialogue frames.
#include "helper/textColorHelper.hpp"  // Breaks speaker names into colored segments when rendering.
#include "helper/colorHelper.hpp"  // Applies palette colors for outlines, text, and frames.
#include "rendering/textLayout.hpp"  // Lays out multi-line colored text segments inside boxes.
#include "rendering/locations.hpp" // Needed for LocationId definitions used by portrait backgrounds.
#include "story/textStyles.hpp"    // Retrieves speaker styles for portraits and name labels.

namespace {
    constexpr float kTextBoxPadding = 20.f;
    constexpr unsigned int kTextCharacterSize = 28;
    constexpr unsigned int kNameCharacterSize = kTextCharacterSize - 4;
    constexpr float kSpeakerNameBottomOffset = 10.f;
    constexpr float kPortraitRenderScale = 1.23f;
    constexpr float kPortraitBackgroundScale = 1.29f;
    constexpr float kPortraitSpriteOffset = 3.f;
    constexpr float kDialogueLineSpacingMultiplier = 1.2f;

    const sf::Texture* siblingPortrait(const Game& game) {
        bool useCape = game.cloakEquipped;
        auto siblingGender = (game.playerGender == Game::DragonbornGender::Female)
            ? Game::DragonbornGender::Male
            : Game::DragonbornGender::Female;
        if (siblingGender == Game::DragonbornGender::Female) {
            return useCape
                ? &game.resources.portraitDragonbornFemaleCape
                : &game.resources.portraitDragonbornFemaleNoCape;
        }
        return useCape
            ? &game.resources.portraitDragonbornMaleCape
            : &game.resources.portraitDragonbornMaleNoCape;
    }

    const sf::Texture* portraitForSpeaker(const Game& game, const std::string& speakerName) {
        using TextStyles::SpeakerId;

        switch (TextStyles::speakerFromName(speakerName)) {
            case SpeakerId::StoryTeller:
            case SpeakerId::NoNameNPC:
                return &game.resources.portraitStoryTeller;
            case SpeakerId::NoNameWanderer:
                return &game.resources.portraitVillageWanderer;
            case SpeakerId::VillageElder:
                return &game.resources.portraitVillageElder;
            case SpeakerId::VillageWanderer:
                return &game.resources.portraitVillageWanderer;
            case SpeakerId::Blacksmith:
                return &game.resources.portraitBlacksmith;
            case SpeakerId::MasterBates:
                return &game.resources.portraitMasterBates;
            case SpeakerId::NoahBates:
                return &game.resources.portraitNoahBates;
        case SpeakerId::Player:
            if (game.playerGender == Game::DragonbornGender::Female)
                return game.cloakEquipped
                    ? &game.resources.portraitDragonbornFemaleCape
                    : &game.resources.portraitDragonbornFemaleNoCape;
            return game.cloakEquipped
                ? &game.resources.portraitDragonbornMaleCape
                : &game.resources.portraitDragonbornMaleNoCape;
            case SpeakerId::FireDragon:
                return &game.resources.portraitFireDragon;
            case SpeakerId::WaterDragon:
                return &game.resources.portraitWaterDragon;
            case SpeakerId::AirDragon:
                return &game.resources.portraitAirDragon;
            case SpeakerId::EarthDragon:
                return &game.resources.portraitEarthDragon;
            case SpeakerId::DragonbornSibling:
                return siblingPortrait(game);
            case SpeakerId::Unknown:
            default:
                return nullptr;
        }
    }

    const sf::Texture* portraitBackgroundForLocation(const Game& game) {
        if (!game.currentLocation)
            return &game.resources.portraitBackgroundToryTailor;

        switch (game.currentLocation->id) {
            case LocationId::Perigonal: return &game.resources.portraitBackgroundPetrigonal;
            case LocationId::Gonad: return &game.resources.portraitBackgroundGonad;
            case LocationId::FigsidsForge: return &game.resources.portraitBackgroundBlacksmith;
            case LocationId::Blyathyroid: return &game.resources.portraitBackgroundBlyathyroid;
            case LocationId::Lacrimere: return &game.resources.portraitBackgroundLacrimere;
            case LocationId::Cladrenal: return &game.resources.portraitBackgroundCladrenal;
            case LocationId::Aerobronchi: return &game.resources.portraitBackgroundAerobronchi;
            case LocationId::Seminiferous: return &game.resources.portraitBackgroundSeminiferous;
            case LocationId::UmbraOssea: return &game.resources.portraitBackgroundUmbraOssea;
        }

        return &game.resources.portraitBackgroundToryTailor;
    }

    void drawSpeakerPortrait(
        sf::RenderTarget& target,
        const Game& game,
        const sf::RectangleShape& nameBox,
        const sf::Texture& texture,
        TextStyles::SpeakerId speakerId,
        float uiAlphaFactor
    ) {
        auto boxPos = nameBox.getPosition();
        auto boxSize = nameBox.getSize();

        sf::Vector2f areaSize{ boxSize.x * 0.5f, boxSize.y * 0.5f };
        if (areaSize.x <= 0.f || areaSize.y <= 0.f)
            return;

        sf::Vector2f areaCenter{
            boxPos.x + (boxSize.x * 0.5f),
            boxPos.y + (boxSize.y * 0.5f)
        };

        sf::FloatRect portraitArea(
            sf::Vector2f{
                areaCenter.x - (areaSize.x * 0.5f),
                areaCenter.y - (areaSize.y * 0.5f)
            },
            areaSize
        );

        auto drawTextureInArea = [&](const sf::Texture& tex, float scaleFactor, bool fill, float verticalOffset) {
            auto texSize = tex.getSize();
            if (texSize.x == 0 || texSize.y == 0)
                return;

            sf::Sprite sprite{ tex };
            float targetWidth = portraitArea.size.x * scaleFactor;
            float targetHeight = portraitArea.size.y * scaleFactor;
            float scaleX = targetWidth / static_cast<float>(texSize.x);
            float scaleY = targetHeight / static_cast<float>(texSize.y);
            float scale = fill ? std::max(scaleX, scaleY) : std::min(scaleX, scaleY);
            sprite.setScale({ scale, scale });

            auto localBounds = sprite.getLocalBounds();
            sprite.setOrigin({
                localBounds.position.x + (localBounds.size.x * 0.5f),
                localBounds.position.y + (localBounds.size.y * 0.5f)
            });

            sf::Vector2f drawCenter = areaCenter;
            drawCenter.y += verticalOffset;
            sprite.setPosition(drawCenter);

            sf::Color color = sprite.getColor();
            color.a = static_cast<std::uint8_t>(std::clamp(uiAlphaFactor, 0.f, 1.f) * 255.f);
            sprite.setColor(color);

            target.draw(sprite);
        };

        const sf::Texture* background = (speakerId == TextStyles::SpeakerId::StoryTeller)
            ? &game.resources.portraitBackgroundToryTailor
            : portraitBackgroundForLocation(game);
        if (background) {
            drawTextureInArea(*background, kPortraitBackgroundScale, false, 0.f);
        }
        drawTextureInArea(texture, kPortraitRenderScale, false, kPortraitSpriteOffset);
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

        auto drawBoxFill = [&](const sf::RectangleShape& box) {
            sf::RectangleShape fill = box;
            fill.setFillColor(ColorHelper::applyAlphaFactor(box.getFillColor(), uiAlphaFactor));
            fill.setOutlineThickness(0.f);
            target.draw(fill);
        };
        drawBoxFill(game.nameBox);
        drawBoxFill(game.textBox);

        sf::Color frameColor = game.frameColor(uiAlphaFactor);
        game.uiFrame.draw(target, game.nameBox, frameColor);
        game.uiFrame.draw(target, game.textBox, frameColor);
    }

    void drawSpeakerName(
        sf::RenderTarget& target
        , Game& game
        , const TextStyles::SpeakerStyle& info
        , TextStyles::SpeakerId speakerId
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
                if (part.second == ColorHelper::Palette::DarkPurple) {
                    t.setOutlineColor(sf::Color::White);
                    t.setOutlineThickness(2.f);
                } else {
                    t.setOutlineThickness(0.f);
                    t.setOutlineColor(sf::Color::Transparent);
                }
                auto b = t.getLocalBounds();
                totalWidth += b.size.x;
                texts.push_back(std::move(t));
            }

            auto namePos = game.nameBox.getPosition();
            auto nameSize = game.nameBox.getSize();
            float x = namePos.x + (nameSize.x / 2.f) - (totalWidth / 2.f);
            float y = baseY;

            for (auto& t : texts) {
                auto b = t.getLocalBounds();
                t.setOrigin({ b.position.x, b.position.y + b.size.y });
                t.setPosition({ x, y });
                target.draw(t);
                x += b.size.x;
            }
        };

        nameText.setFillColor(ColorHelper::applyAlphaFactor(info.color, uiAlphaFactor));
        if (info.color == ColorHelper::Palette::DarkPurple) {
            nameText.setOutlineColor(sf::Color::White);
            nameText.setOutlineThickness(2.f);
        } else {
            nameText.setOutlineThickness(0.f);
            nameText.setOutlineColor(sf::Color::Transparent);
        }
        nameText.setString(info.name);

        auto namePos = game.nameBox.getPosition();
        auto nameSize = game.nameBox.getSize();
        auto bounds = nameText.getLocalBounds();

        nameText.setOrigin({ bounds.position.x + (bounds.size.x / 2.f), bounds.position.y + bounds.size.y });
        float x = namePos.x + (nameSize.x / 2.f);
        float baseY = namePos.y + nameSize.y - kSpeakerNameBottomOffset;
        nameText.setPosition({ x, baseY });

        if (const sf::Texture* portraitTex = portraitForSpeaker(game, info.name)) {
            drawSpeakerPortrait(target, game, game.nameBox, *portraitTex, speakerId, uiAlphaFactor);
        }


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
        auto cursorPos = drawColoredSegments(
            target,
            game.resources.uiFont,
            segments,
            basePos,
            kTextCharacterSize,
            maxWidth,
            uiAlphaFactor,
            kDialogueLineSpacingMultiplier
        );

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
