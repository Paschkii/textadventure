// === C++ Libraries ===
#include <array>      // Stores the fixed-size tab lists.
#include <cmath>      // Rounds time values when formatting durations.
#include <iomanip>    // Formats elapsed time for rankings.
#include <sstream>    // Builds ranking/time strings.

// === SFML Libraries ===
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

// === Header Files ===
#include "core/game.hpp"
#include "core/ranking.hpp"
#include "helper/colorHelper.hpp"
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

    std::string formatTime(double totalSeconds) {
        int seconds = static_cast<int>(std::round(totalSeconds));
        int minutes = seconds / 60;
        seconds %= 60;
        std::ostringstream out;
        out << minutes << ':';
        out << std::setfill('0') << std::setw(2) << seconds;
        return out.str();
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

    if (auto move = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f point = game.window.mapPixelToCoords(move->position);
        if (game.menuActive)
            updateHovered(point);
        else
            game.menuButtonHovered = game.menuButton.getGlobalBounds().contains(point);
    }

    if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button == sf::Mouse::Button::Left) {
            sf::Vector2f point = game.window.mapPixelToCoords(button->position);

            if (!game.menuActive && game.menuButton.getGlobalBounds().contains(point)) {
                game.menuActive = true;
                consumed = true;
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
                    bool insidePanel = game.menuPanel.getGlobalBounds().contains(point);
                    if (!insidePanel)
                        game.menuActive = false;
                    consumed = true;
                }
            }
        }
    }

    if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            if (game.menuActive)
                game.menuActive = false;
            else {
                game.menuActive = true;
                consumed = true;
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
    if (game.menuButton.getSize().x <= 0.f || game.menuButton.getSize().y <= 0.f)
        return;

    sf::Color buttonColor = sf::Color::White;
    if (game.menuActive)
        buttonColor = ColorHelper::darken(buttonColor, 0.38f);
    else if (game.menuButtonHovered)
        buttonColor = ColorHelper::darken(buttonColor, 0.25f);

    game.menuButton.setFillColor(buttonColor);
    target.draw(game.menuButton);

    if (!game.menuActive)
        return;

    sf::Vector2u windowSize = target.getSize();
    sf::RectangleShape overlay({ static_cast<float>(windowSize.x), static_cast<float>(windowSize.y) });
    overlay.setFillColor(ColorHelper::Palette::Overlay);
    target.draw(overlay);

    // Draw tabs
    float tabFade = game.menuActive ? 1.f : 0.f;
    for (std::size_t idx = 0; idx < kTabLabels.size(); ++idx) {
        const auto& bounds = game.menuTabBounds[idx];
        if (bounds.size.x <= 0.f || bounds.size.y <= 0.f)
            continue;

        sf::RectangleShape tabBackground({ bounds.size.x, bounds.size.y });
        tabBackground.setPosition({ bounds.position.x, bounds.position.y });
        tabBackground.setFillColor(sf::Color(30, 24, 15, 200));
        tabBackground.setOutlineColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(tabFade * 200)));
        tabBackground.setOutlineThickness(1.f);
        if (static_cast<int>(idx) == game.menuActiveTab || static_cast<int>(idx) == game.menuHoveredTab) {
            tabBackground.setFillColor(ColorHelper::applyAlphaFactor(sf::Color::White, 0.15f));
            tabBackground.setOutlineThickness(2.f);
        }
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

        if (icon) {
            sf::Sprite sprite(*icon);
            float iconSize = std::min(bounds.size.y * 0.6f, 48.f);
            if (icon->getSize().y > 0)
                sprite.setScale(sf::Vector2f{ iconSize / static_cast<float>(icon->getSize().y), iconSize / static_cast<float>(icon->getSize().y) });
            sprite.setPosition({
                bounds.position.x + 12.f,
                bounds.position.y + (bounds.size.y - iconSize) * 0.5f
            });
            target.draw(sprite);
        }

        sf::Text label{ game.resources.uiFont, kTabLabels[idx], 20 };
        bool active = static_cast<int>(idx) == game.menuActiveTab;
        label.setFillColor(active ? ColorHelper::Palette::SoftYellow : ColorHelper::Palette::Normal);
        auto labelBounds = label.getLocalBounds();
        label.setOrigin({ 0.f, labelBounds.position.y + labelBounds.size.y * 0.5f });
        float labelX = bounds.position.x + 60.f;
        float labelY = bounds.position.y + bounds.size.y * 0.5f;
        label.setPosition({ labelX, labelY });
        target.draw(label);
    }

    game.menuPanel.setFillColor(sf::Color(54, 33, 18));
    game.menuPanel.setOutlineColor(sf::Color::White);
    game.menuPanel.setOutlineThickness(3.f);
    target.draw(game.menuPanel);

    auto panelBounds = game.menuPanel.getGlobalBounds();
    switch (static_cast<MenuTab>(game.menuActiveTab)) {
        case MenuTab::Inventory:
            drawPlaceholderContent(game, target, panelBounds, "Inventory");
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
            drawPlaceholderContent(game, target, panelBounds, "Quests");
            break;
        case MenuTab::Settings:
            drawPlaceholderContent(game, target, panelBounds, "Settings");
            break;
        case MenuTab::Rankings:
            drawRankingContent(game, target, panelBounds);
            break;
    }
}

} // namespace ui::menu
