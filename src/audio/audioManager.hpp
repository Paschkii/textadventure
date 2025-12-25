#pragma once
// === C++ Libraries ===
#include <optional>  // Captures optional sound handles and the current location ID.
// === SFML Libraries ===
#include <SFML/Audio.hpp>  // Declares sf::Sound and sf::SoundBuffer used by the manager.
// === Header Files ===
#include "helper/soundHelper.hpp"      // Offers the fade helpers used when ramping volumes.
#include "rendering/locations.hpp"     // Provides LocationId for selecting music per location.
#include "resources/resources.hpp"     // Supplies the Resources struct that holds all buffers.

class AudioManager {
public:
    AudioManager() = default;

    void init(Resources& resources);

    void startLocationMusic(LocationId id);
    void fadeOutLocationMusic(float duration);

    void startTitleScreenMusic();
    void fadeOutTitleScreenMusic(float duration);
    void startIntroDialogueMusic();
    void stopIntroDialogueMusic();

    float locationMusicVolume() const;
    bool hasLocationMusic() const;
    void setLocationMusicVolume(float volume);
    void cancelLocationMusicFade();

    void update();

    void startTypingSound();
    void stopTypingSound();

private:
    const sf::SoundBuffer* locationMusicBuffer(LocationId id) const;

    Resources* resources = nullptr;
    std::optional<sf::Sound> textBlipSound;
    std::optional<sf::Sound> locationMusic;
    std::optional<LocationId> locationMusicId;
    SoundFadeState locationMusicFade;
    std::optional<sf::Sound> titleScreenSound;
    std::optional<sf::Sound> introDialogueSound;
    SoundFadeState titleScreenFade;
    bool titleScreenMusicStarted = false;
};
