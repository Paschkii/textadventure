// === C++ Libraries ===
#include <algorithm>  // Uses clamp/min helpers later in this file.
#include <array>      // Stores the fixed-size tab lists.
#include <cmath>      // Rounds time values when formatting durations.
#include <cstdint>    // Provides uint8_t used when tinting sprites/colors.
#include <iomanip>    // Formats elapsed time for rankings.
#include <sstream>    // Builds ranking/time strings.
#include <string>     // Needed for the tutorial text.
#include <vector>     // Holds segments for the tutorial popup text.
#include <optional>   // Tracks optional popup assets.

// === SFML Libraries ===
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

// === Header Files ===
#include "core/game.hpp"
#include "core/ranking.hpp"
#include "helper/colorHelper.hpp"
#include "helper/textColorHelper.hpp"
#include "rendering/textLayout.hpp"
#include "story/dialogInput.hpp"
#include "story/textStyles.hpp"
#include "ui/mapSelectionUI.hpp"
#include "ui/menuUI.hpp"
#include "ui/popupStyle.hpp"

namespace ui::menu {

namespace {
    constexpr std::array<const char*, 6> kTabLabels = {
        "Inventory",
        "Skills",
        "Map",
        "Quests",
        "Settings",
        "Rankings"
    };

    enum class MenuTab {
        Inventory = 0,
        Skills,
        Map,
        Quests,
        Settings,
        Rankings,
    };

    std::string formatTime(double totalSeconds) {
        int seconds = static_cast<int>(std::round(totalSeconds));
        int minutes = seconds / 60;
        seconds %= 60;
        std::ostringstream out;
        out << minutes << ':';
        out << std::setfill('0') << std::setw(2) << seconds;
        return out.str();
    }

    constexpr std::array<const char*, 2> kInventoryTutorialMessages = {
        "This is your inventory. Wanda will store all your belongings in here.",
        "If you ever need to see what items you possess or what they do, you can simply open the Menu by pressing ESC or clicking on the Menu Symbol."
    };
    constexpr float kMenuButtonFadeDuration = 1.f;
    constexpr float kInventoryTutorialButtonWidth = 140.f;
    constexpr float kInventoryTutorialButtonHeight = 36.f;
    constexpr float kInventoryTutorialCloseDuration = 1.f;
    constexpr float kMapTutorialButtonWidth = 140.f;
    constexpr float kMapTutorialButtonHeight = 44.f;
    constexpr float kMapTutorialButtonPadding = 12.f;
    constexpr unsigned int kMapTutorialTextSize = 20;
    constexpr float kMapTutorialLineSpacing = 1.f;

    void beginInventoryTutorial(Game& game) {
        if (!game.inventoryTutorialPending)
            return;
        game.inventoryArrowActive = false;
        game.inventoryTutorialPending = false;
        game.inventoryTutorialPopupActive = true;
        game.inventoryTutorialButtonHovered = false;
        game.inventoryTutorialAdvancePending = false;
    }

    void drawTutorialButton(
        Game& game,
        sf::RenderTarget& target,
        const sf::FloatRect& bounds,
        bool hovered,
        float alphaFactor
        , const std::string& label = "Ok"
    ) {
        sf::Color baseColor = TextStyles::UI::PanelDark;
        sf::Color outlineColor = ColorHelper::Palette::Amber;
        if (hovered)
            baseColor = ColorHelper::Palette::Green;
        baseColor = ColorHelper::applyAlphaFactor(baseColor, alphaFactor * (hovered ? 0.9f : 0.7f));
        outlineColor = ColorHelper::applyAlphaFactor(outlineColor, alphaFactor);

        RoundedRectangleShape buttonShape({ bounds.size.x, bounds.size.y }, bounds.size.y * 0.5f, 20);
        buttonShape.setPosition({ bounds.position.x, bounds.position.y });
        buttonShape.setFillColor(baseColor);
        buttonShape.setOutlineThickness(2.f);
        buttonShape.setOutlineColor(outlineColor);
        target.draw(buttonShape);

        sf::Text text{ game.resources.uiFont, label, 22 };
        text.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, alphaFactor));
        auto textBounds = text.getLocalBounds();
        text.setOrigin({
            textBounds.position.x + textBounds.size.x * 0.5f,
            textBounds.position.y + textBounds.size.y * 0.5f
        });
        text.setPosition({
            bounds.position.x + bounds.size.x * 0.5f,
            bounds.position.y + bounds.size.y * 0.5f
        });
        target.draw(text);
    }

    void drawMapTutorialPopup(Game& game, sf::RenderTarget& target, const sf::FloatRect& panelBounds, float menuFadeFactor) {
        if (!game.mapTutorialActive || game.menuActiveTab != static_cast<int>(MenuTab::Map)) {
            game.mapTutorialPopupBounds = {};
            game.mapTutorialOkBounds = {};
            return;
        }

        if (game.currentProcessedLine.empty()
            && game.currentDialogue
            && game.dialogueIndex < game.currentDialogue->size())
        {
            game.currentProcessedLine = injectSpeakerNames(
                (*game.currentDialogue)[game.dialogueIndex].text,
                game
            );
        }

        std::optional<LocationId> highlight = game.mapTutorialHighlight;
        const sf::Texture* dragonTexture = nullptr;
        if (highlight) {
            switch (*highlight) {
                case LocationId::Aerobronchi:
                    dragonTexture = &game.resources.airDragon;
                    break;
                case LocationId::Blyathyroid:
                    dragonTexture = &game.resources.fireDragon;
                    break;
                case LocationId::Cladrenal:
                    dragonTexture = &game.resources.earthDragon;
                    break;
                case LocationId::Lacrimere:
                    dragonTexture = &game.resources.waterDragon;
                    break;
                default:
                    break;
            }
        }

        float popupWidth = std::clamp(panelBounds.size.x * 0.46f, 320.f, 480.f);
        float popupHeight = std::clamp(panelBounds.size.y * 0.28f, 160.f, 240.f);
        float anchorX = panelBounds.position.x + (panelBounds.size.x * game.mapTutorialAnchorNormalized.x);
        float anchorY = panelBounds.position.y + (panelBounds.size.y * game.mapTutorialAnchorNormalized.y);
        float popupX = anchorX - popupWidth * 0.5f;
        float popupY = anchorY - popupHeight * 0.5f;
        popupX = std::clamp(popupX, panelBounds.position.x + 12.f, panelBounds.position.x + panelBounds.size.x - popupWidth - 12.f);
        popupY = std::clamp(popupY, panelBounds.position.y + 12.f, panelBounds.position.y + panelBounds.size.y - popupHeight - 12.f);
        game.mapTutorialPopupBounds = { { popupX, popupY }, { popupWidth, popupHeight } };

        RoundedRectangleShape popupShape({ popupWidth, popupHeight }, 18.f, 20);
        popupShape.setPosition({ popupX, popupY });
        popupShape.setFillColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, menuFadeFactor));
        popupShape.setOutlineThickness(2.f);
        popupShape.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::FrameGoldLight, menuFadeFactor));
        target.draw(popupShape);

        sf::Text header{ game.resources.uiFont, "Tory Tailor", 24 };
        header.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, menuFadeFactor));
        header.setPosition({ popupX + 18.f, popupY + 18.f });
        target.draw(header);

        std::optional<sf::Sprite> dragonSprite;
        float dragonSpriteWidth = 0.f;
        float dragonSpriteHeight = 0.f;
        if (dragonTexture && dragonTexture->getSize().x > 0 && dragonTexture->getSize().y > 0) {
            dragonSprite.emplace(*dragonTexture);
            float maxDragonWidth = std::clamp(popupWidth * 0.32f, 64.f, popupWidth * 0.5f);
            float maxDragonHeight = std::clamp(popupHeight * 0.4f, 64.f, popupHeight * 0.6f);
            float texWidth = static_cast<float>(dragonTexture->getSize().x);
            float texHeight = static_cast<float>(dragonTexture->getSize().y);
            float scale = std::min(maxDragonWidth / texWidth, maxDragonHeight / texHeight);
            if (scale > 0.f) {
                dragonSpriteWidth = texWidth * scale;
                dragonSpriteHeight = texHeight * scale;
                dragonSprite->setScale({ scale, scale });
            }
        }

        float textX = popupX + 18.f;
        float textY = popupY + 42.f;
        float textRightPadding = dragonSpriteWidth > 0.f ? (dragonSpriteWidth + 12.f) : 0.f;
        float maxTextWidth = std::max(0.f, popupWidth - 36.f - textRightPadding);
        auto segments = buildColoredSegments(game.currentProcessedLine);
        drawColoredSegments(
            target,
            game.resources.uiFont,
            segments,
            { textX, textY },
            kMapTutorialTextSize,
            maxTextWidth,
            menuFadeFactor,
            kMapTutorialLineSpacing
        );

        float buttonX = popupX + popupWidth - kMapTutorialButtonPadding - kMapTutorialButtonWidth;
        float buttonY = popupY + popupHeight - kMapTutorialButtonPadding - kMapTutorialButtonHeight;

        if (dragonSprite && dragonSpriteWidth > 0.f && dragonSpriteHeight > 0.f) {
            float spriteX = buttonX + (kMapTutorialButtonWidth * 0.5f);
            float spriteY = buttonY - (kMapTutorialButtonHeight + dragonSpriteHeight) * 0.5f;
            float minSpriteY = popupY + 24.f + (dragonSpriteHeight * 0.5f);
            spriteY = std::max(spriteY, minSpriteY);
            auto dragonBounds = dragonSprite->getLocalBounds();
            dragonSprite->setOrigin({
                dragonBounds.size.x * 0.5f,
                dragonBounds.size.y * 0.5f
            });
            dragonSprite->setPosition({ spriteX, spriteY });
            dragonSprite->setColor(ColorHelper::applyAlphaFactor(sf::Color::White, menuFadeFactor));
            target.draw(*dragonSprite);
        }
        game.mapTutorialOkBounds = { { buttonX, buttonY }, { kMapTutorialButtonWidth, kMapTutorialButtonHeight } };
        drawTutorialButton(game, target, game.mapTutorialOkBounds, game.mapTutorialOkHovered, menuFadeFactor);
    }

    float drawInventoryContent(Game& game, sf::RenderTarget& target, const sf::FloatRect& bounds, float alphaFactor) {
        sf::RectangleShape contentBox;
        contentBox.setPosition({ bounds.position.x + 12.f, bounds.position.y + 12.f });
        contentBox.setSize({ bounds.size.x - 24.f, bounds.size.y - 24.f });
        auto applyAlpha = [&](const sf::Color& color) {
            return ColorHelper::applyAlphaFactor(color, alphaFactor);
        };
        contentBox.setFillColor(applyAlpha(sf::Color::Transparent));
        contentBox.setOutlineColor(applyAlpha(sf::Color(255, 255, 255, 120)));
        contentBox.setOutlineThickness(1.5f);
        target.draw(contentBox);

        constexpr float verticalPadding = 30.f;
        constexpr float horizontalPadding = 28.f;
        constexpr float columnSpacing = 32.f;
        constexpr float sectionCornerRadius = 18.f;
        float columnTop = bounds.position.y + verticalPadding;
        float columnBottom = bounds.position.y + bounds.size.y - verticalPadding;
        float columnHeight = std::max(0.f, columnBottom - columnTop);
        float availableWidth = bounds.size.x - (horizontalPadding * 2.f) - columnSpacing;
        float leftColumnWidth = std::max(200.f, availableWidth * 0.65f);
        float rightColumnWidth = availableWidth - leftColumnWidth;
        if (rightColumnWidth < 200.f) {
            rightColumnWidth = 200.f;
            leftColumnWidth = std::max(200.f, availableWidth - rightColumnWidth);
        }

        float leftColumnX = bounds.position.x + horizontalPadding;
        float rightColumnX = leftColumnX + leftColumnWidth + columnSpacing;

        const sf::Color sectionColor(205, 193, 160, 210);
        RoundedRectangleShape leftSection({ leftColumnWidth, columnHeight }, sectionCornerRadius, 12);
        leftSection.setPosition({ leftColumnX, columnTop });
        leftSection.setFillColor(applyAlpha(sectionColor));
        target.draw(leftSection);

        RoundedRectangleShape rightSection({ rightColumnWidth, columnHeight }, sectionCornerRadius, 12);
        rightSection.setPosition({ rightColumnX, columnTop });
        rightSection.setFillColor(applyAlpha(sectionColor));
        target.draw(rightSection);

        constexpr float kHeaderFontSize = 34.f;
        constexpr float kHeaderIconHeight = 32.f;
        constexpr float kHeaderIconSpacing = 10.f;
        auto drawSectionHeader = [&](const sf::Texture* icon, const std::string& title, float x, float y, bool centered = false) {
            sf::Text header{ game.resources.uiFont, title, static_cast<unsigned int>(kHeaderFontSize) };
            header.setFillColor(applyAlpha(ColorHelper::Palette::SoftYellow));
            float iconWidth = 0.f;
            float iconHeight = 0.f;
            std::optional<sf::Sprite> headerSprite;
            if (icon && icon->getSize().x > 0 && icon->getSize().y > 0) {
                headerSprite.emplace(*icon);
                float scale = kHeaderIconHeight / static_cast<float>(icon->getSize().y);
                headerSprite->setScale({ scale, scale });
                iconWidth = static_cast<float>(icon->getSize().x) * scale;
                iconHeight = static_cast<float>(icon->getSize().y) * scale;
            }
            float baseX = x;
            if (centered) {
                float totalWidth = header.getLocalBounds().size.x + (iconWidth > 0.f ? iconWidth + kHeaderIconSpacing : 0.f);
                baseX = x - (totalWidth * 0.5f);
            }
            float textX = baseX + (iconWidth > 0.f ? iconWidth + kHeaderIconSpacing : 0.f);
            header.setPosition({ textX, y });
            auto headerGlobalBounds = header.getGlobalBounds();
            float headerHeight = headerGlobalBounds.size.y;
            if (headerHeight <= 0.f)
                headerHeight = kHeaderFontSize;
            float textCenterY = headerGlobalBounds.position.y + (headerHeight * 0.5f);
            if (headerSprite) {
                float iconY = textCenterY - (iconHeight * 0.5f);
                headerSprite->setPosition({ baseX, iconY });
                headerSprite->setColor(applyAlpha(sf::Color::White));
                target.draw(*headerSprite);
            }
            target.draw(header);
            return headerHeight;
        };

        float inventoryHeaderY = columnTop + 18.f;
        float inventoryHeaderHeight = drawSectionHeader(
            &game.resources.buttonInventory,
            "Inventory",
            leftColumnX + (leftColumnWidth * 0.5f),
            inventoryHeaderY,
            true
        );

        sf::Text info{ game.resources.uiFont, "", 18 };
        info.setFillColor(applyAlpha(ColorHelper::Palette::Dim));
        info.setLineSpacing(1.2f);
        info.setPosition({ leftColumnX + 16.f, inventoryHeaderY + inventoryHeaderHeight + 10.f });
        target.draw(info);

        auto& icons = game.itemController.icons();
        if (icons.empty()) {
            sf::Text empty{ game.resources.uiFont, "Your inventory is empty.", 20 };
            empty.setFillColor(applyAlpha(ColorHelper::Palette::Dim));
            auto emptyBounds = empty.getLocalBounds();
            empty.setOrigin({
                emptyBounds.position.x + emptyBounds.size.x * 0.5f,
                emptyBounds.position.y + emptyBounds.size.y * 0.5f
            });
            empty.setPosition({
                leftColumnX + (leftColumnWidth * 0.5f),
                columnTop + columnHeight * 0.5f
            });
            target.draw(empty);
        }

        constexpr float iconSize = 72.f;
        constexpr float iconSpacing = 28.f;
        constexpr int columns = 4;
        constexpr float slotCornerRadius = 12.f;
        const sf::Color slotBackground(18, 10, 6, 230);

        std::size_t rows = std::max<std::size_t>(3, icons.empty() ? 0 : (icons.size() + columns - 1) / columns);
        float gridHeight = rows * iconSize + (rows - 1) * iconSpacing;
        float gridWidth = columns * iconSize + (columns - 1) * iconSpacing;
        float gridStartX = leftColumnX + (leftColumnWidth - gridWidth) * 0.5f;
        float startGridX = std::max(leftColumnX + 16.f, gridStartX);
        float infoY = info.getPosition().y;
        float gridStartY = infoY + info.getLocalBounds().size.y + 24.f;
        float gridBottomLimit = columnTop + columnHeight - 20.f - gridHeight;
        float startGridY = std::min(gridStartY, gridBottomLimit);
        startGridY = std::max(startGridY, infoY);

        const float gridOutlinePad = 8.f;
        RoundedRectangleShape gridOutline({ gridWidth + gridOutlinePad * 2.f, gridHeight + gridOutlinePad * 2.f }, 16.f, 16);
        gridOutline.setPosition({ startGridX - gridOutlinePad, startGridY - gridOutlinePad });
        gridOutline.setFillColor(applyAlpha(ColorHelper::Palette::DarkBrown));
        gridOutline.setOutlineColor(applyAlpha(TextStyles::UI::PanelDark));
        gridOutline.setOutlineThickness(2.f);
        target.draw(gridOutline);

        for (std::size_t row = 0; row < rows; ++row) {
            for (std::size_t column = 0; column < columns; ++column) {
                float cellX = startGridX + static_cast<float>(column) * (iconSize + iconSpacing);
                float cellY = startGridY + static_cast<float>(row) * (iconSize + iconSpacing);
                RoundedRectangleShape cell({ iconSize, iconSize }, slotCornerRadius, 8);
                cell.setPosition({ cellX, cellY });
                cell.setFillColor(applyAlpha(slotBackground));
                cell.setOutlineThickness(0.f);
                target.draw(cell);
            }
        }

        constexpr float kDividerThickness = 2.f;
        const sf::Color dividerColor(34, 22, 12, 220);
        float adjustedGridWidth = gridWidth;
        for (int divider = 1; divider < columns; ++divider) {
            float x = startGridX
                + static_cast<float>(divider) * iconSize
                + static_cast<float>(divider - 1) * iconSpacing
                + (iconSpacing * 0.5f);
            sf::RectangleShape line({ kDividerThickness, gridHeight });
            line.setPosition({ x - (kDividerThickness * 0.5f), startGridY });
            line.setFillColor(applyAlpha(dividerColor));
            target.draw(line);
        }
        for (std::size_t divider = 1; divider < rows; ++divider) {
            float y = startGridY
                + static_cast<float>(divider) * iconSize
                + static_cast<float>(divider - 1) * iconSpacing
                + (iconSpacing * 0.5f);
            sf::RectangleShape line({ adjustedGridWidth, kDividerThickness });
            line.setPosition({ startGridX, y - (kDividerThickness * 0.5f) });
            line.setFillColor(applyAlpha(dividerColor));
            target.draw(line);
        }

        for (std::size_t idx = 0; idx < icons.size(); ++idx) {
            std::size_t column = idx % columns;
            std::size_t row = idx / columns;

            sf::Sprite sprite = icons[idx].sprite;
            auto texRect = sprite.getTextureRect();
            if (texRect.size.y > 0) {
                float scale = iconSize / static_cast<float>(texRect.size.y);
                sprite.setScale({ scale, scale });
            }

            auto spriteBounds = sprite.getLocalBounds();
            sprite.setOrigin({
                spriteBounds.position.x + spriteBounds.size.x * 0.5f,
                spriteBounds.position.y + spriteBounds.size.y * 0.5f
            });
            float posX = startGridX + static_cast<float>(column) * (iconSize + iconSpacing);
            float posY = startGridY + static_cast<float>(row) * (iconSize + iconSpacing);
            sprite.setPosition({ posX + iconSize * 0.5f, posY + iconSize * 0.5f });

            sf::Color tint = sprite.getColor();
            tint.a = static_cast<std::uint8_t>(tint.a * 0.9f);
            sprite.setColor(applyAlpha(tint));
            target.draw(sprite);
        }

        float characterHeaderY = columnTop + 18.f;
        float characterHeaderHeight = drawSectionHeader(
            &game.resources.buttonCharacter,
            "Character",
            rightColumnX + (rightColumnWidth * 0.5f),
            characterHeaderY,
            true
        );
        float charBoxY = startGridY;
        float charBoxHeight = std::min(gridHeight, columnBottom - charBoxY - 10.f);
        if (charBoxHeight < 0.f)
            charBoxHeight = 0.f;
        RoundedRectangleShape characterBox({ rightColumnWidth - 20.f, charBoxHeight }, sectionCornerRadius, 12);
        characterBox.setPosition({ rightColumnX + 10.f, charBoxY });
        characterBox.setFillColor(applyAlpha(slotBackground));
        target.draw(characterBox);

        return columnBottom;
    }

    void drawInventoryTutorialPopup(
        Game& game,
        sf::RenderTarget& target,
        const sf::FloatRect& panelBounds,
        float contentBottomY,
        float alphaFactor
    ) {
    game.inventoryTutorialButtonBounds = {};
    if (!game.inventoryTutorialPopupActive) {
        game.inventoryTutorialButtonHovered = false;
        return;
    }

        float width = panelBounds.size.x * 0.9f;
        float height = 210.f;
        float popupY = std::min(
            panelBounds.position.y + panelBounds.size.y - height - 12.f,
            contentBottomY + 32.f
        );
        popupY = std::max(popupY, panelBounds.position.y + 12.f);
        sf::Vector2f position{
            panelBounds.position.x + (panelBounds.size.x - width) * 0.5f,
            popupY
        };

        sf::FloatRect popupBounds{ { position.x, position.y }, { width, height } };
        ui::popup::drawPopupFrame(target, popupBounds, alphaFactor);

        const float textLeft = position.x + 16.f;
        float textY = position.y + 10.f;
        float maxTextWidth = width - 32.f;
        const auto storytellerStyle = TextStyles::speakerStyle(TextStyles::SpeakerId::StoryTeller);
        constexpr float kTitleFontSize = 30.f;
        constexpr float kTitleIconSize = 36.f;
        constexpr float kTitleIconSpacing = 8.f;
        constexpr float kTitleIconScaleFactor = 0.4f;
        constexpr float kTutorialMessageSpacing = 36.f;
        std::optional<sf::Sprite> titleIcon;
        float titleIconWidth = 0.f;
        float titleIconHeight = 0.f;
        float titleY = textY - 10.f;
        if (game.resources.buttonHelp.getSize().x > 0 && game.resources.buttonHelp.getSize().y > 0) {
            titleIcon.emplace(game.resources.buttonHelp);
            auto iconBounds = titleIcon->getLocalBounds();
            float scale = iconBounds.size.y > 0.f ? (kTitleIconSize / iconBounds.size.y) : 1.f;
            scale *= kTitleIconScaleFactor;
            titleIcon->setScale({ scale, scale });
            titleIconWidth = iconBounds.size.x * scale;
            titleIconHeight = iconBounds.size.y * scale;
        }

        sf::Text titleText{ game.resources.uiFont, "Introduction to the Menu", static_cast<unsigned int>(kTitleFontSize) };
        titleText.setFillColor(ColorHelper::Palette::TitleAccent);
        float iconInset = titleIcon ? titleIconWidth + kTitleIconSpacing : 0.f;
        float titleX = textLeft + iconInset;
        titleText.setPosition({ titleX, titleY });
        auto titleLocalBounds = titleText.getLocalBounds();
        float titleHeight = titleLocalBounds.size.y;
        float titleCenterY = titleY + (titleHeight * 0.5f);
        if (titleIcon) {
        float iconY = titleCenterY - (titleIconHeight * 0.5f) + titleIconHeight;
            titleIcon->setPosition({ textLeft, iconY });
            titleIcon->setColor(ColorHelper::applyAlphaFactor(sf::Color::White, alphaFactor));
            target.draw(*titleIcon);
        }
        target.draw(titleText);

        if (titleIconHeight > titleHeight)
            titleHeight = titleIconHeight;
        textY += titleHeight + 12.f;

        for (std::size_t idx = 0; idx < kInventoryTutorialMessages.size(); ++idx) {
            std::vector<ColoredTextSegment> segments;
            if (idx == 0)
                segments.push_back({ storytellerStyle.name + ": ", storytellerStyle.color });
            segments.push_back({ kInventoryTutorialMessages[idx], ColorHelper::Palette::Normal });

            drawColoredSegments(
                target,
                game.resources.uiFont,
                segments,
                { textLeft, textY },
                22,
                maxTextWidth,
                alphaFactor,
                ui::popup::kLineSpacingMultiplier
            );
            textY += kTutorialMessageSpacing;
        }

        if (game.inventoryTutorialClosing)
            game.inventoryTutorialButtonHovered = false;
        bool hovered = game.inventoryTutorialButtonHovered && !game.inventoryTutorialClosing;
        sf::Vector2f buttonPos{
            position.x + width - kInventoryTutorialButtonWidth - 16.f,
            position.y + height - kInventoryTutorialButtonHeight - 12.f
        };
        game.inventoryTutorialButtonBounds = { { buttonPos.x, buttonPos.y }, { kInventoryTutorialButtonWidth, kInventoryTutorialButtonHeight } };
        drawTutorialButton(
            game,
            target,
            game.inventoryTutorialButtonBounds,
            hovered,
            alphaFactor,
            "Understood"
        );
    }

    void updateMenuButtonFade(Game& game) {
        if (!game.menuButtonUnlocked || !game.menuButtonFadeActive)
            return;
        float progress = std::min(1.f, game.menuButtonFadeClock.getElapsedTime().asSeconds() / kMenuButtonFadeDuration);
        game.menuButtonAlpha = progress;
        if (progress >= 1.f)
            game.menuButtonFadeActive = false;
    }

    void drawInventoryArrow(Game& game, sf::RenderTarget& target) {
        if (!game.inventoryArrowActive)
            return;

        auto bounds = game.menuButton.getGlobalBounds();
        if (bounds.size.x <= 0.f || bounds.size.y <= 0.f)
            return;

        float blinkInterval = std::max(0.01f, game.returnBlinkInterval);
        if (game.inventoryArrowBlinkClock.getElapsedTime().asSeconds() >= blinkInterval) {
            game.inventoryArrowVisible = !game.inventoryArrowVisible;
            game.inventoryArrowBlinkClock.restart();
        }

        if (!game.inventoryArrowVisible)
            return;

        float arrowHeight = bounds.size.y * 0.6f;
        float arrowWidth = arrowHeight * 0.6f;
        float centerY = bounds.position.y + (bounds.size.y * 0.5f);
        float startX = bounds.position.x - arrowWidth - 12.f;

        sf::ConvexShape arrow;
        arrow.setPointCount(3);
        arrow.setPoint(0, { startX, centerY - (arrowHeight / 2.f) });
        arrow.setPoint(1, { bounds.position.x - 12.f, centerY });
        arrow.setPoint(2, { startX, centerY + (arrowHeight / 2.f) });

        arrow.setFillColor(ColorHelper::Palette::SoftYellow);
        arrow.setOutlineThickness(3.f);
        arrow.setOutlineColor(TextStyles::UI::PanelDark);
        target.draw(arrow);
    }

    void drawQuestContent(Game& game, sf::RenderTarget& target, const sf::FloatRect& bounds, float alphaFactor) {
        auto applyAlpha = [&](const sf::Color& color) {
            return ColorHelper::applyAlphaFactor(color, alphaFactor);
        };

        sf::RectangleShape contentBox;
        contentBox.setPosition({ bounds.position.x + 12.f, bounds.position.y + 12.f });
        contentBox.setSize({ bounds.size.x - 24.f, bounds.size.y - 24.f });
        contentBox.setFillColor(applyAlpha(sf::Color(34, 22, 12, 10)));
        contentBox.setOutlineColor(applyAlpha(sf::Color(255, 255, 255, 80)));
        contentBox.setOutlineThickness(2.f);
        target.draw(contentBox);

        constexpr float titleFontSize = 36.f;
        constexpr float titleIconHeight = 32.f;
        constexpr float titleIconSpacing = 10.f;
        float titleY = bounds.position.y + 28.f;
        std::optional<sf::Sprite> titleIcon;
        float iconWidth = 0.f;
        float iconHeight = 0.f;
        if (game.resources.buttonQuests.getSize().x > 0 && game.resources.buttonQuests.getSize().y > 0) {
            titleIcon.emplace(game.resources.buttonQuests);
            float scale = titleIconHeight / static_cast<float>(game.resources.buttonQuests.getSize().y);
            titleIcon->setScale({ scale, scale });
            iconWidth = static_cast<float>(game.resources.buttonQuests.getSize().x) * scale;
            iconHeight = static_cast<float>(game.resources.buttonQuests.getSize().y) * scale;
        }
        sf::Text titleText{ game.resources.uiFont, "Quests", static_cast<unsigned int>(titleFontSize) };
        titleText.setFillColor(applyAlpha(ColorHelper::Palette::SoftYellow));
        float titleWidth = titleText.getLocalBounds().size.x;
        float totalTitleWidth = titleWidth + (iconWidth > 0.f ? iconWidth + titleIconSpacing : 0.f);
        float titleStartX = bounds.position.x + (bounds.size.x - totalTitleWidth) * 0.5f;
        float titleTextX = titleStartX + (iconWidth > 0.f ? iconWidth + titleIconSpacing : 0.f);
        titleText.setPosition({ titleTextX, titleY });
        auto titleGlobalBounds = titleText.getGlobalBounds();
        float titleHeight = titleGlobalBounds.size.y;
        float maxTitleHeight = std::max(titleHeight, iconHeight);
        if (titleIcon) {
            float titleCenterY = titleGlobalBounds.position.y + (titleHeight * 0.5f);
            float iconY = titleCenterY - (iconHeight * 0.5f);
            titleIcon->setPosition({ titleStartX, iconY });
            titleIcon->setColor(applyAlpha(sf::Color::White));
            target.draw(*titleIcon);
        }
        target.draw(titleText);

        constexpr float horizontalPadding = 24.f;
        constexpr float columnSpacing = 30.f;
        constexpr float sectionCornerRadius = 18.f;
        float columnTop = titleY + maxTitleHeight + 20.f;
        float columnBottom = bounds.position.y + bounds.size.y - 24.f;
        float columnHeight = std::max(0.f, columnBottom - columnTop);
        float availableWidth = bounds.size.x - (horizontalPadding * 2.f) - columnSpacing;
        float columnWidth = std::max(220.f, availableWidth * 0.5f);
        float requiredWidth = (columnWidth * 2.f) + columnSpacing;
        float maxInnerWidth = bounds.size.x - horizontalPadding * 2.f;
        if (requiredWidth > maxInnerWidth)
            columnWidth = (maxInnerWidth - columnSpacing) * 0.5f;

        float leftColumnX = bounds.position.x + horizontalPadding;
        float rightColumnX = leftColumnX + columnWidth + columnSpacing;

        const sf::Color sectionColor(205, 193, 160, 220);
        RoundedRectangleShape leftColumn({ columnWidth, columnHeight }, sectionCornerRadius, 12);
        leftColumn.setPosition({ leftColumnX, columnTop });
        leftColumn.setFillColor(applyAlpha(sectionColor));
        target.draw(leftColumn);

        RoundedRectangleShape rightColumn({ columnWidth, columnHeight }, sectionCornerRadius, 12);
        rightColumn.setPosition({ rightColumnX, columnTop });
        rightColumn.setFillColor(applyAlpha(sectionColor));
        target.draw(rightColumn);

        const sf::Color columnTextColor(0, 0, 0);

        auto sanitizeQuestName = [](const std::string& source) {
            std::string cleaned = source;
            cleaned.erase(std::remove_if(cleaned.begin(), cleaned.end(), [](unsigned char ch) {
                return ch == '\r' || ch == '\n';
            }), cleaned.end());
            while (cleaned.size() >= 2 && cleaned.compare(cleaned.size() - 2, 2, "??") == 0)
                cleaned.resize(cleaned.size() - 2);
            return cleaned.empty() ? source : cleaned;
        };

        game.questFoldButtonBounds.assign(game.questLog.size(), {});
        game.questFoldHoveredIndex = -1;

        sf::Text activeHeader{ game.resources.titleFont, "Active Quests", 30 };
        activeHeader.setFillColor(applyAlpha(ColorHelper::Palette::SoftYellow));
        activeHeader.setPosition({ leftColumnX + 18.f, columnTop + 16.f });
        target.draw(activeHeader);

        float entryStartY = columnTop + 56.f;
        constexpr float kEntryHeight = 130.f;
        constexpr float kEntrySpacing = 12.f;
        constexpr float kEntryPadding = 12.f;
        float cardWidth = columnWidth - (kEntryPadding * 2.f);
        float cardX = leftColumnX + kEntryPadding;

        float cursorY = entryStartY;
        constexpr float kCardHorizontalPadding = 14.f;
        constexpr float kCardVerticalPadding = 12.f;
        constexpr float kTextSpacing = 6.f;
        constexpr float kQuestLineSpacingMultiplier = 0.8f;
        constexpr float kFoldAnimationStep = 0.08f;
        constexpr sf::Vector2f kFoldButtonSize{ 36.f, 18.f };
        constexpr float kFoldButtonVerticalMargin = 10.f;
        for (std::size_t idx = 0; idx < game.questLog.size(); ++idx) {
            auto& entry = game.questLog[idx];
            if (entry.completed)
                continue;

            float targetProgress = entry.collapsed ? 0.f : 1.f;
            float diff = targetProgress - entry.foldProgress;
            if (std::abs(diff) <= kFoldAnimationStep)
                entry.foldProgress = targetProgress;
            else
                entry.foldProgress += (diff > 0.f ? kFoldAnimationStep : -kFoldAnimationStep);

            float textX = cardX + kCardHorizontalPadding;
            float measurementBaseY = cursorY + kCardVerticalPadding;

            std::string displayName = sanitizeQuestName(entry.name);
            sf::Text nameText{ game.resources.titleFont, displayName, 22 };
            float nameHeight = nameText.getLocalBounds().size.y;

            float giverY = measurementBaseY + nameHeight + kTextSpacing;
            sf::Text giverText{ game.resources.uiFont, "From " + entry.giver, 16 };
            float giverHeight = giverText.getLocalBounds().size.y;
            float goalStartY = giverY + giverHeight + kTextSpacing;

            std::vector<ColoredTextSegment> goalSegments = {
                { entry.goal, ColorHelper::Palette::Normal }
            };
            auto measuredCursor = drawColoredSegments(
                target,
                game.resources.uiFont,
                goalSegments,
                { textX, goalStartY },
                16,
                cardWidth - 28.f,
                1.f,
                kQuestLineSpacingMultiplier,
                true
            );
            float goalHeight = measuredCursor.y - goalStartY;

            float contentHeight = (goalStartY - cursorY) + goalHeight + kCardVerticalPadding;
            float expandedHeight = std::max(kEntryHeight, contentHeight);

            float titleOnlyHeight = nameHeight + (kCardVerticalPadding * 2.f);
            float buttonAreaHeight = (kFoldButtonVerticalMargin * 2.f) + kFoldButtonSize.y;
            float collapsedHeight = std::max(titleOnlyHeight, buttonAreaHeight);
            collapsedHeight = std::min(collapsedHeight, expandedHeight);
            float cardHeight = collapsedHeight + (expandedHeight - collapsedHeight) * entry.foldProgress;
            if (cursorY + cardHeight > columnTop + columnHeight - 12.f)
                break;

            float detailAlpha = entry.foldProgress;

            RoundedRectangleShape card({ cardWidth, cardHeight }, 16.f, 18);
            card.setPosition({ cardX, cursorY });
            card.setFillColor(applyAlpha(sf::Color(18, 12, 6, 220)));
            card.setOutlineThickness(2.f);
            card.setOutlineColor(applyAlpha(ColorHelper::Palette::FrameGoldLight));
            target.draw(card);

            float textY = measurementBaseY;
            nameText.setFillColor(applyAlpha(ColorHelper::Palette::SoftYellow));
            nameText.setPosition({ textX, textY });
            target.draw(nameText);
            textY += nameHeight + kTextSpacing;

            auto detailColorize = [&](const sf::Color& base) {
                return ColorHelper::applyAlphaFactor(base, alphaFactor * detailAlpha);
            };
            giverText.setFillColor(detailColorize(sf::Color::White));
            giverText.setPosition({ textX, textY });
            target.draw(giverText);
            textY += giverHeight + kTextSpacing;

            auto goalCursor = drawColoredSegments(
                target,
                game.resources.uiFont,
                goalSegments,
                { textX, textY },
                16,
                cardWidth - 28.f,
                detailAlpha,
                kQuestLineSpacingMultiplier,
                false
            );

            sf::Text spacingMetrics{ game.resources.uiFont, "Hg", 16 };
            float extraSpacing = spacingMetrics.getLineSpacing() * 0.8f - 4.f;
            textY = goalCursor.y + extraSpacing;

            float xpLineY = std::max(cursorY + cardHeight - 28.f, textY);
            sf::Text xpText{ game.resources.uiFont, "XP Reward: " + std::to_string(entry.xpReward), 18 };
            xpText.setFillColor(detailColorize(ColorHelper::Palette::DarkPurple));
            xpText.setPosition({ textX, xpLineY });
            target.draw(xpText);

            if (entry.loot) {
                sf::Text lootText{ game.resources.uiFont, "Loot: " + *entry.loot, 18 };
                lootText.setFillColor(detailColorize(ColorHelper::Palette::SoftYellow));
                auto lootBounds = lootText.getLocalBounds();
                lootText.setPosition({
                    cardX + cardWidth - 12.f - lootBounds.size.x,
                    xpLineY
                });
                target.draw(lootText);
            }

            sf::Vector2f buttonPos{
                cardX + cardWidth - kFoldButtonSize.x - 16.f,
                cursorY + 10.f
            };
            RoundedRectangleShape buttonShape(kFoldButtonSize, kFoldButtonSize.y * 0.5f, 10);
            buttonShape.setPosition(buttonPos);
            float buttonHoverAlpha = (game.questFoldHoveredIndex == static_cast<int>(idx)) ? 1.f : 0.85f;
            buttonShape.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, alphaFactor * buttonHoverAlpha));
            buttonShape.setOutlineThickness(1.5f);
            buttonShape.setOutlineColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, alphaFactor * buttonHoverAlpha));
            target.draw(buttonShape);

            sf::Text foldLabel{
                game.resources.uiFont,
                entry.collapsed ? "+" : "-",
                14
            };
            foldLabel.setFillColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, alphaFactor));
            auto foldBounds = foldLabel.getLocalBounds();
            foldLabel.setOrigin({
                foldBounds.position.x + foldBounds.size.x * 0.5f,
                foldBounds.position.y + foldBounds.size.y * 0.5f
            });
            foldLabel.setPosition({
                buttonPos.x + (kFoldButtonSize.x * 0.5f),
                buttonPos.y + (kFoldButtonSize.y * 0.5f)
            });
            target.draw(foldLabel);

            game.questFoldButtonBounds[idx] = { buttonPos, kFoldButtonSize };

            cursorY += cardHeight + kEntrySpacing;
        }

        sf::Text finishedHeader{ game.resources.titleFont, "Finished Quests", 30 };
        finishedHeader.setFillColor(applyAlpha(ColorHelper::Palette::SoftYellow));
        finishedHeader.setPosition({ rightColumnX + 18.f, columnTop + 16.f });
        target.draw(finishedHeader);

        float finishedStartY = columnTop + 56.f;
        float finishedCursorY = finishedStartY;
        const float kFinishedCardHeight = 46.f;
        const float kFinishedNameSize = 20.f;
        constexpr float kFinishedSpacing = 12.f;
        for (const auto& entry : game.questLog) {
            if (!entry.completed)
                continue;

            RoundedRectangleShape card({ cardWidth, kFinishedCardHeight }, 12.f, 12);
            card.setPosition({ rightColumnX + kEntryPadding, finishedCursorY });
            card.setFillColor(applyAlpha(sf::Color(10, 10, 10, 200)));
            card.setOutlineThickness(2.f);
            card.setOutlineColor(applyAlpha(sf::Color(255, 255, 255, 120)));
            target.draw(card);

            std::string displayName = sanitizeQuestName(entry.name);
            sf::Text nameText{ game.resources.titleFont, displayName, static_cast<unsigned int>(kFinishedNameSize) };
            nameText.setFillColor(applyAlpha(ColorHelper::Palette::SoftYellow));
            nameText.setPosition({
                rightColumnX + kEntryPadding + 8.f,
                finishedCursorY + (kFinishedCardHeight - nameText.getLocalBounds().size.y) * 0.5f
            });
            target.draw(nameText);

            auto nameBounds = nameText.getGlobalBounds();
            sf::RectangleShape strike({ nameBounds.size.x, 1.5f });
            strike.setFillColor(ColorHelper::applyAlphaFactor(sf::Color::White, alphaFactor));
            strike.setPosition({
                nameBounds.position.x,
                nameBounds.position.y + (nameBounds.size.y * 0.5f)
            });
            target.draw(strike);

            finishedCursorY += kFinishedCardHeight + kFinishedSpacing;
            if (finishedCursorY > columnTop + columnHeight - 12.f)
                break;
        }

    }

    void drawSkillsContent(Game& game, sf::RenderTarget& target, const sf::FloatRect& bounds, float alphaFactor) {
        sf::RectangleShape contentBox;
        contentBox.setPosition({ bounds.position.x + 12.f, bounds.position.y + 12.f });
        contentBox.setSize({ bounds.size.x - 24.f, bounds.size.y - 24.f });
        auto applyAlpha = [&](const sf::Color& color) {
            return ColorHelper::applyAlphaFactor(color, alphaFactor);
        };
        contentBox.setFillColor(applyAlpha(sf::Color(34, 22, 12, 10)));
        contentBox.setOutlineColor(applyAlpha(sf::Color(255, 255, 255, 80)));
        contentBox.setOutlineThickness(2.f);
        target.draw(contentBox);

        constexpr float padding = 28.f;
        constexpr float gapBetweenSprites = 24.f;
        float availableWidth = bounds.size.x - (padding * 2.f);
        float availableHeight = bounds.size.y - (padding * 2.f);
        sf::Sprite treeSprite(game.resources.skilltree);
        sf::Sprite barSprite(game.resources.skillbar);
        auto treeBounds = treeSprite.getLocalBounds();
        auto barBounds = barSprite.getLocalBounds();
        float maxTextureWidth = std::max(treeBounds.size.x, barBounds.size.x);
        float totalTextureHeight = treeBounds.size.y + barBounds.size.y + gapBetweenSprites;
        if (maxTextureWidth <= 0.f)
            maxTextureWidth = 1.f;
        if (totalTextureHeight <= 0.f)
            totalTextureHeight = gapBetweenSprites;

        float widthScale = availableWidth / maxTextureWidth;
        float heightScale = availableHeight / totalTextureHeight;
        float scale = std::min(widthScale, heightScale);
        if (scale > 1.f)
            scale = 1.f;
        if (scale <= 0.f)
            scale = 0.1f;
        treeSprite.setScale({ scale, scale });
        barSprite.setScale({ scale, scale });

        float treeWidth = treeBounds.size.x * scale;
        float treeHeight = treeBounds.size.y * scale;
        float barWidth = barBounds.size.x * scale;
        float barHeight = barBounds.size.y * scale;
        float treeX = bounds.position.x + (bounds.size.x - treeWidth) * 0.5f;
        float barX = bounds.position.x + (bounds.size.x - barWidth) * 0.5f;
        float treeY = bounds.position.y + padding;
        float barY = treeY + treeHeight + gapBetweenSprites;
        treeSprite.setPosition({ treeX, treeY });
        barSprite.setPosition({ barX, barY });
        treeSprite.setColor(ColorHelper::applyAlphaFactor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(255 * 0.95f)), alphaFactor));
        barSprite.setColor(ColorHelper::applyAlphaFactor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(255 * 0.95f)), alphaFactor));
        target.draw(treeSprite);
        target.draw(barSprite);
    }

    void drawPlaceholderContent(
        Game& game,
        sf::RenderTarget& target,
        const sf::FloatRect& bounds,
        const std::string& title
    ) {
        sf::RectangleShape contentBox;
        contentBox.setPosition({ bounds.position.x + 12.f, bounds.position.y + 12.f });
        contentBox.setSize({ bounds.size.x - 24.f, bounds.size.y - 24.f });
        contentBox.setFillColor(sf::Color(45, 30, 15));
        contentBox.setOutlineColor(sf::Color::White);
        contentBox.setOutlineThickness(2.f);
        target.draw(contentBox);

        sf::Text header{ game.resources.uiFont, title, 32 };
        header.setFillColor(ColorHelper::Palette::Normal);
        auto headerBounds = header.getLocalBounds();
        header.setOrigin({
            headerBounds.position.x + headerBounds.size.x * 0.5f,
            headerBounds.position.y + headerBounds.size.y * 0.5f
        });
        header.setPosition({
            bounds.position.x + bounds.size.x * 0.5f,
            bounds.position.y + 48.f
        });
        target.draw(header);

        sf::Text info{ game.resources.uiFont, "Content coming soon...", 18 };
        info.setFillColor(ColorHelper::Palette::Dim);
        auto infoBounds = info.getLocalBounds();
        info.setOrigin({
            infoBounds.position.x + infoBounds.size.x * 0.5f,
            infoBounds.position.y + infoBounds.size.y * 0.5f
        });
        info.setPosition({
            bounds.position.x + bounds.size.x * 0.5f,
            bounds.position.y + 96.f
        });
        target.draw(info);
    }

    void drawRankingContent(Game& game, sf::RenderTarget& target, const sf::FloatRect& bounds) {
        sf::RectangleShape contentBox;
        contentBox.setPosition({ bounds.position.x + 12.f, bounds.position.y + 12.f });
        contentBox.setSize({ bounds.size.x - 24.f, bounds.size.y - 24.f });
        contentBox.setFillColor(sf::Color(45, 30, 15));
        contentBox.setOutlineColor(sf::Color::White);
        contentBox.setOutlineThickness(2.f);
        target.draw(contentBox);

        sf::Text header{ game.resources.uiFont, "Rankings", 34 };
        header.setFillColor(ColorHelper::Palette::Normal);
        auto headerBounds = header.getLocalBounds();
        header.setOrigin({
            headerBounds.position.x + headerBounds.size.x * 0.5f,
            headerBounds.position.y + headerBounds.size.y * 0.5f
        });
        header.setPosition({
            bounds.position.x + bounds.size.x * 0.5f,
            bounds.position.y + 42.f
        });
        target.draw(header);

        auto entries = game.rankingManager.entries();
        float y = bounds.position.y + 80.f;
        float rightEdge = bounds.position.x + bounds.size.x - 24.f;
        for (std::size_t i = 0; i < entries.size() && y < bounds.position.y + bounds.size.y - 40.f; ++i) {
            const auto& entry = entries[i];
            sf::Text label{ game.resources.uiFont, "", 22 };
            std::ostringstream text;
            text << (i + 1) << ". " << entry.playerName << "  ·  " << formatTime(entry.totalSeconds);
            text << "  ·  Faults: " << entry.faults;
            label.setString(text.str());
            if (static_cast<int>(i + 1) == game.lastRecordedRank)
                label.setFillColor(ColorHelper::Palette::SoftYellow);
            else
                label.setFillColor(ColorHelper::Palette::Normal);
            label.setPosition({ bounds.position.x + 28.f, y });
            target.draw(label);
            y += 30.f;
        }

        if (entries.empty()) {
            sf::Text empty{ game.resources.uiFont, "No completed runs yet.", 20 };
            empty.setFillColor(ColorHelper::Palette::Dim);
            auto emptyBounds = empty.getLocalBounds();
            empty.setOrigin({
                emptyBounds.position.x + emptyBounds.size.x * 0.5f,
                emptyBounds.position.y + emptyBounds.size.y * 0.5f
            });
        empty.setPosition({
            bounds.position.x + bounds.size.x * 0.5f,
            bounds.position.y + bounds.size.y * 0.5f
        });
            target.draw(empty);
        }
    }
} // namespace

bool handleEvent(Game& game, const sf::Event& event) {
    bool consumed = false;
    auto updateHovered = [&](const sf::Vector2f& point) {
        game.menuHoveredTab = -1;
        for (std::size_t idx = 0; idx < game.menuTabBounds.size(); ++idx) {
            if (game.menuTabBounds[idx].contains(point)) {
                game.menuHoveredTab = static_cast<int>(idx);
                break;
            }
        }
    };
    bool menuButtonInteractable = game.menuButtonUnlocked && game.menuButtonAlpha >= 1.f;
    bool mapTabActive = game.menuActive && game.menuActiveTab == static_cast<int>(MenuTab::Map);
    auto dispatchMenuMapEvent = [&]() -> bool {
        if (!mapTabActive)
            return false;
        if (!game.mapTutorialActive && !game.mapInteractionUnlocked)
            return false;
        const auto viewFromPanel = [&]() -> std::optional<sf::View> {
            auto panelBounds = game.menuPanel.getGlobalBounds();
            auto windowSize = game.window.getSize();
            if (panelBounds.size.x <= 0.f || panelBounds.size.y <= 0.f || windowSize.x == 0 || windowSize.y == 0)
                return std::nullopt;
            sf::View mapView(sf::FloatRect({ 0.f, 0.f }, { panelBounds.size.x, panelBounds.size.y }));
            mapView.setCenter(sf::Vector2f{ panelBounds.size.x * 0.5f, panelBounds.size.y * 0.5f });
            mapView.setViewport(sf::FloatRect(
                { panelBounds.position.x / static_cast<float>(windowSize.x),
                  panelBounds.position.y / static_cast<float>(windowSize.y) },
                { panelBounds.size.x / static_cast<float>(windowSize.x),
                  panelBounds.size.y / static_cast<float>(windowSize.y) }
            ));
            return mapView;
        }();
        if (viewFromPanel) {
            handleMapSelectionEvent(game, event, viewFromPanel.operator->());
        }
        else {
            handleMapSelectionEvent(game, event, nullptr);
        }
        return true;
    };
    if (game.forcedDestinationSelection && game.menuActive)
        game.menuActiveTab = static_cast<int>(MenuTab::Map);
    if (!game.mapTutorialActive && dispatchMenuMapEvent())
        return true;

    if (auto move = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f point = game.window.mapPixelToCoords(move->position);
        if (game.mapTutorialActive) {
            game.menuHoveredTab = -1;
            game.mapTutorialOkHovered = game.mapTutorialOkBounds.contains(point);
            game.menuButtonHovered = false;
        }
        else if (game.menuActive) {
            if (!(game.inventoryTutorialPopupActive && !game.inventoryTutorialClosing))
                updateHovered(point);
            else
                game.menuHoveredTab = -1;
            game.mapTutorialOkHovered = false;
            if (!(game.inventoryTutorialPopupActive && !game.inventoryTutorialClosing)
                && game.menuActiveTab == static_cast<int>(MenuTab::Quests))
            {
                game.questFoldHoveredIndex = -1;
                for (std::size_t idx = 0; idx < game.questFoldButtonBounds.size(); ++idx) {
                    if (game.questFoldButtonBounds[idx].contains(point)) {
                        game.questFoldHoveredIndex = static_cast<int>(idx);
                        break;
                    }
                }
            }
            else {
                game.questFoldHoveredIndex = -1;
            }
        }
        else if (menuButtonInteractable) {
            game.menuButtonHovered = game.menuButton.getGlobalBounds().contains(point);
            game.mapTutorialOkHovered = false;
        }
        else {
            game.menuButtonHovered = false;
            game.mapTutorialOkHovered = false;
        }
        if (!game.mapTutorialActive) {
            if (game.inventoryTutorialPopupActive && !game.inventoryTutorialClosing) {
                game.inventoryTutorialButtonHovered = game.inventoryTutorialButtonBounds.contains(point);
            }
            else {
                game.inventoryTutorialButtonHovered = false;
            }
        }
        else {
            game.inventoryTutorialButtonHovered = false;
        }
        if (!game.menuActive)
            game.questFoldHoveredIndex = -1;
    }

    if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button == sf::Mouse::Button::Left) {
            sf::Vector2f point = game.window.mapPixelToCoords(button->position);

            if (game.mapTutorialActive) {
                if (game.mapTutorialOkBounds.contains(point)) {
                    game.mapTutorialAwaitingOk = false;
                    game.mapTutorialOkHovered = false;
                    advanceDialogueLine(game);
                }
                consumed = true;
                return true;
            }

            if (!game.menuActive && menuButtonInteractable && game.menuButton.getGlobalBounds().contains(point)) {
                game.menuActive = true;
                consumed = true;
                beginInventoryTutorial(game);
            }
            else if (game.menuActive) {
                bool clickedTab = false;
                if (!(game.inventoryTutorialPopupActive && !game.inventoryTutorialClosing)) {
                    for (std::size_t idx = 0; idx < game.menuTabBounds.size(); ++idx) {
                        if (game.menuTabBounds[idx].contains(point)) {
                            game.menuActiveTab = static_cast<int>(idx);
                            clickedTab = true;
                            break;
                        }
                    }
                }

                if (clickedTab && !game.forcedDestinationSelection) {
                    consumed = true;
                }
                else {
                    auto panelBounds = game.menuPanel.getGlobalBounds();
                    bool insidePanel = panelBounds.contains(point);
                    if (!(game.inventoryTutorialPopupActive && !game.inventoryTutorialClosing)
                        && game.menuActiveTab == static_cast<int>(MenuTab::Quests))
                    {
                        for (std::size_t idx = 0; idx < game.questFoldButtonBounds.size(); ++idx) {
                            if (game.questFoldButtonBounds[idx].contains(point)) {
                                auto& entry = game.questLog[idx];
                                if (!entry.completed) {
                                    entry.collapsed = !entry.collapsed;
                                    consumed = true;
                                    return true;
                                }
                            }
                        }
                    }
                    if (game.inventoryTutorialPopupActive) {
                        if (!game.inventoryTutorialClosing
                            && game.inventoryTutorialButtonBounds.contains(point))
                            {
                                game.inventoryTutorialClosing = true;
                                game.inventoryTutorialCloseClock.restart();
                                game.inventoryTutorialCloseProgress = 0.f;
                                game.inventoryTutorialButtonHovered = false;
                                game.inventoryTutorialAdvancePending = true;
                            }
                    }
                    else if (!insidePanel) {
                        if (!game.forcedDestinationSelection)
                            game.menuActive = false;
                    }
                    consumed = true;
                }
            }
        }
    }

    if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            if (game.menuActive) {
                if (!game.inventoryTutorialPopupActive && !game.mapTutorialActive && !game.forcedDestinationSelection)
                    game.menuActive = false;
            }
            else if (menuButtonInteractable) {
                game.menuActive = true;
                consumed = true;
                beginInventoryTutorial(game);
            }
        }

        if (game.menuActive) {
        if (game.mapTutorialActive) {
            consumed = true;
        }
        else if (!game.forcedDestinationSelection) {
            if (key->code == sf::Keyboard::Key::Right) {
                game.menuActiveTab = (game.menuActiveTab + 1) % static_cast<int>(kTabLabels.size());
                consumed = true;
            }
            else if (key->code == sf::Keyboard::Key::Left) {
                game.menuActiveTab = (game.menuActiveTab - 1 + static_cast<int>(kTabLabels.size())) % static_cast<int>(kTabLabels.size());
                consumed = true;
            }
        }
        else if (key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::Left) {
            consumed = true;
        }
        }
    }

    if (!game.menuActive)
        game.menuHoveredTab = -1;

    if (game.menuActive)
        consumed = true;

    return consumed;
}

void draw(Game& game, sf::RenderTarget& target) {
    updateMenuButtonFade(game);
    float tutorialCloseProgress = game.inventoryTutorialClosing
        ? std::min(1.f, game.inventoryTutorialCloseClock.getElapsedTime().asSeconds() / kInventoryTutorialCloseDuration)
        : 0.f;
    game.inventoryTutorialCloseProgress = tutorialCloseProgress;
    float tutorialAlpha = game.inventoryTutorialClosing ? (1.f - tutorialCloseProgress) : 1.f;

    if (game.menuButton.getSize().x <= 0.f || game.menuButton.getSize().y <= 0.f)
        return;

    if (game.forcedDestinationSelection)
        game.menuActiveTab = static_cast<int>(MenuTab::Map);
    game.menuMapPopup.reset();

    sf::Color buttonColor = sf::Color::White;
    if (game.menuActive)
        buttonColor = ColorHelper::darken(buttonColor, 0.38f);
    else if (game.menuButtonHovered)
        buttonColor = ColorHelper::darken(buttonColor, 0.25f);

    float menuAlpha = std::clamp(game.menuButtonAlpha, 0.f, 1.f);
    float forgingOverlayAlpha = (game.weaponForging.phase == Game::WeaponForgingState::Phase::Idle)
        ? 0.f
        : game.weaponForging.alpha;
    menuAlpha = std::clamp(menuAlpha * (1.f - forgingOverlayAlpha), 0.f, 1.f);
    buttonColor.a = static_cast<std::uint8_t>(buttonColor.a * menuAlpha);
    game.menuButton.setFillColor(buttonColor);
    if (menuAlpha > 0.f) {
        constexpr float kMenuButtonBackdropPad = 12.f;
        sf::RectangleShape backdrop({
            game.menuButton.getSize().x + kMenuButtonBackdropPad,
            game.menuButton.getSize().y + kMenuButtonBackdropPad
        });
        backdrop.setPosition({
            game.menuButton.getPosition().x - (kMenuButtonBackdropPad * 0.5f),
            game.menuButton.getPosition().y - (kMenuButtonBackdropPad * 0.5f)
        });
        backdrop.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::DialogBackdrop, menuAlpha));
        target.draw(backdrop);
        target.draw(game.menuButton);
    }

    drawInventoryArrow(game, target);

    if (!game.menuActive)
        return;

    float menuFadeFactor = (game.inventoryTutorialClosing) ? tutorialAlpha : 1.f;
    sf::Vector2u windowSize = target.getSize();
    sf::RectangleShape overlay({ static_cast<float>(windowSize.x), static_cast<float>(windowSize.y) });
    overlay.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Overlay, menuFadeFactor));
    target.draw(overlay);

    // Draw tabs
    float tabFade = game.menuActive ? 1.f : 0.f;
    constexpr float kTabIconMaxHeight = 36.f;
    constexpr float kTabIconLabelSpacing = 10.f;
    for (std::size_t idx = 0; idx < kTabLabels.size(); ++idx) {
        const auto& bounds = game.menuTabBounds[idx];
        if (bounds.size.x <= 0.f || bounds.size.y <= 0.f)
            continue;

        sf::RectangleShape tabBackground({ bounds.size.x, bounds.size.y });
        tabBackground.setPosition({ bounds.position.x, bounds.position.y });
        sf::Color fillColor = sf::Color(30, 24, 15, 200);
        std::uint8_t outlineAlpha = static_cast<std::uint8_t>(tabFade * 200);
        sf::Color outlineColor(255, 255, 255, outlineAlpha);
        tabBackground.setOutlineThickness(1.f);
        if (static_cast<int>(idx) == game.menuActiveTab || static_cast<int>(idx) == game.menuHoveredTab) {
            fillColor = ColorHelper::applyAlphaFactor(sf::Color::White, 0.15f);
            tabBackground.setOutlineThickness(2.f);
        }

        tabBackground.setFillColor(ColorHelper::applyAlphaFactor(fillColor, menuFadeFactor));
        tabBackground.setOutlineColor(ColorHelper::applyAlphaFactor(outlineColor, menuFadeFactor));
        target.draw(tabBackground);

        const sf::Texture* icon = nullptr;
        switch (static_cast<MenuTab>(idx)) {
            case MenuTab::Inventory: icon = &game.resources.buttonInventory; break;
            case MenuTab::Skills: icon = &game.resources.buttonSkills; break;
            case MenuTab::Map: icon = &game.resources.buttonMap; break;
            case MenuTab::Quests: icon = &game.resources.buttonQuests; break;
            case MenuTab::Settings: icon = &game.resources.buttonSettings; break;
            case MenuTab::Rankings: icon = &game.resources.buttonRankings; break;
        }

        std::optional<sf::Sprite> sprite;
        float iconWidth = 0.f;
        float iconHeight = 0.f;
        if (icon && icon->getSize().x > 0 && icon->getSize().y > 0) {
            sprite.emplace(*icon);
            float iconTargetHeight = std::min(bounds.size.y * 0.45f, kTabIconMaxHeight);
            float scale = iconTargetHeight / static_cast<float>(icon->getSize().y);
            iconHeight = static_cast<float>(icon->getSize().y) * scale;
            iconWidth = static_cast<float>(icon->getSize().x) * scale;
            sprite->setScale({ scale, scale });
        }

        sf::Text label{ game.resources.uiFont, kTabLabels[idx], 22 };
        bool active = static_cast<int>(idx) == game.menuActiveTab;
        sf::Color textColor = active ? ColorHelper::Palette::SoftYellow : ColorHelper::Palette::Normal;
        label.setFillColor(ColorHelper::applyAlphaFactor(textColor, menuFadeFactor));
        auto labelBounds = label.getLocalBounds();
        float labelWidth = labelBounds.size.x;
        label.setOrigin({ labelBounds.position.x, labelBounds.position.y + labelBounds.size.y * 0.5f });

        float spacing = iconWidth > 0.f ? kTabIconLabelSpacing : 0.f;
        float combinedWidth = labelWidth + (iconWidth > 0.f ? iconWidth + spacing : 0.f);
        float startX = bounds.position.x + (bounds.size.x - combinedWidth) * 0.5f;
        float currentX = startX;

        if (sprite.has_value()) {
            float iconY = bounds.position.y + (bounds.size.y - iconHeight) * 0.5f;
            sprite->setPosition({ currentX, iconY });
            sprite->setColor(ColorHelper::applyAlphaFactor(sprite->getColor(), menuFadeFactor));
            target.draw(*sprite);
            currentX += iconWidth + spacing;
        }

        float labelY = bounds.position.y + bounds.size.y * 0.5f;
        label.setPosition({ currentX, labelY });
        target.draw(label);
    }

    game.menuPanel.setFillColor(ColorHelper::applyAlphaFactor(sf::Color(54, 33, 18), menuFadeFactor));
    game.menuPanel.setOutlineColor(ColorHelper::applyAlphaFactor(sf::Color::White, menuFadeFactor));
    game.menuPanel.setOutlineThickness(3.f);
    target.draw(game.menuPanel);

    auto panelBounds = game.menuPanel.getGlobalBounds();
    float contentBottom = panelBounds.position.y + panelBounds.size.y - 24.f;
    switch (static_cast<MenuTab>(game.menuActiveTab)) {
        case MenuTab::Inventory:
            contentBottom = drawInventoryContent(game, target, panelBounds, menuFadeFactor);
            break;
        case MenuTab::Skills:
            drawSkillsContent(game, target, panelBounds, menuFadeFactor);
            break;
        case MenuTab::Map: {
            sf::View prev = target.getView();
            sf::View mapView(sf::FloatRect({ 0.f, 0.f }, { panelBounds.size.x, panelBounds.size.y }));
            mapView.setCenter(sf::Vector2f{ panelBounds.size.x * 0.5f, panelBounds.size.y * 0.5f });
            mapView.setViewport(sf::FloatRect(
                { panelBounds.position.x / static_cast<float>(windowSize.x),
                  panelBounds.position.y / static_cast<float>(windowSize.y) },
                { panelBounds.size.x / static_cast<float>(windowSize.x),
                  panelBounds.size.y / static_cast<float>(windowSize.y) }
            ));
            target.setView(mapView);
            auto popup = drawMapSelectionUI(game, target);
            game.menuMapPopup = popup;
            target.setView(prev);
            break;
        }
        case MenuTab::Quests:
            drawQuestContent(game, target, panelBounds, menuFadeFactor);
            break;
        case MenuTab::Settings:
            drawPlaceholderContent(game, target, panelBounds, "Settings");
            break;
        case MenuTab::Rankings:
            drawRankingContent(game, target, panelBounds);
            break;
    }

    drawMapTutorialPopup(game, target, panelBounds, menuFadeFactor);
    drawInventoryTutorialPopup(game, target, panelBounds, contentBottom, menuFadeFactor);

    if (game.inventoryTutorialClosing && tutorialCloseProgress >= 1.f) {
        game.inventoryTutorialClosing = false;
        game.inventoryTutorialPopupActive = false;
        game.inventoryArrowActive = false;
        game.inventoryTutorialCompleted = true;
        game.menuActive = false;
        if (game.inventoryTutorialAdvancePending) {
            if (game.currentDialogue == &perigonal
                && game.dialogueIndex < game.currentDialogue->size()
                && (*game.currentDialogue)[game.dialogueIndex].text == kInventoryArrowLineText)
            {
                advanceDialogueLine(game);
            }
            game.inventoryTutorialAdvancePending = false;
        }
        game.inventoryTutorialButtonBounds = {};
        game.inventoryTutorialButtonHovered = false;
        game.inventoryTutorialCloseProgress = 0.f;
    }
}


} // namespace ui::menu
