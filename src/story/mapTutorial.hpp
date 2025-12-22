#pragma once
// === C++ Libraries ===
#include <array>            // Stores the tutorial step metadata.
#include <optional>         // Records which location to highlight.
#include <cstddef>          // Supplies ssize_t for static array declarations.
// === SFML Libraries ===
#include <SFML/System/Vector2.hpp> // Defines sf::Vector2f for popup anchors.
#include "rendering/locations.hpp" // Provides LocationId used by the steps.

namespace StoryIntro {

struct MapTutorialStep {
    std::size_t dialogueIndex;
    std::optional<LocationId> highlightLocation;
    sf::Vector2f popupAnchorNormalized;
};

namespace MapTutorial {
    inline constexpr std::array<MapTutorialStep, 18> kSteps{{
        { 7, std::nullopt, { 0.5f, 0.15f } },
        { 8, std::nullopt, { 0.5f, 0.15f } },
        { 9, LocationId::Aerobronchi, { 0.5f, 0.15f } },
        { 10, LocationId::Aerobronchi, { 0.5f, 0.15f } },
        { 11, LocationId::Aerobronchi, { 0.5f, 0.15f } },
        { 12, LocationId::Aerobronchi, { 0.5f, 0.15f } },
        { 13, LocationId::Blyathyroid, { 0.25f, 0.5f } },
        { 14, LocationId::Blyathyroid, { 0.25f, 0.5f } },
        { 15, LocationId::Blyathyroid, { 0.25f, 0.5f } },
        { 16, LocationId::Blyathyroid, { 0.25f, 0.5f } },
        { 17, LocationId::Cladrenal, { 0.75f, 0.5f } },
        { 18, LocationId::Cladrenal, { 0.75f, 0.5f } },
        { 19, LocationId::Cladrenal, { 0.75f, 0.5f } },
        { 20, LocationId::Cladrenal, { 0.75f, 0.5f } },
        { 21, LocationId::Lacrimere, { 0.5f, 0.8f } },
        { 22, LocationId::Lacrimere, { 0.5f, 0.8f } },
        { 23, LocationId::Lacrimere, { 0.5f, 0.8f } },
        { 24, LocationId::Lacrimere, { 0.5f, 0.8f } },
    }};

    inline constexpr std::size_t kStartIndex = kSteps.front().dialogueIndex;
    inline constexpr std::size_t kEndIndex = kSteps.back().dialogueIndex;

    inline std::optional<std::size_t> stepIndexFor(std::size_t dialogueIndex) {
        if (dialogueIndex < kStartIndex || dialogueIndex > kEndIndex)
            return std::nullopt;
        return dialogueIndex - kStartIndex;
    }

    inline const MapTutorialStep& step(std::size_t index) {
        return kSteps[index];
    }
}

} // namespace StoryIntro
