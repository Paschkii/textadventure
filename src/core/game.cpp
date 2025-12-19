// === C++ Libraries ===
#include <cstddef>    // Needed for std::size_t definitions like playerNameMaxLength.
#include <cstdlib>    // Provides std::exit for the fatal resource-loading failure path.
#include <ctime>      // Formats the start timestamp stored for each run.
#include <iostream>   // Used for logging resource errors via std::cout.
#include <chrono>     // Tracks session start/finish times for the leaderboard.
// === SFML Libraries ===
#include <SFML/Graphics.hpp>  // Provides sf::Color for palette logic applied to UI boxes.
#include <SFML/Window.hpp>    // Supplies sf::Event, sf::Keyboard, and sf::Style used in the event loop.
// === Header Files ===
#include "game.hpp"                   // Declares the Game class whose body is defined here.
#include "teleportController.hpp"     // Needed for the teleport controller member and travel helpers.
#include "helper/colorHelper.hpp"     // Supplies ColorHelper::Palette/alpha helpers used when drawing UI boxes.
#include "helper/dragonHelpers.hpp"   // Provides ui::dragons::loadDragonPortraits called during initialization.
#include "helper/layoutHelpers.hpp"   // Defines ui::layout::updateLayout used by Game::updateLayout.
#include "helper/weaponHelpers.hpp"   // Offers ui::weapons::loadWeaponOptions for weapon setup.
#include "helper/healingPotion.hpp"   // Manages the healing potion timer granted by Wanda Rinn.
#include "rendering/dialogRender.hpp" // Renders the dialog UI via renderGame in the game loop.
#include "story/dialogInput.hpp"      // Contains waitForEnter and dialog flow helpers used in the loop.
#include "story/dialogueLine.hpp"     // Supplies the DialogueLine type processed while waiting for Enter.
#include "story/storyIntro.hpp"       // Provides the intro dialogue vector that Game starts with.
#include "story/textStyles.hpp"       // Gives TextStyles used for speaker colors and UI states.
#include "ui/confirmationUI.hpp"      // Declares confirmationPrompt handling invoked while running the loop.
#include "ui/introTitle.hpp"          // Declares intro title helpers used during the intro screen.
#include "ui/genderSelectionUI.hpp"   // Handles the dragonborn selection overlay.
#include "ui/mapSelectionUI.hpp"      // Declares handleMapSelectionEvent invoked for map choices.
#include "ui/quizUI.hpp"              // Declares handleQuizEvent triggered while in quiz mode.
#include "ui/weaponSelectionUI.hpp"   // Declares handleWeaponSelectionEvent and related callbacks.
#include "ui/menuUI.hpp"              // Handles the in-game menu button + overlay.
#include "ui/bookshelfUI.hpp"         // Draws the optional bookshelf puzzle.

constexpr unsigned int windowWidth = 1280;
constexpr unsigned int windowHeight = 720;
constexpr unsigned int fpsLimit = 60;
constexpr std::size_t playerNameMaxLength = 18;

namespace {
    const sf::Texture* backgroundForLocation(const Game& game, LocationId id) {
        switch (id) {
            case LocationId::Perigonal: return &game.resources.backgroundPetrigonal;
            case LocationId::Gonad: return &game.resources.backgroundGonad;
            case LocationId::Blyathyroid: return &game.resources.backgroundBlyathyroid;
            case LocationId::Lacrimere: return &game.resources.backgroundLacrimere;
            case LocationId::Cladrenal: return &game.resources.backgroundCladrenal;
            case LocationId::Aerobronchi: return &game.resources.backgroundAerobronchi;
            case LocationId::Seminiferous: return &game.resources.backgroundSeminiferous;
            default: return nullptr;
        }
    }
}

// Sets up resources, audio, and UI state for a new Game instance.
Game::Game()
: window(sf::VideoMode({windowWidth, windowHeight}), "Glandular", sf::Style::Titlebar | sf::Style::Close)
{
    if (!resources.loadAll()) {
        std::cout << "Fatal: konnte Ressourcen nicht laden.\n";
        std::exit(1);
    }
    audioManager.init(resources);
    itemController.init(resources);
    teleportController.loadResources(resources);
    setBackgroundTexture(resources.introBackground);
    backgroundFadeInActive = false;
    backgroundVisible = true;
    returnSprite.emplace(resources.returnSymbol);
    returnSprite->setColor(ColorHelper::Palette::IconGray);

    locations = Locations::buildLocations(resources);

    enterSound.emplace(resources.enterKey);
    confirmSound.emplace(resources.confirm);
    rejectSound.emplace(resources.reject);
    startGameSound.emplace(resources.startGame);
    buttonHoverSound.emplace(resources.buttonHovered);
    buttonHoverSound->setVolume(120.f);
    introTitleHoverSound.emplace(resources.titleButtons);
    introTitleHoverSound->setVolume(120.f);
    quizLoggingSound.emplace(resources.quizLoggingAnswer);
    quizCorrectSound.emplace(resources.quizAnswerCorrect);
    quizIncorrectSound.emplace(resources.quizAnswerIncorrect);
    quizStartSound.emplace(resources.quizStart);
    quizQuestionStartSound.emplace(resources.quizQuestionStart);
    quizQuestionThinkingSound.emplace(resources.quizQuestionThinking);
    quizEndSound.emplace(resources.quizEnd);
    // === Framerate limitieren ===
    window.setFramerateLimit(fpsLimit);
    // === NameBox Style setzen ===
    nameBox.setFillColor(ColorHelper::Palette::DialogBackdrop);
    nameBox.setOutlineColor(ColorHelper::Palette::Normal);
    nameBox.setOutlineThickness(2.f);
    playerStatusBox.setFillColor(TextStyles::UI::PanelDark);
    playerStatusBox.setOutlineThickness(2.f);
    playerStatusBox.setOutlineColor(ColorHelper::Palette::FrameGoldLight);
    optionsBox.setFillColor(sf::Color::Transparent);
    optionsBox.setOutlineThickness(0.f);
    introOptionBackdrop.setFillColor(sf::Color(12, 12, 18, 210));
    introOptionBackdrop.setOutlineThickness(0.f);
    // === TextBox Style setzen ===
    textBox.setFillColor(ColorHelper::Palette::DialogBackdrop);
    textBox.setOutlineColor(ColorHelper::Palette::Normal);
    textBox.setOutlineThickness(2.f);
    // === LocationBox Style setzen ===
    locationBox.setFillColor(sf::Color::Transparent);
    locationBox.setOutlineThickness(0.f);
    // === ItemBox Style setzen ===
    itemBox.setFillColor(sf::Color::Transparent);
    itemBox.setOutlineColor(ColorHelper::Palette::Normal);
    itemBox.setOutlineThickness(2.f);

    weaponPanel.setFillColor(sf::Color::Transparent);
    weaponPanel.setOutlineColor(ColorHelper::Palette::Normal);
    weaponPanel.setOutlineThickness(2.f);
    menuButton.setTexture(&resources.menuButton, true);
    menuButton.setFillColor(sf::Color::White);
    menuButton.setOutlineThickness(0.f);

    menuPanel.setFillColor(TextStyles::UI::PanelDark);
    menuPanel.setOutlineColor(TextStyles::UI::Border);
    menuPanel.setOutlineThickness(2.f);

    currentDialogue = &intro;

    ui::weapons::loadWeaponOptions(*this);
    ui::dragons::loadDragonPortraits(*this);
    rankingManager.load("assets/data/rankings.json");
}

// Tracks the active location and starts its music.
void Game::setCurrentLocation(const Location* location, bool updateBackground) {
    currentLocation = location;
    if (location)
        audioManager.startLocationMusic(location->id);
    if (location && updateBackground) {
        if (const sf::Texture* backgroundTexture = backgroundForLocation(*this, location->id))
            setBackgroundTexture(*backgroundTexture);
    }
}

void Game::setBackgroundTexture(const sf::Texture& texture) {
    queuedBackgroundTexture = nullptr;
    if (background && &background->getTexture() == &texture)
        return;
    background.emplace(texture);
    backgroundFadeInActive = true;
    backgroundFadeClock.restart();
    backgroundVisible = false;
}

// Begins the looping title-screen music.
void Game::startTitleScreenMusic() {
    audioManager.startTitleScreenMusic();
}

// Fades the title music smoothly over the provided duration.
void Game::fadeOutTitleScreenMusic(float duration) {
    audioManager.fadeOutTitleScreenMusic(duration);
}

// Triggers travel handling once a destination is selected.
void Game::startTravel(LocationId id) {
    core::handleTravel(*this, id);
}

// Launches the teleport overlay, stopping typing audio first.
void Game::beginTeleport(LocationId id) {
    if (teleportController.active())
        return;

    stopTypingSound();
    transientReturnToMap = false;
    teleportController.begin(id, audioManager);
}

// Advances the teleport sequence and invokes callbacks when ready.
void Game::updateTeleport() {
    teleportController.update([this](LocationId id) {
        startTravel(id);
    });
}

// Picks the base UI frame color based on the current location.
sf::Color Game::frameBaseColor() const {
    if (currentLocation && currentLocation->id == LocationId::Seminiferous)
        return ColorHelper::Palette::DarkPurple;

    return TextStyles::UI::PanelDark;
}

// Applies the given alpha factor to the base frame color.
sf::Color Game::frameColor(float uiAlphaFactor) const {
    return ColorHelper::applyAlphaFactor(frameBaseColor(), uiAlphaFactor);
}

// Recomputes the layout whenever the window geometry changes (Not happening right now, window is limited to fixed size).
void Game::updateLayout() {
    ui::layout::updateLayout(*this);
}

void Game::beginSessionTimer() {
    rankingSession.started = true;
    rankingSession.timer.restart();
    rankingSession.startTime = std::chrono::system_clock::now();
    totalRiddleFaults = 0;
}

int Game::recordSessionRanking() {
    if (!rankingSession.started)
        return -1;

    rankingSession.started = false;
    core::RankingEntry entry;
    auto now = std::chrono::system_clock::now();
    entry.id = static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
    entry.playerName = playerName.empty() ? "Player" : playerName;
    entry.totalSeconds = rankingSession.timer.getElapsedTime().asSeconds();
    entry.faults = totalRiddleFaults;

    std::time_t startTime = std::chrono::system_clock::to_time_t(rankingSession.startTime);
    std::tm timeBuffer{};
#if defined(_MSC_VER)
    localtime_s(&timeBuffer, &startTime);
#else
    localtime_r(&startTime, &timeBuffer);
#endif

    char formatted[64];
    std::strftime(formatted, sizeof(formatted), "%Y-%m-%d %H:%M:%S", &timeBuffer);
    entry.started = formatted;

    lastRecordedEntryId = entry.id;
    lastRecordedRank = rankingManager.recordEntry(entry);
    ui::ranking::triggerOverlay(rankingOverlay, lastRecordedRank);
    return lastRecordedRank;
}


// Drives the main event/render loop until the window closes.
void Game::run() {
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            bool eventConsumed = false;

            if (event->is<sf::Event::Closed>()) {
                window.close();
                continue;
            }

            if (ui::ranking::isOverlayActive(rankingOverlay)) {
                ui::ranking::handleOverlayEvent(rankingOverlay, *event);
                continue;
            }

            if (teleportController.active()
                || endSequenceController.isActive()
                || endSequenceController.isScreenVisible())
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
            if (!confirmationPrompt.active && ui::menu::handleEvent(*this, *event))
                continue;
            if (genderSelectionActive) {
                if (ui::genderSelection::handleEvent(*this, *event))
                    continue;
            }
            if (state == GameState::IntroTitle && !confirmationPrompt.active) {
                if (handleIntroTitleEvent(*this, *event))
                    continue;
            }

            if (auto key = event->getIf<sf::Event::KeyReleased>()) {
                if (key->scancode == sf::Keyboard::Scan::Enter) {
                    // Block Enter key while a confirmation prompt is active to avoid
                    // accidental confirmation via keyboard (prevent misclicks).
                    if (confirmationPrompt.active)
                        continue;

                    // Enter sound is played only when a new dialogue line actually starts.
                    // The logic for playing the sound is handled inside `waitForEnter`.

                    if (state == GameState::IntroScreen) {
                        if (!introFadeOutActive) {
                            introFadeOutActive = true;
                            introClock.restart();
                        }
                        eventConsumed = true;
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
            else if (state == GameState::Bookshelf && !confirmationPrompt.active) {
                if (ui::bookshelf::handleEvent(*this, *event))
                    continue;
            }
            else if (state == GameState::FinalChoice && !confirmationPrompt.active)
                handleFinalChoiceEvent(*this, *event);
        }

        // sf::Sprite returnSprite(returnSymbol);

        updateTeleport();
        endSequenceController.update();
        if (endSequenceController.isScreenVisible() && rankingOverlay.pending)
            ui::ranking::activateOverlay(rankingOverlay);
        ui::ranking::updateOverlay(rankingOverlay);
        audioManager.update();
        updateQuizIntro(*this);
        helper::healingPotion::update(*this);
        updateLayout();

        window.clear(ColorHelper::Palette::BlueNearBlack);
        renderGame(*this);
        window.display();
    }
}

// Plays the typing effect sound to match dialogue text.
void Game::startTypingSound() {
    audioManager.startTypingSound();
}

// Stops the typing effect so it does not linger.
void Game::stopTypingSound() {
    audioManager.stopTypingSound();
}

void Game::startBookshelfQuest() {
    if (state == GameState::Bookshelf)
        return;
    bookshelf.previousState = state;
    state = GameState::Bookshelf;
    ui::bookshelf::enter(*this);
}

void Game::exitBookshelfQuest() {
    if (state != GameState::Bookshelf)
        return;
    state = bookshelf.previousState;
}
