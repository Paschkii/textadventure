#pragma once
// === C++ Libraries ===
#include <functional>  // Needs std::function for processFade's onStop callbacks.
#include <optional>    // Stores optional sf::Sound references manipulated during fades.
// === SFML Libraries ===
#include <SFML/Audio.hpp>        // Declares sf::Sound used when adjusting volumes.
#include <SFML/System/Clock.hpp> // Provides sf::Clock used to track fade progress.

// Tracks the progress of a volume fade for a single sound.
struct SoundFadeState {
    bool active = false;
    float duration = 1.f;
    float startVolume = 0.f;
    float targetVolume = 0.f;
    sf::Clock clock;
};

namespace sound {
    // Begins interpolating a sound volume from startVolume to targetVolume over duration seconds.
    void startFade(SoundFadeState& fade, float startVolume, float targetVolume, float duration);
    // Advances the fade, updates the sound's volume, and stops the sound if needed.
    void processFade(
        std::optional<sf::Sound>& sound,
        SoundFadeState& fade,
        const std::function<void()>& onStop
    );
}
