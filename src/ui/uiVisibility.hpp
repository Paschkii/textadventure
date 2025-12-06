#pragma once
#include <cstdint>

class Game;

// BitMask for UI Elements
enum class UiElement : std::uint32_t {
    None                = 0,
    TextBox             = 1u << 0,
    NameBox             = 1u << 1,
    LocationBox         = 1u << 2,
    IntroTitle          = 1u << 3,
    MapSelectionUI      = 1u << 4,
    QuizUI              = 1u << 5,
    WeaponSelectionUI   = 1u << 6,
    All                 = 0xFFFFFFFu,
};

using UiElementMask = std::uint32_t;

constexpr UiElementMask operator|(UiElement lhs, UiElement rhs) {
    return static_cast<UiElementMask>(lhs) | static_cast<UiElementMask>(rhs);
}

constexpr UiElementMask operator|(UiElementMask lhs, UiElement rhs) {
    return lhs | static_cast<UiElementMask>(rhs);
}

constexpr bool hasElement(UiElementMask mask, UiElement element) {
    return (mask & static_cast<UiElementMask>(element)) != 0u;
}

struct UiVisibility {
    float alphaFactor = 1.f;
    bool hidden = false;
    bool backgroundFadeTriggered = false;
};

/*
Computes visibility flags for UI components that opt-in via the provided mask.

The mask is used to describe which UI group(s) should respect the intro fade/hide logic.
Modules should pass the set of elements they are about to draw (e.g., TextBox | NameBox)
and use the returned alphaFactor/hidden values when rendering their assets. Future UI
modules like MapSelectionUI, QuizUI, or WeaponSelectionUI can reuse this helper to decide
when to skip drawing or fade their visuals in tandem with the intro dialogue.
*/

UiVisibility computeUiVisibility(Game& game, UiElementMask elements);