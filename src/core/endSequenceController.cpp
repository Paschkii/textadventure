// === C++ Libraries ===
#include <algorithm>
// === Header Files ===
#include "core/endSequenceController.hpp"

namespace core {

// Resets flags and begins the end-sequence fade timeline.
void EndSequenceController::start() {
    active_ = true;
    fadeOutActive_ = true;
    fadeInActive_ = false;
    screenVisible_ = false;
    clock_.restart();
}

// Advances the fade states so the overlay and text transition complete.
void EndSequenceController::update() {
    if (!active_ && !screenVisible_)
        return;

    float elapsed = clock_.getElapsedTime().asSeconds();
    if (fadeOutActive_) {
        float progress = std::min(1.f, elapsed / fadeOutDuration_);
        if (progress >= 1.f) {
            fadeOutActive_ = false;
            fadeInActive_ = true;
            clock_.restart();
        }
        return;
    }

    if (fadeInActive_) {
        float progress = std::min(1.f, elapsed / fadeInDuration_);
        if (progress >= 1.f) {
            fadeInActive_ = false;
            active_ = false;
            screenVisible_ = true;
        }
        return;
    }
}

// Reports how opaque the overlay should be during the current phase.
float EndSequenceController::overlayAlpha() const {
    if (!active_ && !screenVisible_)
        return 0.f;
    if (fadeOutActive_) {
        return std::min(1.f, clock_.getElapsedTime().asSeconds() / fadeOutDuration_);
    }
    if (fadeInActive_) {
        return 1.f - std::min(1.f, clock_.getElapsedTime().asSeconds() / fadeInDuration_);
    }
    return 1.f;
}

// Reports how opaque the end-screen text should be right now.
float EndSequenceController::textAlpha() const {
    if (fadeInActive_) {
        return std::min(1.f, clock_.getElapsedTime().asSeconds() / fadeInDuration_);
    }
    if (screenVisible_)
        return 1.f;
    return 0.f;
}

} // namespace core
