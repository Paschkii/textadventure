#pragma once
// === C++ Libraries ===
#include <functional>  // Provides TravelCallback's std::function for signaling destinations.
#include <memory>      // Stores sound handles in std::unique_ptr instances.
#include <optional>    // Tracks optional teleport targets and state.
// === SFML Libraries ===
#include <SFML/Audio.hpp>       // Declares sf::Sound used by the teleport sequence.
#include <SFML/System/Clock.hpp>  // Gives sf::Clock for timing phase transitions.
// === Header Files ===
#include "rendering/locations.hpp"  // Supplies LocationId and location helpers used in handleTravel.

struct AudioManager;
struct Resources;
struct Game;

namespace core {

// TeleportController manages the teleport sequence timers, audio, and masking so
// that the main game logic can focus on state transitions and UI rendering.
class TeleportController {
public:
    enum class Phase {
        None,
        FadeOut,
        Cooldown,
        FadeIn,
    };

    using TravelCallback = std::function<void(LocationId)>;

    TeleportController() = default;

    // Loads the teleport sound buffers from the central resources manager.
    void loadResources(const Resources& resources);

    // Starts a teleport towards the given target and lets the provided audio manager
    // handle the music fade-out.
    void begin(LocationId target, AudioManager& audio);

    // Updates the teleport timers, plays the appropriate sounds, and invokes the
    // callback as soon as the teleport destination should be reached.
    void update(const TravelCallback& travelCallback);

    bool active() const noexcept { return active_; }
    Phase phase() const noexcept { return phase_; }
    float fadeOutDuration() const noexcept { return fadeOutDuration_; }
    float fadeInDuration() const noexcept { return fadeInDuration_; }

    // Computes the overlay opacity that should be rendered on top of the scene
    // while the teleport sequence is running.
    float overlayAlpha() const noexcept;

private:
    void stopSounds();

    Phase phase_ = Phase::None;
    bool active_ = false;
    sf::Clock clock_;
    float fadeOutDuration_ = 3.0f;
    float cooldownDuration_ = 2.0f;
    float fadeInDuration_ = 1.0f;
    std::optional<LocationId> target_;
    bool middleStarted_ = false;
    bool stopPlayed_ = false;

    std::unique_ptr<sf::Sound> teleportStartSound_;
    std::unique_ptr<sf::Sound> teleportMiddleSound_;
    std::unique_ptr<sf::Sound> teleportStopSound_;
};

// Updates game state and dialogue when a travel destination is reached.
void handleTravel(Game& game, LocationId id);

} // namespace core
