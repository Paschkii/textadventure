#include "game.hpp"
#include "story/dialogueLine.hpp"
#include "story/dialogInput.hpp"
#include "rendering/dialogRender.hpp"
#include "story/storyIntro.hpp"
#include "story/textStyles.hpp"
#include "ui/confirmationUI.hpp"
#include "ui/introTitle.hpp"
#include "ui/weaponSelectionUI.hpp"
#include "ui/mapSelectionUI.hpp"
#include "ui/quizUI.hpp"
#include "helper/layoutHelpers.hpp"
#include "helper/colorHelper.hpp"
#include "helper/weaponHelpers.hpp"
#include "helper/dragonHelpers.hpp"
#include <iostream>
#include <algorithm>
#include <filesystem>

constexpr unsigned int windowWidth = 1280;
constexpr unsigned int windowHeight = 720;
constexpr unsigned int fpsLimit = 60;
constexpr std::size_t playerNameMaxLength = 18;

namespace {
    const sf::SoundBuffer* locationMusicBuffer(const Resources& resources, LocationId id) {
        switch (id) {
            case LocationId::Gonad: return &resources.locationMusicGonad;
            case LocationId::Blyathyroid: return &resources.locationMusicBlyathyroid;
            case LocationId::Lacrimere: return &resources.locationMusicLacrimere;
            case LocationId::Cladrenal: return &resources.locationMusicCladrenal;
            case LocationId::Aerobronchi: return &resources.locationMusicAerobronchi;
            case LocationId::MasterBatesStronghold: return &resources.locationMusicSeminiferous;
            default: return nullptr;
        }
    }

    void startFade(SoundFadeState& fade, float startVolume, float targetVolume, float duration) {
        fade.active = true;
        fade.startVolume = startVolume;
        fade.targetVolume = targetVolume;
        fade.duration = std::max(duration, 0.f);
        fade.clock.restart();
    }
}

Game::Game()
: window(sf::VideoMode({windowWidth, windowHeight}), "Glandular", sf::Style::Titlebar | sf::Style::Close)
{
    if (!resources.loadAll()) {
        std::cout << "Fatal: konnte Ressourcen nicht laden.\n";
        std::exit(1);
    }
    background.emplace(resources.introBackground);
    returnSprite.emplace(resources.returnSymbol);
    returnSprite->setColor(ColorHelper::Palette::IconGray);

    locations = Locations::buildLocations(resources);

    textBlipSound.emplace(resources.typewriter);
    enterSound.emplace(resources.enterKey);
    confirmSound.emplace(resources.confirm);
    rejectSound.emplace(resources.reject);
    startGameSound.emplace(resources.startGame);
    acquireSound.emplace(resources.acquire);
    teleportStartSound.emplace(resources.teleportStart);
    teleportMiddleSound.emplace(resources.teleportMiddle);
    teleportStopSound.emplace(resources.teleportStop);
    quizLoggingSound.emplace(resources.quizLoggingAnswer);
    quizCorrectSound.emplace(resources.quizAnswerCorrect);
    quizIncorrectSound.emplace(resources.quizAnswerIncorrect);
    quizStartSound.emplace(resources.quizStart);
    quizQuestionStartSound.emplace(resources.quizQuestionStart);
    quizQuestionThinkingSound.emplace(resources.quizQuestionThinking);
    quizEndSound.emplace(resources.quizEnd);
    titleScreenSound.emplace(resources.titleScreen);
    titleScreenSound->setLooping(true);
    titleScreenSound->setVolume(0.f);
    // === Framerate limitieren ===
    window.setFramerateLimit(fpsLimit);
    // === NameBox Style setzen ===
    nameBox.setFillColor(sf::Color::Transparent);
    nameBox.setOutlineColor(ColorHelper::Palette::Normal);
    nameBox.setOutlineThickness(2.f);
    // === TextBox Style setzen ===
    textBox.setFillColor(sf::Color::Transparent);
    textBox.setOutlineColor(ColorHelper::Palette::Normal);
    textBox.setOutlineThickness(2.f);
    // === LocationBox Style setzen ===
    locationBox.setFillColor(sf::Color::Transparent);
    locationBox.setOutlineColor(ColorHelper::Palette::Normal);
    locationBox.setOutlineThickness(2.f);
    // === ItemBox Style setzen ===
    itemBox.setFillColor(sf::Color::Transparent);
    itemBox.setOutlineColor(ColorHelper::Palette::Normal);
    itemBox.setOutlineThickness(2.f);

    weaponPanel.setFillColor(sf::Color::Transparent);
    weaponPanel.setOutlineColor(ColorHelper::Palette::Normal);
    weaponPanel.setOutlineThickness(2.f);

    currentDialogue = &intro;

    ui::weapons::loadWeaponOptions(*this);
    ui::dragons::loadDragonPortraits(*this);
}

namespace {
    const std::vector<DialogueLine>* dialogueForLocation(LocationId id) {
        switch (id) {
            case LocationId::Blyathyroid: return &firedragon;
            case LocationId::Aerobronchi: return &airdragon;
            case LocationId::Lacrimere: return &waterdragon;
            case LocationId::Cladrenal: return &earthdragon;
            case LocationId::Gonad: return &gonad;
            default: return nullptr;
        }
    }
}

void Game::setCurrentLocation(const Location* location) {
    currentLocation = location;
    if (location)
        startLocationMusic(location->id);
}

void Game::startLocationMusic(LocationId id) {
    const sf::SoundBuffer* buffer = locationMusicBuffer(resources, id);
    if (!buffer)
        return;

    locationMusicFade.active = false; // reset any old fades before starting a new track

    if (!locationMusic)
        locationMusic.emplace(*buffer);
    else
        locationMusic->setBuffer(*buffer);

    locationMusic->setLooping(true);
    locationMusic->setVolume(0.f);
    locationMusic->play();
    locationMusicId = id;

    startFade(locationMusicFade, 0.f, 100.f, 1.5f);
}

void Game::fadeOutLocationMusic(float duration) {
    if (!locationMusic || locationMusic->getStatus() != sf::Sound::Status::Playing)
        return;

    float startVol = locationMusic->getVolume();
    if (startVol <= 0.f) {
        locationMusic->stop();
        locationMusicId.reset();
        return;
    }

    startFade(locationMusicFade, startVol, 0.f, duration);
}

void Game::startTitleScreenMusic() {
    if (titleScreenMusicStarted || !titleScreenSound)
        return;

    titleScreenSound->setLooping(true);
    titleScreenSound->setVolume(0.f);
    titleScreenSound->play();
    startFade(titleScreenFade, 0.f, 100.f, 2.0f);
    titleScreenMusicStarted = true;
}

void Game::fadeOutTitleScreenMusic(float duration) {
    if (!titleScreenSound)
        return;

    float startVol = titleScreenSound->getVolume();
    if (titleScreenSound->getStatus() != sf::Sound::Status::Playing && startVol <= 0.f)
        return;

    if (startVol <= 0.f) {
        titleScreenSound->stop();
        return;
    }

    startFade(titleScreenFade, startVol, 0.f, duration);
}

void Game::updateSoundFades() {
    auto applyFade = [](std::optional<sf::Sound>& sound, SoundFadeState& fade, auto onStop) {
        if (!fade.active || !sound)
            return;

        float duration = fade.duration > 0.f ? fade.duration : 0.f;
        float t = duration > 0.f ? std::min(1.f, fade.clock.getElapsedTime().asSeconds() / duration) : 1.f;
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
    };

    applyFade(locationMusic, locationMusicFade, [this]() { locationMusicId.reset(); });
    applyFade(titleScreenSound, titleScreenFade, []() {});
}

void Game::startTravel(LocationId id) {
    auto locPtr = Locations::findById(locations, id);
    const std::vector<DialogueLine>* dialog = dialogueForLocation(id);
    if (!locPtr || !dialog)
        return;

    holdMapDialogue = false;

    bool allStonesCollected = dragonStoneCount >= 4;
    if (id == LocationId::Gonad && allStonesCollected && finalEncounterPending) {
        finalEncounterPending = false;
        finalEncounterActive = true;
        if (auto stronghold = Locations::findById(locations, LocationId::MasterBatesStronghold))
            setCurrentLocation(stronghold);
        else
            setCurrentLocation(locPtr);

        transientDialogue.clear();
        transientDialogue.insert(transientDialogue.end(), finalEncounter.begin(), finalEncounter.end());
        currentDialogue = &transientDialogue;
        dialogueIndex = 0;
        visibleText.clear();
        charIndex = 0;
        typewriterClock.restart();
        state = GameState::Dialogue;
        return;
    }

    setCurrentLocation(locPtr);

    if (id == LocationId::Gonad && lastCompletedLocation.has_value()) {
        transientDialogue.clear();
        for (const auto& line : gonadWelcomeBack) {
            transientDialogue.push_back(line);
        }
        currentDialogue = &transientDialogue;
        transientReturnToMap = true;
    } else {
        currentDialogue = dialog;
    }

    dialogueIndex = 0;
    visibleText.clear();
    charIndex = 0;
    typewriterClock.restart();
    state = GameState::Dialogue;
}

void Game::beginTeleport(LocationId id) {
    if (teleportActive)
        return;

    stopTypingSound();
    transientReturnToMap = false;
    fadeOutLocationMusic(teleportFadeOutDuration);
    teleportTarget = id;
    teleportPhase = TeleportPhase::FadeOut;
    teleportActive = true;
    teleportClock.restart();
    teleportMiddleStarted = false;
    teleportStopPlayed = false;

    if (teleportStartSound) {
        teleportStartSound->stop();
        teleportStartSound->play();
    }
    if (teleportMiddleSound)
        teleportMiddleSound->stop();
    if (teleportStopSound)
        teleportStopSound->stop();
}

void Game::updateTeleport() {
    if (!teleportActive || teleportPhase == TeleportPhase::None)
        return;

    float elapsed = teleportClock.getElapsedTime().asSeconds();

    if (teleportPhase == TeleportPhase::FadeOut) {
        if (!teleportMiddleStarted && elapsed >= 3.0f && teleportMiddleSound) {
            teleportMiddleSound->stop();
            teleportMiddleSound->play();
            teleportMiddleStarted = true;
        }

        if (elapsed >= teleportFadeOutDuration) {
            teleportPhase = TeleportPhase::Cooldown;
            teleportClock.restart();

            if (teleportStartSound)
                teleportStartSound->stop();

            if (teleportTarget)
                startTravel(*teleportTarget);
        }
    }
    else if (teleportPhase == TeleportPhase::Cooldown) {
        if (elapsed >= teleportCooldownDuration) {
            teleportPhase = TeleportPhase::FadeIn;
            teleportClock.restart();

            if (teleportStopSound && !teleportStopPlayed) {
                teleportStopSound->stop();
                teleportStopSound->play();
                teleportStopPlayed = true;
            }
        }
    }
    else if (teleportPhase == TeleportPhase::FadeIn) {
        if (elapsed >= teleportFadeInDuration) {
            teleportPhase = TeleportPhase::None;
            teleportActive = false;
            if (teleportMiddleSound)
                teleportMiddleSound->stop();
            teleportTarget.reset();
            teleportMiddleStarted = false;
            teleportStopPlayed = false;
        }
    }
}

void Game::updateEndSequence() {
    if (!endSequenceActive && !endScreenVisible)
        return;

    float elapsed = endClock.getElapsedTime().asSeconds();

    if (endFadeOutActive) {
        float progress = std::min(1.f, elapsed / endFadeOutDuration);
        if (progress >= 1.f) {
            endFadeOutActive = false;
            endFadeInActive = true;
            endClock.restart();
        }
    }
    else if (endFadeInActive) {
        float progress = std::min(1.f, elapsed / endFadeInDuration);
        if (progress >= 1.f) {
            endFadeInActive = false;
            endSequenceActive = false;
            endScreenVisible = true;
        }
    }
}

sf::Color Game::frameBaseColor() const {
    if (currentLocation && currentLocation->id == LocationId::MasterBatesStronghold)
        return ColorHelper::Palette::DarkPurple;

    return TextStyles::UI::PanelDark;
}

sf::Color Game::frameColor(float uiAlphaFactor) const {
    return ColorHelper::applyAlphaFactor(frameBaseColor(), uiAlphaFactor);
}

void Game::addItemIcon(const sf::Texture& texture) {
    itemIcons.emplace_back(texture);
    if (acquireSound) {
        acquireSound->stop();
        acquireSound->play();
    }
}

void Game::addDragonstoneIcon(LocationId id) {
    const sf::Texture* tex = nullptr;
    std::size_t idx = 0;
    switch (id) {
        case LocationId::Blyathyroid:
            tex = &resources.dragonstoneFire;
            idx = 0;
            break;
        case LocationId::Aerobronchi:
            tex = &resources.dragonstoneAir;
            idx = 1;
            break;
        case LocationId::Lacrimere:
            tex = &resources.dragonstoneWater;
            idx = 2;
            break;
        case LocationId::Cladrenal:
            tex = &resources.dragonstoneEarth;
            idx = 3;
            break;
        default:
            return;
    }

    if (!tex || idx >= dragonstoneIconAdded.size() || dragonstoneIconAdded[idx])
        return;

    dragonstoneIconAdded[idx] = true;
    addItemIcon(*tex);
}

// === Layout updaten je nach Fenstergröße ===
void Game::updateLayout() {
    ui::layout::updateLayout(*this);
}


// === Öffnet das Fenster und führt Tasks aus ===
void Game::run() {
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            bool eventConsumed = false;

            if (event->is<sf::Event::Closed>()) {
                window.close();
                continue;
            }

            if (teleportActive || endSequenceActive || endScreenVisible)
                continue;

            bool confirmationHandled = confirmationPrompt.active && handleConfirmationEvent(*this, *event);
            if (confirmationHandled)
                continue;

            if (confirmationPrompt.active && event->is<sf::Event::TextEntered>())
                continue;

            if (askingName && event->is<sf::Event::TextEntered>()) {
                const auto& text = event->getIf<sf::Event::TextEntered>();
                char32_t code = text->unicode;

                if (code == U'\r' || code == U'\n') {}
                else if (code == U'\b') {
                    if (!nameInput.empty())
                        nameInput.pop_back();
                }
                else if (code >= 32 && code < 127) {
                    if (nameInput.size() < playerNameMaxLength)
                        nameInput.push_back(static_cast<char>(code));
                }
            }

            if (auto key = event->getIf<sf::Event::KeyReleased>()) {
                if (key->scancode == sf::Keyboard::Scan::Enter) {
                    // Block Enter key while a confirmation prompt is active to avoid
                    // accidental confirmation via keyboard (prevent misclicks).
                    if (confirmationPrompt.active)
                        continue;

                    if (state == GameState::IntroTitle) {
                        if (titleDropStarted && !introTitleDropComplete(*this))
                            continue;

                        if (introPromptBlinkActive) {
                            if (!introPromptInputEnabled)
                                continue;

                            introPromptBlinkActive = false;
                            introPromptVisible = true;
                            introPromptFade = 1.f;
                        }
                    }

                    // Enter sound is played only when a new dialogue line actually starts.
                    // The logic for playing the sound is handled inside `waitForEnter`.

                    if (state == GameState::IntroScreen) {
                        if (!introFadeOutActive) {
                            introFadeOutActive = true;
                            introClock.restart();
                        }
                        eventConsumed = true;
                    }
                    else if (state == GameState::IntroTitle) {
                        if (introDialogueFinished && introTitleDropComplete(*this)) {
                            triggerIntroTitleExit(*this);
                            eventConsumed = true;
                        }
                        else if (currentDialogue && dialogueIndex < currentDialogue->size()) {
                            eventConsumed = waitForEnter(*this, (*currentDialogue)[dialogueIndex]);
                        }
                        // If the intro dialogue finished and we are waiting for the title
                        // drop to complete, ignore Enter to avoid null dialogue access.
                    }
                    else if (state == GameState::Dialogue || state == GameState::MapSelection) {
                        if (currentDialogue && dialogueIndex < currentDialogue->size()) {
                            eventConsumed = waitForEnter(*this, (*currentDialogue)[dialogueIndex]);
                        }
                    }
                }
            }

            if (eventConsumed)
                continue;

            if (state == GameState::WeaponSelection && !confirmationPrompt.active)
                handleWeaponSelectionEvent(*this, *event);
            else if (state == GameState::MapSelection && !confirmationPrompt.active)
                handleMapSelectionEvent(*this, *event);
            else if (state == GameState::Quiz && !confirmationPrompt.active)
                handleQuizEvent(*this, *event);
            else if (state == GameState::FinalChoice && !confirmationPrompt.active)
                handleFinalChoiceEvent(*this, *event);
        }

        // sf::Sprite returnSprite(returnSymbol);

        updateTeleport();
        updateEndSequence();
        updateSoundFades();
        updateQuizIntro(*this);
        updateLayout();

        window.clear(ColorHelper::Palette::BlueNearBlack);
        renderGame(*this);
        window.display();
    }
}

void Game::startTypingSound() {
    if (textBlipSound->getStatus() != sf::Sound::Status::Playing) {
        // std::cout << "start on " << this << " / sound " << &textBlipSound << "\n";
        textBlipSound->play();
    }
}

void Game::stopTypingSound() {
    if (textBlipSound->getStatus() == sf::Sound::Status::Playing) {
        // std::cout << "end on " << this << " / sound " << &textBlipSound << "\n";
        textBlipSound->stop();
    }
}
