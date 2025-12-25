#pragma once
// === SFML Libraries ===
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

namespace core {

// Manages the fade-out/in overlays shown at the end of the story.
class EndSequenceController {
public:
    EndSequenceController() = default;

    // Kicks off the full end-sequence fade animation.
    void start();
    // Steps the fade timers and flips visibility flags; call every frame.
    void update();

    bool isActive() const noexcept { return active_; }
    bool isScreenVisible() const noexcept { return screenVisible_; }
    float overlayAlpha() const;
    float textAlpha() const;

private:
    bool active_ = false;              // Sequence currently animating.
    bool fadeOutActive_ = false;       // True while the screen is fading to black.
    bool fadeInActive_ = false;        // True while the end text is fading in.
    bool screenVisible_ = false;       // Set once the final screen is fully visible.
    sf::Clock clock_;                  // Tracks the current phase duration.
    float fadeOutDuration_ = 2.0f;     // How long the blackout takes.
    float fadeInDuration_ = 3.0f;      // How long the text fade-in takes.
};

} // namespace core
