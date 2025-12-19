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

namespace ui::menu {

namespace {
    constexpr std::array<const char*, 6> kTabLabels = {
        "Inventory",
        "Character",
        "Map",
        "Quests",
        "Settings",
        "Rankings"
    };

    enum class MenuTab {
        Inventory = 0,
        Character,
        Map,
        Quests,
        Settings,
        Rankings,
    };

    class RoundedRectangleShape : public sf::Shape {
    public:
        RoundedRectangleShape(sf::Vector2f size = {}, float radius = 0.f, std::size_t cornerPointCount = 8)
        : m_size(size)
        , m_radius(radius)
        , m_cornerPointCount(std::max<std::size_t>(2, cornerPointCount))
        {
            update();
        }

        void setSize(sf::Vector2f size) {
            m_size = size;
            update();
        }

        void setCornerRadius(float radius) {
            m_radius = radius;
            update();
        }

        void setCornerPointCount(std::size_t count) {
            m_cornerPointCount = std::max<std::size_t>(2, count);
            update();
        }

        [[nodiscard]] sf::Vector2f getSize() const {
            return m_size;
        }

        [[nodiscard]] float getCornerRadius() const {
            return m_radius;
        }

        [[nodiscard]] std::size_t getPointCount() const override {
            return m_cornerPointCount * 4;
        }

        [[nodiscard]] sf::Vector2f getPoint(std::size_t index) const override {
            float radius = std::min(m_radius, std::min(m_size.x, m_size.y) * 0.5f);
            std::size_t corner = index / m_cornerPointCount;
            std::size_t pointIndex = index % m_cornerPointCount;

            if (radius <= 0.f) {
                return {
                    (corner % 2) * m_size.x,
                    (corner / 2) * m_size.y
                };
            }

            constexpr float kPi = 3.14159265358979323846f;
            constexpr float kQuarter = kPi * 0.5f;
            float progress = static_cast<float>(pointIndex) / static_cast<float>(m_cornerPointCount - 1);
            float startAngle = 0.f;
            sf::Vector2f center;

            switch (corner) {
                case 0:
                    center = { radius, radius };
                    startAngle = kPi;
                    break;
                case 1:
                    center = { m_size.x - radius, radius };
                    startAngle = 1.5f * kPi;
                    break;
                case 2:
                    center = { m_size.x - radius, m_size.y - radius };
                    startAngle = 0.f;
                    break;
                default:
                    center = { radius, m_size.y - radius };
                    startAngle = 0.5f * kPi;
                    break;
            }

            float angle = startAngle + progress * kQuarter;
            return center + sf::Vector2f{ std::cos(angle), std::sin(angle) } * radius;
        }

    private:
        sf::Vector2f m_size;
        float m_radius = 0.f;
        std::size_t m_cornerPointCount = 4;
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

    void beginInventoryTutorial(Game& game) {
        if (!game.inventoryTutorialPending)
            return;
        game.inventoryArrowActive = false;
        game.inventoryTutorialPending = false;
        game.inventoryTutorialPopupActive = true;
        game.inventoryTutorialButtonHovered = false;
        game.inventoryTutorialAdvancePending = false;
    }

    float drawInventoryContent(Game& game, sf::RenderTarget& target, const sf::FloatRect& bounds) {
        sf::RectangleShape contentBox;
        contentBox.setPosition({ bounds.position.x + 12.f, bounds.position.y + 12.f });
        contentBox.setSize({ bounds.size.x - 24.f, bounds.size.y - 24.f });
        contentBox.setFillColor(sf::Color(28, 18, 10));
        contentBox.setOutlineColor(sf::Color(255, 255, 255, 120));
        contentBox.setOutlineThickness(1.5f);
        target.draw(contentBox);

        sf::Text header{ game.resources.uiFont, "Inventory", 36 };
        header.setFillColor(ColorHelper::Palette::SoftYellow);
        header.setPosition({ bounds.position.x + 32.f, bounds.position.y + 38.f });
        target.draw(header);

        sf::Text info{ game.resources.uiFont, "Wanda keeps track of the gear you recover here.", 18 };
        info.setFillColor(ColorHelper::Palette::Dim);
        info.setPosition({ bounds.position.x + 32.f, bounds.position.y + 84.f });
        target.draw(info);

        auto& icons = game.itemController.icons();
        if (icons.empty()) {
            sf::Text empty{ game.resources.uiFont, "Your inventory is empty.", 20 };
            empty.setFillColor(ColorHelper::Palette::Dim);
            empty.setPosition({ bounds.position.x + 32.f, bounds.position.y + 140.f });
            target.draw(empty);
        }

        constexpr float iconSize = 72.f;
        constexpr float iconSpacing = 28.f;
        constexpr int columns = 4;
        constexpr float slotCornerRadius = 12.f;
        const sf::Color slotBackground(18, 10, 6, 230);

        float startX = bounds.position.x + 32.f;
        float startY = bounds.position.y + 150.f;
        std::size_t rows = std::max<std::size_t>(3, icons.empty() ? 0 : (icons.size() + columns - 1) / columns);
        float gridHeight = rows * iconSize + (rows - 1) * iconSpacing;

        for (std::size_t row = 0; row < rows; ++row) {
            for (std::size_t column = 0; column < columns; ++column) {
                float cellX = startX + static_cast<float>(column) * (iconSize + iconSpacing);
                float cellY = startY + static_cast<float>(row) * (iconSize + iconSpacing);
                RoundedRectangleShape cell({ iconSize, iconSize }, slotCornerRadius, 8);
                cell.setPosition({ cellX, cellY });
                cell.setFillColor(slotBackground);
                cell.setOutlineThickness(0.f);
                target.draw(cell);
            }
        }

        constexpr float kDividerThickness = 2.f;
        const sf::Color dividerColor(205, 193, 160, 200);
        float gridWidth = columns * iconSize + (columns - 1) * iconSpacing;
        for (int divider = 1; divider < columns; ++divider) {
            float x = startX
                + static_cast<float>(divider) * iconSize
                + static_cast<float>(divider - 1) * iconSpacing
                + (iconSpacing * 0.5f);
            sf::RectangleShape line({ kDividerThickness, gridHeight });
            line.setPosition({ x - (kDividerThickness * 0.5f), startY });
            line.setFillColor(dividerColor);
            target.draw(line);
        }
        for (std::size_t divider = 1; divider < rows; ++divider) {
            float y = startY
                + static_cast<float>(divider) * iconSize
                + static_cast<float>(divider - 1) * iconSpacing
                + (iconSpacing * 0.5f);
            sf::RectangleShape line({ gridWidth, kDividerThickness });
            line.setPosition({ startX, y - (kDividerThickness * 0.5f) });
            line.setFillColor(dividerColor);
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
            float posX = startX + static_cast<float>(column) * (iconSize + iconSpacing);
            float posY = startY + static_cast<float>(row) * (iconSize + iconSpacing);
            sprite.setPosition({ posX + iconSize * 0.5f, posY + iconSize * 0.5f });

            sf::Color tint = sprite.getColor();
            tint.a = static_cast<std::uint8_t>(tint.a * 0.9f);
            sprite.setColor(tint);
            target.draw(sprite);
        }

        return startY + gridHeight;
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

        sf::RectangleShape popup({ width, height });
        popup.setPosition(position);
        popup.setFillColor(ColorHelper::applyAlphaFactor(sf::Color(12, 18, 40, 230), alphaFactor));
        popup.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, alphaFactor));
        popup.setOutlineThickness(2.f);
        target.draw(popup);

        const float textLeft = position.x + 16.f;
        float textY = position.y + 10.f;
        float maxTextWidth = width - 32.f;
        const auto storytellerStyle = TextStyles::speakerStyle(TextStyles::SpeakerId::StoryTeller);
        constexpr float kTitleFontSize = 30.f;
        constexpr float kTitleIconSize = 36.f;
        constexpr float kTitleIconSpacing = 8.f;
        constexpr float kTitleIconScaleFactor = 0.6f;
        constexpr float kTutorialMessageSpacing = 36.f;
        std::optional<sf::Sprite> titleIcon;
        float titleIconWidth = 0.f;
        float titleIconHeight = 0.f;
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
        if (titleIcon) {
            float iconY = textY + (kTitleFontSize - titleIconHeight) * 0.5f;
            titleIcon->setPosition({ textLeft, iconY });
            titleIcon->setColor(ColorHelper::applyAlphaFactor(sf::Color::White, alphaFactor));
            target.draw(*titleIcon);
        }
        titleText.setPosition({ titleX, textY });
        target.draw(titleText);

        float titleHeight = titleText.getLocalBounds().size.y;
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
                alphaFactor
            );
            textY += kTutorialMessageSpacing;
        }

        if (game.inventoryTutorialClosing)
            game.inventoryTutorialButtonHovered = false;
        bool hovered = game.inventoryTutorialButtonHovered && !game.inventoryTutorialClosing;
        sf::Color buttonColor = hovered ? ColorHelper::Palette::Green : TextStyles::UI::PanelDark;
        float fillAlpha = alphaFactor * (hovered ? 0.9f : 0.7f);
        buttonColor = ColorHelper::applyAlphaFactor(buttonColor, fillAlpha);
        sf::RectangleShape button({ kInventoryTutorialButtonWidth, kInventoryTutorialButtonHeight });
        button.setFillColor(buttonColor);
        button.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Amber, alphaFactor));
        button.setOutlineThickness(2.f);
        sf::Vector2f buttonPos{
            position.x + width - kInventoryTutorialButtonWidth - 16.f,
            position.y + height - kInventoryTutorialButtonHeight - 12.f
        };
        button.setPosition(buttonPos);
        target.draw(button);

        sf::Text label{ game.resources.uiFont, "Understood", 18 };
        label.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, alphaFactor));
        auto labelBounds = label.getLocalBounds();
        label.setOrigin({
            labelBounds.position.x + (labelBounds.size.x * 0.5f),
            labelBounds.position.y + (labelBounds.size.y * 0.5f)
        });
        label.setPosition({
            buttonPos.x + (kInventoryTutorialButtonWidth * 0.5f),
            buttonPos.y + (kInventoryTutorialButtonHeight * 0.5f)
        });
        target.draw(label);

        game.inventoryTutorialButtonBounds = button.getGlobalBounds();
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

    void drawQuestContent(Game& game, sf::RenderTarget& target, const sf::FloatRect& bounds) {
        sf::RectangleShape contentBox;
        contentBox.setPosition({ bounds.position.x + 12.f, bounds.position.y + 12.f });
        contentBox.setSize({ bounds.size.x - 24.f, bounds.size.y - 24.f });
        contentBox.setFillColor(sf::Color(34, 22, 12));
        contentBox.setOutlineColor(sf::Color(255, 255, 255, 180));
        contentBox.setOutlineThickness(2.f);
        target.draw(contentBox);

        sf::Text header{ game.resources.uiFont, "Questbuch", 36 };
        header.setFillColor(ColorHelper::Palette::SoftYellow);
        auto headerBounds = header.getLocalBounds();
        header.setOrigin({ headerBounds.position.x + headerBounds.size.x * 0.5f, headerBounds.position.y + headerBounds.size.y * 0.5f });
        header.setPosition({ bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + 60.f });
        target.draw(header);

        constexpr float padding = 24.f;
        constexpr float columnSpacing = 28.f;
        float columnTop = bounds.position.y + 140.f;
        float columnHeight = bounds.position.y + bounds.size.y - columnTop - 24.f;
        float columnWidth = (bounds.size.x - padding * 2.f - columnSpacing) * 0.5f;
        auto drawSection = [&](float x, const std::string& title, const std::string& description) {
            sf::RectangleShape sectionBg;
            sectionBg.setPosition({ x, columnTop });
            sectionBg.setSize({ columnWidth, columnHeight });
            sectionBg.setFillColor(sf::Color(30, 18, 10));
            sectionBg.setOutlineColor(sf::Color(255, 255, 255, 160));
            sectionBg.setOutlineThickness(1.f);
            target.draw(sectionBg);

            sf::Text label{ game.resources.uiFont, title, 28 };
            label.setFillColor(ColorHelper::Palette::SoftYellow);
            label.setPosition({ x + 16.f, columnTop + 16.f });
            target.draw(label);

            sf::Text detail{ game.resources.uiFont, description, 18 };
            detail.setFillColor(ColorHelper::Palette::Dim);
            detail.setPosition({ x + 16.f, columnTop + 50.f });
            detail.setLineSpacing(1.2f);
            target.draw(detail);
        };

        float leftX = bounds.position.x + padding;
        float rightX = leftX + columnWidth + columnSpacing;
        drawSection(
            leftX,
            "Active Quests",
            "Hier erscheinen alle Aufgaben, die du gerade verfolgst.\nWir fügen sie bald hinzu."
        );
        drawSection(
            rightX,
            "Finished Quests",
            "Abgeschlossene Abenteuer bleiben hier erhalten.\nAuch diese Liste ist in Arbeit."
        );
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

    if (auto move = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f point = game.window.mapPixelToCoords(move->position);
        if (game.menuActive) {
            updateHovered(point);
        }
        else if (menuButtonInteractable) {
            game.menuButtonHovered = game.menuButton.getGlobalBounds().contains(point);
        }
        else {
            game.menuButtonHovered = false;
        }
        if (game.inventoryTutorialPopupActive && !game.inventoryTutorialClosing) {
            game.inventoryTutorialButtonHovered = game.inventoryTutorialButtonBounds.contains(point);
        }
        else {
            game.inventoryTutorialButtonHovered = false;
        }
    }

    if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button == sf::Mouse::Button::Left) {
            sf::Vector2f point = game.window.mapPixelToCoords(button->position);

            if (!game.menuActive && menuButtonInteractable && game.menuButton.getGlobalBounds().contains(point)) {
                game.menuActive = true;
                consumed = true;
                beginInventoryTutorial(game);
            }
            else if (game.menuActive) {
                bool clickedTab = false;
                for (std::size_t idx = 0; idx < game.menuTabBounds.size(); ++idx) {
                    if (game.menuTabBounds[idx].contains(point)) {
                        game.menuActiveTab = static_cast<int>(idx);
                        clickedTab = true;
                        break;
                    }
                }

                if (clickedTab) {
                    consumed = true;
                }
                else {
                    auto panelBounds = game.menuPanel.getGlobalBounds();
                    bool insidePanel = panelBounds.contains(point);
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
                if (!game.inventoryTutorialPopupActive)
                    game.menuActive = false;
            }
            else if (menuButtonInteractable) {
                game.menuActive = true;
                consumed = true;
                beginInventoryTutorial(game);
            }
        }

        if (game.menuActive) {
            if (key->code == sf::Keyboard::Key::Right) {
                game.menuActiveTab = (game.menuActiveTab + 1) % static_cast<int>(kTabLabels.size());
                consumed = true;
            }
            else if (key->code == sf::Keyboard::Key::Left) {
                game.menuActiveTab = (game.menuActiveTab - 1 + static_cast<int>(kTabLabels.size())) % static_cast<int>(kTabLabels.size());
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

    sf::Color buttonColor = sf::Color::White;
    if (game.menuActive)
        buttonColor = ColorHelper::darken(buttonColor, 0.38f);
    else if (game.menuButtonHovered)
        buttonColor = ColorHelper::darken(buttonColor, 0.25f);

    float menuAlpha = std::clamp(game.menuButtonAlpha, 0.f, 1.f);
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
            case MenuTab::Character: icon = &game.resources.buttonCharacter; break;
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
            contentBottom = drawInventoryContent(game, target, panelBounds);
            break;
        case MenuTab::Character:
            drawPlaceholderContent(game, target, panelBounds, "Character");
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
            drawMapSelectionUI(game, target);
            target.setView(prev);
            break;
        }
        case MenuTab::Quests:
            drawQuestContent(game, target, panelBounds);
            break;
        case MenuTab::Settings:
            drawPlaceholderContent(game, target, panelBounds, "Settings");
            break;
        case MenuTab::Rankings:
            drawRankingContent(game, target, panelBounds);
            break;
    }

    drawInventoryTutorialPopup(game, target, panelBounds, contentBottom, menuFadeFactor);

    if (game.inventoryTutorialClosing && tutorialCloseProgress >= 1.f) {
        game.inventoryTutorialClosing = false;
        game.inventoryTutorialPopupActive = false;
        game.inventoryArrowActive = false;
        game.inventoryTutorialCompleted = true;
        game.menuActive = false;
        if (game.inventoryTutorialAdvancePending) {
            if (game.currentDialogue == &perigonal && game.dialogueIndex == kInventoryArrowLineIndex)
                advanceDialogueLine(game);
            game.inventoryTutorialAdvancePending = false;
        }
        game.inventoryTutorialButtonBounds = {};
        game.inventoryTutorialButtonHovered = false;
        game.inventoryTutorialCloseProgress = 0.f;
    }
}


} // namespace ui::menu
