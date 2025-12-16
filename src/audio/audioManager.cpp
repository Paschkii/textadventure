// === SFML Libraries ===
#include <SFML/Audio.hpp>    // Brings sf::Sound and sf::SoundBuffer used inside the implementation.
// === Header Files ===
#include "audio/audioManager.hpp"  // Declares AudioManager whose member functions are defined here.

const sf::SoundBuffer* AudioManager::locationMusicBuffer(LocationId id) const {
    if (!resources)
        return nullptr;

    switch (id) {
        case LocationId::Gonad: return &resources->locationMusicGonad;
        case LocationId::Blyathyroid: return &resources->locationMusicBlyathyroid;
        case LocationId::Lacrimere: return &resources->locationMusicLacrimere;
        case LocationId::Cladrenal: return &resources->locationMusicCladrenal;
        case LocationId::Aerobronchi: return &resources->locationMusicAerobronchi;
        case LocationId::Seminiferous: return &resources->locationMusicSeminiferous;
        default: return nullptr;
    }
}

void AudioManager::init(Resources& res) {
    resources = &res;
    if (!textBlipSound)
        textBlipSound.emplace(resources->typewriter);

    if (!titleScreenSound) {
        titleScreenSound.emplace(resources->introTitle);
        titleScreenSound->setLooping(true);
        titleScreenSound->setVolume(0.f);
    }
}

void AudioManager::startLocationMusic(LocationId id) {
    if (!resources)
        return;

    const sf::SoundBuffer* buffer = locationMusicBuffer(id);
    if (!buffer)
        return;

    locationMusicFade.active = false;

    if (!locationMusic)
        locationMusic.emplace(*buffer);
    else
        locationMusic->setBuffer(*buffer);

    locationMusic->setLooping(true);
    locationMusic->setVolume(0.f);
    locationMusic->play();
    locationMusicId = id;

    sound::startFade(locationMusicFade, 0.f, 100.f, 1.5f);
}

void AudioManager::fadeOutLocationMusic(float duration) {
    if (!locationMusic || locationMusic->getStatus() != sf::Sound::Status::Playing)
        return;

    float startVol = locationMusic->getVolume();
    if (startVol <= 0.f) {
        locationMusic->stop();
        locationMusicId.reset();
        return;
    }

    sound::startFade(locationMusicFade, startVol, 0.f, duration);
}

void AudioManager::startTitleScreenMusic() {
    if (titleScreenMusicStarted || !titleScreenSound)
        return;

    titleScreenSound->setLooping(true);
    titleScreenSound->setVolume(0.f);
    titleScreenSound->play();
    sound::startFade(titleScreenFade, 0.f, 100.f, 2.0f);
    titleScreenMusicStarted = true;
}

void AudioManager::fadeOutTitleScreenMusic(float duration) {
    if (!titleScreenSound)
        return;

    float startVol = titleScreenSound->getVolume();
    if (titleScreenSound->getStatus() != sf::Sound::Status::Playing && startVol <= 0.f)
        return;

    if (startVol <= 0.f) {
        titleScreenSound->stop();
        return;
    }

    sound::startFade(titleScreenFade, startVol, 0.f, duration);
}

float AudioManager::locationMusicVolume() const {
    if (!locationMusic)
        return 100.f;
    return locationMusic->getVolume();
}

bool AudioManager::hasLocationMusic() const {
    return static_cast<bool>(locationMusic);
}

void AudioManager::setLocationMusicVolume(float volume) {
    if (!locationMusic)
        return;
    locationMusic->setVolume(volume);
}

void AudioManager::cancelLocationMusicFade() {
    locationMusicFade.active = false;
}

void AudioManager::startIntroDialogueMusic() {
    if (!resources)
        return;

    if (introDialogueSound && introDialogueSound->getStatus() == sf::Sound::Status::Playing)
        return;

    if (!introDialogueSound)
        introDialogueSound.emplace(resources->introDialogue);
    else
        introDialogueSound->setBuffer(resources->introDialogue);

    introDialogueSound->setLooping(true);
    introDialogueSound->setVolume(80.f);
    introDialogueSound->play();
}

void AudioManager::stopIntroDialogueMusic() {
    if (!introDialogueSound)
        return;
    introDialogueSound->stop();
}

void AudioManager::update() {
    sound::processFade(locationMusic, locationMusicFade, [this]() { locationMusicId.reset(); });
    sound::processFade(titleScreenSound, titleScreenFade, []() {});
}

void AudioManager::startTypingSound() {
    if (!textBlipSound)
        return;

    if (textBlipSound->getStatus() != sf::Sound::Status::Playing)
        textBlipSound->play();
}

void AudioManager::stopTypingSound() {
    if (textBlipSound && textBlipSound->getStatus() == sf::Sound::Status::Playing)
        textBlipSound->stop();
}
