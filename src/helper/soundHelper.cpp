// === C++ Libraries ===
#include <algorithm>  // Uses std::max/min when clamping durations and interpolating volumes.
// === Header Files ===
#include "helper/soundHelper.hpp"  // Declares SoundFadeState and the fade helpers implemented here.

namespace sound {

// Initializes a fade so future updates will tween the sound volume.
void startFade(SoundFadeState& fade, float startVolume, float targetVolume, float duration) {
    fade.active = true;
    fade.startVolume = startVolume;
    fade.targetVolume = targetVolume;
    fade.duration = std::max(duration, 0.f);
    fade.clock.restart();
}

// Advances the fade, applies the interpolated volume, and stops the sound if the fade finished.
void processFade(
    std::optional<sf::Sound>& sound,
    SoundFadeState& fade,
    const std::function<void()>& onStop
) {
    if (!fade.active || !sound)
        return;

    float duration = fade.duration > 0.f ? fade.duration : 0.f;
    float t = duration > 0.f
        ? std::min(1.f, fade.clock.getElapsedTime().asSeconds() / duration)
        : 1.f;
    float volume = fade.startVolume + (fade.targetVolume - fade.startVolume) * t;
    volume = std::clamp(volume, 0.f, 100.f);

    sound->setVolume(volume);

    if (t >= 1.f) {
        fade.active = false;
        if (fade.targetVolume <= 0.f) {
            sound->stop();
            onStop();
        }
    }
}

} // namespace sound
