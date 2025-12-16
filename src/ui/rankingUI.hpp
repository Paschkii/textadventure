#pragma once
// === C++ Libraries ===
#include <cstdint>              // Stores the player rank number shown in the popup.
#include <string>               // Needed for the popup message.
#include <vector>               // For passing the ranking entry list to the draw helper.

// === SFML Libraries ===
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

// === Header Files ===
#include "../core/ranking.hpp"

namespace ui::ranking {

struct OverlayState {
    bool pending = false;
    bool visible = false;
    float fadeDuration = 1.0f;
    float popupHoldSeconds = 2.5f;
    float popupFadeSeconds = 0.6f;
    float fadeAlpha = 0.f;
    float popupAlpha = 0.f;
    float scrollOffset = 0.f;
    float showDelay = 2.5f;
    int playerRank = -1;
    sf::Clock fadeClock;
    sf::Clock popupClock;
};

void triggerOverlay(OverlayState& state, int playerRank);
void activateOverlay(OverlayState& state, bool instant = false);
bool isOverlayActive(const OverlayState& state);
bool handleOverlayEvent(OverlayState& state, const sf::Event& event);
void updateOverlay(OverlayState& state);
void drawOverlay(
    const OverlayState& state,
    sf::RenderTarget& target,
    const sf::Font& font,
    const std::vector<core::RankingEntry>& entries,
    int highlightedIndex,
    const std::string& playerName
);

} // namespace ui::ranking
