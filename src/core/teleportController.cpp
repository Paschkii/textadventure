// === C++ Libraries ===
#include <algorithm>  // Provides std::min used for overlay alpha calculations.
#include <memory>     // Needed for std::make_unique storing sf::Sound instances.
// === SFML Libraries ===
#include <SFML/Audio.hpp>  // Declares sf::Sound used for teleport sound effects.
// === Header Files ===
#include "teleportController.hpp"    // Declares TeleportController class plus handleTravel.
#include "audio/audioManager.hpp"    // Controls music fading while a teleport happens.
#include "core/game.hpp"             // Gives access to Game state mutated by handleTravel.
#include "resources/resources.hpp"   // Supplies sound buffers consumed by TeleportController.
#include "story/locationDialogues.hpp"// Provides story::locationDialogueFor used when entering a map.
#include "story/storyIntro.hpp"      // Supplies encounter dialogue sequences referenced during travel.

namespace core {

// Loads teleport sound buffers from the shared resources pool.
void TeleportController::loadResources(const Resources& resources) {
    teleportStartSound_ = std::make_unique<sf::Sound>(resources.teleportStart);
    teleportMiddleSound_ = std::make_unique<sf::Sound>(resources.teleportMiddle);
    teleportStopSound_ = std::make_unique<sf::Sound>(resources.teleportStop);
}

// Starts the teleport phase machine and fades out the current music.
void TeleportController::begin(LocationId target, AudioManager& audio) {
    if (active_)
        return;

    stopSounds();
    active_ = true;
    phase_ = Phase::FadeOut;
    clock_.restart();
    middleStarted_ = false;
    stopPlayed_ = false;
    target_ = target;

    audio.fadeOutLocationMusic(fadeOutDuration_);

    if (teleportStartSound_)
        teleportStartSound_->play();
    if (teleportMiddleSound_)
        teleportMiddleSound_->stop();
    if (teleportStopSound_)
        teleportStopSound_->stop();
}

// Advances timers for each teleport phase and fires travel callbacks.
void TeleportController::update(const TravelCallback& travelCallback) {
    if (!active_ || phase_ == Phase::None)
        return;

    float elapsed = clock_.getElapsedTime().asSeconds();
    switch (phase_) {
        case Phase::FadeOut: {
            if (!middleStarted_ && elapsed >= 3.0f) {
                if (teleportMiddleSound_) {
                    teleportMiddleSound_->stop();
                    teleportMiddleSound_->play();
                }
                middleStarted_ = true;
            }

            if (elapsed >= fadeOutDuration_) {
                phase_ = Phase::Cooldown;
                clock_.restart();
                if (teleportStartSound_)
                    teleportStartSound_->stop();
                if (target_ && travelCallback)
                    travelCallback(*target_);
            }
            break;
        }
        case Phase::Cooldown: {
            if (elapsed >= cooldownDuration_) {
                phase_ = Phase::FadeIn;
                clock_.restart();
                if (!stopPlayed_) {
                    if (teleportStopSound_) {
                        teleportStopSound_->stop();
                        teleportStopSound_->play();
                    }
                    stopPlayed_ = true;
                }
            }
            break;
        }
        case Phase::FadeIn: {
            if (elapsed >= fadeInDuration_) {
                phase_ = Phase::None;
                active_ = false;
                if (teleportMiddleSound_)
                    teleportMiddleSound_->stop();
                stopPlayed_ = false;
                middleStarted_ = false;
                target_.reset();
            }
            break;
        }
        case Phase::None:
        default:
            break;
    }
}

// Returns the mask opacity that should be drawn for the current phase.
float TeleportController::overlayAlpha() const noexcept {
    switch (phase_) {
        case Phase::FadeOut:
            return std::min(1.f, clock_.getElapsedTime().asSeconds() / fadeOutDuration_);
        case Phase::Cooldown:
            return 1.f;
        case Phase::FadeIn:
            return 1.f - std::min(1.f, clock_.getElapsedTime().asSeconds() / fadeInDuration_);
        case Phase::None:
        default:
            return 0.f;
    }
}

// Stops any teleport sound that is currently playing.
void TeleportController::stopSounds() {
    if (teleportStartSound_)
        teleportStartSound_->stop();
    if (teleportMiddleSound_)
        teleportMiddleSound_->stop();
    if (teleportStopSound_)
        teleportStopSound_->stop();
}

// Swaps game state to the requested location and sets up its dialogue.
void handleTravel(Game& game, LocationId id) {
    auto locPtr = Locations::findById(game.locations, id);
    if (!locPtr)
        return;

    game.holdMapDialogue = false;

    bool allStonesCollected = game.dragonStoneCount >= 4;
    if (id == LocationId::Gonad && allStonesCollected && game.finalEncounterPending) {
        game.setCurrentLocation(locPtr);
        game.transientDialogue.clear();
        game.transientDialogue.insert(game.transientDialogue.end(), gonad_part_three.begin(), gonad_part_three.end());
        game.currentDialogue = &game.transientDialogue;
        game.dialogueIndex = 0;
        game.visibleText.clear();
        game.charIndex = 0;
        game.typewriterClock.restart();
        game.transientReturnToMap = true;
        game.pendingReturnToMenuMap = false;
        game.state = GameState::Dialogue;
        return;
    }

    if (id == LocationId::Seminiferous && game.finalEncounterPending) {
        game.finalEncounterPending = false;
        game.finalEncounterActive = true;
        game.setCurrentLocation(locPtr);

        game.transientDialogue.clear();
        game.transientDialogue.insert(game.transientDialogue.end(), seminiferous_part_one.begin(), seminiferous_part_one.end());
        game.currentDialogue = &game.transientDialogue;
        game.dialogueIndex = 0;
        game.visibleText.clear();
        game.charIndex = 0;
        game.typewriterClock.restart();
        game.state = GameState::Dialogue;
        return;
    }

    const std::vector<DialogueLine>* dialog = story::locationDialogueFor(id);
    if (!dialog)
        return;

    game.setCurrentLocation(locPtr);

    if (id == LocationId::Gonad && game.lastCompletedLocation.has_value()) {
        game.transientDialogue.clear();
        for (const auto& line : gonadWelcomeBack) {
            game.transientDialogue.push_back(line);
        }
        game.currentDialogue = &game.transientDialogue;
        game.transientReturnToMap = true;
    } else {
        game.currentDialogue = dialog;
    }

    game.dialogueIndex = 0;
    game.visibleText.clear();
    game.charIndex = 0;
    game.typewriterClock.restart();
    game.state = GameState::Dialogue;
}

} // namespace core
