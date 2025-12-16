// === C++ Libraries ===
#include <array>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

// === SFML Libraries ===
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>

// === Header Files ===
#include "rankingUI.hpp"
#include "../helper/colorHelper.hpp"
#include "../story/textStyles.hpp"

namespace ui::ranking {

namespace {
float clampScroll(float value) {
    return std::max(0.f, value);
}

std::string ordinalSuffix(int rank) {
    int mod100 = rank % 100;
    if (mod100 >= 11 && mod100 <= 13)
        return "th";
    switch (rank % 10) {
        case 1: return "st";
        case 2: return "nd";
        case 3: return "rd";
        default: return "th";
    }
}

std::string formatDuration(double totalSeconds) {
    int seconds = static_cast<int>(std::round(totalSeconds));
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int remain = seconds % 60;
    std::ostringstream out;
    out << std::setfill('0');
    if (hours > 0)
        out << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << remain;
    else
        out << minutes << ":" << std::setw(2) << remain;
    return out.str();
}

std::string popupMessage(const std::string& playerName, int rank) {
    std::string name = playerName.empty() ? "player" : playerName;
    return "Congratulations " + name + "! You have placed "
        + std::to_string(rank) + ordinalSuffix(rank) + " in the ranking!";
}
} // namespace

void triggerOverlay(OverlayState& state, int playerRank) {
    state.playerRank = playerRank;
    state.pending = true;
    state.visible = false;
    state.fadeAlpha = 0.f;
    state.popupAlpha = 0.f;
    state.scrollOffset = 0.f;
    state.popupClock.restart();
}

void activateOverlay(OverlayState& state, bool instant) {
    state.visible = true;
    state.pending = false;
    state.fadeClock.restart();
    state.popupClock.restart();
    state.fadeAlpha = 0.f;
    state.popupAlpha = state.playerRank > 0 ? 1.f : 0.f;
    state.showDelay = instant ? 0.f : 2.5f;
}

bool isOverlayActive(const OverlayState& state) {
    return state.visible;
}

bool handleOverlayEvent(OverlayState& state, const sf::Event& event) {
    if (!state.visible)
        return false;

    if (event.is<sf::Event::MouseWheelScrolled>()) {
        auto wheel = event.getIf<sf::Event::MouseWheelScrolled>();
        state.scrollOffset -= wheel->delta * 40.f;
        state.scrollOffset = clampScroll(state.scrollOffset);
        return true;
    }
    if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        float delta = 0.f;
        switch (key->scancode) {
            case sf::Keyboard::Scan::Escape:
                state.visible = false;
                state.pending = false;
                return true;
            case sf::Keyboard::Scan::Up: delta = -40.f; break;
            case sf::Keyboard::Scan::Down: delta = 40.f; break;
            case sf::Keyboard::Scan::PageUp: delta = -200.f; break;
            case sf::Keyboard::Scan::PageDown: delta = 200.f; break;
            default: break;
        }
        if (delta != 0.f) {
            state.scrollOffset += delta;
            state.scrollOffset = clampScroll(state.scrollOffset);
            return true;
        }
    }
    return false;
}

void updateOverlay(OverlayState& state) {
    if (!state.visible)
        return;

    float elapsed = state.fadeClock.getElapsedTime().asSeconds();
    if (elapsed < state.showDelay) {
        state.fadeAlpha = 0.f;
        return;
    }

    float fadeProgress = (elapsed - state.showDelay) / state.fadeDuration;
    state.fadeAlpha = std::clamp(fadeProgress, 0.f, 1.f);

    if (state.playerRank > 0) {
        float elapsed = state.popupClock.getElapsedTime().asSeconds();
        if (elapsed >= state.popupHoldSeconds + state.popupFadeSeconds) {
            state.popupAlpha = 0.f;
        }
        else if (elapsed >= state.popupHoldSeconds) {
            float fade = (elapsed - state.popupHoldSeconds) / state.popupFadeSeconds;
            state.popupAlpha = 1.f - std::clamp(fade, 0.f, 1.f);
        }
        else {
            state.popupAlpha = 1.f;
        }
    }
    state.scrollOffset = clampScroll(state.scrollOffset);
}

void drawOverlay(
    const OverlayState& state,
    sf::RenderTarget& target,
    const sf::Font& font,
    const std::vector<core::RankingEntry>& entries,
    int highlightedIndex,
    const std::string& playerName
) {
    if (!state.visible || state.fadeAlpha <= 0.f)
        return;

    auto size = target.getSize();
    float alpha = state.fadeAlpha;

    sf::RectangleShape backdrop({ static_cast<float>(size.x), static_cast<float>(size.y) });
    backdrop.setFillColor(ColorHelper::applyAlphaFactor(sf::Color(0, 0, 0, 200), alpha * 0.95f));
    target.draw(backdrop);

    float panelWidth = std::min(960.f, static_cast<float>(size.x) - 120.f);
    float panelHeight = std::min(560.f, static_cast<float>(size.y) - 160.f);
    sf::Vector2f panelPos{
        (static_cast<float>(size.x) - panelWidth) * 0.5f,
        (static_cast<float>(size.y) - panelHeight) * 0.5f
    };

    sf::RectangleShape panel({ panelWidth, panelHeight });
    panel.setPosition(panelPos);
    panel.setFillColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, alpha * 0.95f));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::FrameGoldLight, alpha));
    target.draw(panel);

    float padding = 22.f;
    sf::Text title{ font, "Ranking", 34 };
    title.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, alpha));
    auto titleBounds = title.getLocalBounds();
    title.setOrigin({
        titleBounds.position.x + titleBounds.size.x * 0.5f,
        titleBounds.position.y + titleBounds.size.y * 0.5f
    });
    title.setPosition({
        panelPos.x + panelWidth * 0.5f,
        panelPos.y + padding
    });
    target.draw(title);

    float headerTop = panelPos.y + padding + titleBounds.size.y + 14.f;
    sf::Text subtitle{ font, "Fastest players are listed at the top.", 20 };
    subtitle.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, alpha));
    subtitle.setPosition({
        panelPos.x + padding,
        headerTop
    });
    target.draw(subtitle);

    float listTop = headerTop + 64.f;
    float listHeight = panelHeight - (listTop - panelPos.y) - padding - 16.f;
    float listLeft = panelPos.x + padding;
    float listWidth = panelWidth - (padding * 2.f);
    float rowHeight = 40.f;
    float maxScroll = std::max(0.f, entries.size() * rowHeight - listHeight);
    float offset = std::clamp(state.scrollOffset, 0.f, maxScroll);

    sf::Text header{ font, "", 18 };
    header.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, alpha));
    float rankColumn = listLeft;
    float playerColumn = listLeft + 68.f;
    float playerWidth = listWidth * 0.5f - 88.f;
    float reducedPlayerWidth = std::max(0.f, playerWidth * 0.7f);
    float timeColumn = playerColumn + reducedPlayerWidth;
    float timeWidth = listWidth * 0.25f + 12.f;
    float reducedTimeWidth = std::max(0.f, timeWidth * 0.3f);
    float faultsColumn = timeColumn + reducedTimeWidth;
    float startedColumn = listLeft + listWidth - 160.f;
    float originalFaultWidth = startedColumn - faultsColumn;
    float reducedFaultWidth = std::max(0.f, originalFaultWidth * 0.2f);
    startedColumn = faultsColumn + reducedFaultWidth;
    std::array<float, 5> columns{
        rankColumn,
        playerColumn,
        timeColumn,
        faultsColumn,
        startedColumn
    };
    std::array<std::string, 5> labels{ "Rank", "Player", "Time", "Faults", "Started" };
    for (std::size_t idx = 0; idx < labels.size(); ++idx) {
        header.setString(labels[idx]);
        header.setPosition({
            columns[idx],
            listTop - 24.f
        });
        target.draw(header);
    }

    if (entries.empty()) {
        sf::Text empty{ font, "No completed runs yet.", 26 };
        empty.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, alpha));
        empty.setPosition({
            listLeft + (listWidth * 0.5f) - 140.f,
            listTop + listHeight * 0.5f - 12.f
        });
        target.draw(empty);
        return;
    }

    for (std::size_t idx = 0; idx < entries.size(); ++idx) {
        float y = listTop + (rowHeight * idx) - offset;
        if (y + rowHeight < listTop)
            continue;
        if (y > listTop + listHeight)
            break;

        bool highlight = static_cast<int>(idx) == highlightedIndex;
        sf::RectangleShape row({ listWidth, rowHeight - 6.f });
        row.setPosition({ listLeft, y + 3.f });
        sf::Color rowColor = highlight
            ? ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, alpha * 0.5f)
            : ColorHelper::applyAlphaFactor(TextStyles::UI::Panel, alpha * 0.65f);
        row.setFillColor(rowColor);
        target.draw(row);

        const auto& entry = entries[idx];
        auto drawColumn = [&](const std::string& value, float xPos) {
            sf::Text text{ font, value, 20 };
            text.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, alpha));
            text.setPosition({
                xPos,
                y + 8.f
            });
            target.draw(text);
        };

        drawColumn(std::to_string(idx + 1), columns[0]);
        drawColumn(entry.playerName, columns[1]);
        drawColumn(formatDuration(entry.totalSeconds), columns[2]);
        drawColumn(std::to_string(entry.faults), columns[3]);
        drawColumn(entry.started, columns[4]);
    }

    if (state.popupAlpha > 0.01f && state.playerRank > 0) {
        std::string message = popupMessage(playerName, state.playerRank);
        sf::Text popupText{ font, message, 24 };
        popupText.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, state.popupAlpha));
        auto bounds = popupText.getLocalBounds();
        sf::RectangleShape popupBg({
            bounds.size.x + 32.f,
            bounds.size.y + 24.f
        });
        popupBg.setFillColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelLight, state.popupAlpha));
        popupBg.setOutlineThickness(2.f);
        popupBg.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::FrameGoldLight, state.popupAlpha));
        popupBg.setPosition({
            panelPos.x + panelWidth * 0.5f - (popupBg.getSize().x * 0.5f),
            panelPos.y + panelHeight * 0.05f
        });

        popupText.setPosition({
            popupBg.getPosition().x + 16.f,
            popupBg.getPosition().y + 12.f
        });

        target.draw(popupBg);
        target.draw(popupText);
    }
}

} // namespace ui::ranking
