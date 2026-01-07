// === C++ Libraries ===
#include <algorithm>  // Searches quest log entries and clamps XP.
#include <cstddef>    // Needed for std::size_t definitions like playerNameMaxLength.
#include <cstdlib>    // Provides std::exit for the fatal resource-loading failure path.
#include <ctime>      // Formats the start timestamp stored for each run.
#include <cmath>      // Provides pow/round used for XP curve.
#include <iostream>   // Used for logging resource errors via std::cout.
#include <chrono>     // Tracks session start/finish times for the leaderboard.
// === SFML Libraries ===
#include <SFML/Graphics.hpp>  // Provides sf::Color for palette logic applied to UI boxes.
#include <SFML/System/Clock.hpp>
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
#include "ui/battleUI.hpp"            // Draws the temporary Pokemon battle demo overlay.
#include "ui/genderSelectionUI.hpp"   // Handles the dragonborn selection overlay.
#include "ui/mapSelectionUI.hpp"      // Declares handleMapSelectionEvent invoked for map choices.
#include "ui/quizUI.hpp"              // Declares handleQuizEvent triggered while in quiz mode.
#include "ui/weaponSelectionUI.hpp"   // Declares handleWeaponSelectionEvent and related callbacks.
#include "ui/menuUI.hpp"              // Handles the in-game menu button + overlay.
#include "ui/treasureChestUI.hpp"     // Draws the treasure chest reward overlay.
#include "story/quests.hpp"           // Provides shared quest definitions and lookup helpers.

constexpr unsigned int windowWidth = 1280;
constexpr unsigned int windowHeight = 720;
constexpr unsigned int fpsLimit = 60;
constexpr std::size_t playerNameMaxLength = 18;

constexpr float kXpCurveExponent = 1.2f;
constexpr float kXpCurveScale = 0.25f;
constexpr int kBaseXpRequirement = 100;
constexpr float kHpGainPerLevel = 5.f;
constexpr int kMaxPlayerLevel = 100;

inline int xpForLevel(int level) {
    int clampedLevel = std::max(1, level);
    float xp = kBaseXpRequirement * std::pow(static_cast<float>(clampedLevel), kXpCurveExponent) * kXpCurveScale;
    int rounded = static_cast<int>(std::round(xp / 5.f) * 5.f);
    return std::max(5, rounded);
}

namespace {
    const sf::Texture* backgroundForLocation(const Game& game, LocationId id) {
        switch (id) {
            case LocationId::Perigonal: return &game.resources.backgroundPetrigonal;
            case LocationId::Gonad: return &game.resources.backgroundGonad;
            case LocationId::FigsidsForge: return &game.resources.backgroundBlacksmith;
            case LocationId::Blyathyroid: return &game.resources.backgroundBlyathyroid;
            case LocationId::Lacrimere: return &game.resources.backgroundLacrimere;
            case LocationId::Cladrenal: return &game.resources.backgroundCladrenal;
            case LocationId::Aerobronchi: return &game.resources.backgroundAerobronchi;
            case LocationId::Seminiferous: return &game.resources.backgroundSeminiferous;
            case LocationId::UmbraOssea: return &game.resources.backgroundUmbraOssea;
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

    blacksmithLocation.id = LocationId::FigsidsForge;
    blacksmithLocation.name = "Figsid's Forge";
    blacksmithLocation.color = ColorHelper::Palette::SoftYellow;

    enterSound.emplace(resources.enterKey);
    confirmSound.emplace(resources.confirm);
    rejectSound.emplace(resources.reject);
    startGameSound.emplace(resources.startGame);
    buttonHoverSound.emplace(resources.buttonHovered);
    buttonHoverSound->setVolume(120.f);
    introTitleHoverSound.emplace(resources.titleButtons);
    introTitleHoverSound->setVolume(120.f);
    menuOpenSound.emplace(resources.openMenu);
    menuCloseSound.emplace(resources.closeMenu);
    quizLoggingSound.emplace(resources.quizLoggingAnswer);
    quizCorrectSound.emplace(resources.quizAnswerCorrect);
    quizIncorrectSound.emplace(resources.quizAnswerIncorrect);
    quizStartSound.emplace(resources.quizStart);
    quizQuestionStartSound.emplace(resources.quizQuestionStart);
    quizQuestionThinkingSound.emplace(resources.quizQuestionThinking);
    quizEndSound.emplace(resources.quizEnd);
    forgeSound.emplace(resources.forgeSound);
    levelUpSound.emplace(resources.levelUp);
    questStartSound.emplace(resources.questStart);
    questEndSound.emplace(resources.questEnd);
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
    playerXpMax = static_cast<float>(xpForLevel(playerLevel));

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
    setMenuActive(false);
    menuActiveTab = -1;
    menuHoveredTab = -1;
    menuMapPopup.reset();
    mouseMapHover.reset();
    keyboardMapHover.reset();
    mapInteractionUnlocked = false;
    transientReturnToMap = false;
    teleportController.begin(id, audioManager);
}

void Game::beginForcedDestinationSelection() {
    forcedDestinationSelection = true;
    setMenuActive(true);
    menuActiveTab = 1;
    menuHoveredTab = -1;
    menuButtonUnlocked = true;
    menuButtonFadeActive = false;
    menuButtonAlpha = 1.f;
    menuButtonHovered = false;
    mouseMapHover.reset();
    keyboardMapHover.reset();
    mapTutorialActive = false;
    mapTutorialAwaitingOk = false;
    mapTutorialHighlight.reset();
    mapTutorialPopupBounds = {};
    mapTutorialOkBounds = {};
    mapTutorialOkHovered = false;
    menuMapPopup.reset();
    mapInteractionUnlocked = true;
}

void Game::exitForcedDestinationSelection() {
    forcedDestinationSelection = false;
    setMenuActive(false);
    menuHoveredTab = -1;
    menuMapPopup.reset();
    mapInteractionUnlocked = false;
}

void Game::setMenuActive(bool active) {
    if (menuActive == active)
        return;
    menuActive = active;
    auto& sound = active ? menuOpenSound : menuCloseSound;
    if (sound) {
        sound->stop();
        sound->play();
    }
}

// Advances the teleport sequence and invokes callbacks when ready.
void Game::updateTeleport() {
    teleportController.update([this](LocationId id) {
        startTravel(id);
    });
}

// Picks the base UI frame color based on the current location.
sf::Color Game::frameBaseColor() const {
    if (currentLocation
        && (currentLocation->id == LocationId::Seminiferous
            || currentLocation->id == LocationId::UmbraOssea))
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
#if defined(_WIN32)
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
    sf::Clock frameClock;
    while (window.isOpen()) {
        sf::Time frameTime = frameClock.restart();
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

            if (state == GameState::BattleDemo) {
                ui::battle::handleEvent(*this, *event);
                continue;
            }

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
                if (key->scancode == sf::Keyboard::Scan::Tab) {
                    playerStatusFolded = !playerStatusFolded;
                    eventConsumed = true;
                    continue;
                }
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

            if (!eventConsumed) {
                if (auto button = event->getIf<sf::Event::MouseButtonReleased>()) {
                    if (button->button == sf::Mouse::Button::Left) {
                        auto mousePos = window.mapPixelToCoords(button->position);
                        if (playerStatusFoldBarBounds.contains(mousePos)) {
                            playerStatusFolded = !playerStatusFolded;
                            eventConsumed = true;
                            continue;
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
            else if (state == GameState::TreasureChest && !confirmationPrompt.active) {
                if (ui::treasureChest::handleEvent(*this, *event))
                    continue;
            }
            else if (state == GameState::FinalChoice && !confirmationPrompt.active)
                handleFinalChoiceEvent(*this, *event);
            else if (state == GameState::Credits)
                ui::credits::handleEvent(*this, *event);
        }

        // sf::Sprite returnSprite(returnSymbol);

        updateTeleport();
        endSequenceController.update();
        if (creditsAfterEndPending && endSequenceController.isScreenVisible()) {
            constexpr float kCreditsStartDelay = 2.0f;
            if (!creditsAfterEndTimerActive) {
                creditsAfterEndTimerActive = true;
                creditsAfterEndClock.restart();
            }
            if (creditsAfterEndClock.getElapsedTime().asSeconds() >= kCreditsStartDelay) {
                creditsAfterEndPending = false;
                creditsAfterEndTimerActive = false;
                endSequenceController.reset();
                state = GameState::Credits;
                creditsState.initialized = false;
                creditsState.active = true;
            }
        }
        if (endSequenceController.isScreenVisible() && rankingOverlay.pending)
            ui::ranking::activateOverlay(rankingOverlay);
        ui::ranking::updateOverlay(rankingOverlay);
        audioManager.update();
        updateQuizIntro(*this);
        updateWeaponForging(*this);
        helper::healingPotion::update(*this);
        ui::treasureChest::update(*this, frameTime.asSeconds());
        ui::battle::update(*this, frameTime);
        if (state == GameState::Credits) {
            ui::credits::update(*this);
        }
        if (menuMapUmbraOverlayFadeInActive) {
            float elapsed = menuMapUmbraOverlayClock.getElapsedTime().asSeconds();
            if (elapsed >= 2.f) {
                menuMapUmbraOverlayFadeInActive = false;
                if (menuMapUmbraOverlayHold) {
                    menuMapUmbraOverlayHold = false;
                    holdMapDialogue = false;
                }
            }
        }
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

void Game::grantXp(int amount) {
    if (amount <= 0 || playerLevel >= kMaxPlayerLevel)
        return;

    float tempXp = playerXp;
    float tempXpMax = playerXpMax;
    int tempLevel = playerLevel;
    float xpRemaining = static_cast<float>(amount);
    std::vector<XpGainSegment> segments;
    segments.reserve(4);
    int levelUpsAwarded = 0;
    while (xpRemaining > 0.f && tempLevel < kMaxPlayerLevel) {
        float xpNeeded = tempXpMax - tempXp;
        if (xpNeeded <= 0.f)
            xpNeeded = tempXpMax;
        float startRatio = (tempXpMax > 0.f) ? (tempXp / tempXpMax) : 0.f;

        if (xpRemaining >= xpNeeded) {
            segments.push_back({ startRatio, 1.f, true });
            xpRemaining -= xpNeeded;
            tempXp = 0.f;
            tempLevel++;
            tempXpMax = static_cast<float>(xpForLevel(tempLevel));
            levelUpsAwarded++;
        }
        else {
            float endRatio = (tempXpMax > 0.f) ? (tempXp + xpRemaining) / tempXpMax : 0.f;
            segments.push_back({ startRatio, endRatio, false });
            tempXp += xpRemaining;
            xpRemaining = 0.f;
        }
    }
    if (tempLevel >= kMaxPlayerLevel) {
        tempLevel = kMaxPlayerLevel;
        tempXp = 0.f;
        tempXpMax = static_cast<float>(xpForLevel(tempLevel));
    }

    if (levelUpsAwarded > 0)
        pendingLevelUps += levelUpsAwarded;
    auto& xpGain = xpGainDisplay;
    xpGain.amount = amount;
    xpGain.segments = std::move(segments);
    xpGain.currentSegment = 0;
    xpGain.active = !xpGain.segments.empty();
    xpGain.waitingForLevelUp = false;
    xpGain.clock.restart();

    playerXp += static_cast<float>(amount);
    auto applyLevelBonus = [&]() {
        if (playerLevel >= kMaxPlayerLevel)
            return;
        playerLevel++;
        playerHpMax += kHpGainPerLevel;
        playerHp = playerHpMax;
        playerXpMax = static_cast<float>(xpForLevel(playerLevel));
    };

    while (playerXpMax > 0.f && playerXp >= playerXpMax && playerLevel < kMaxPlayerLevel) {
        playerXp -= playerXpMax;
        applyLevelBonus();
    }

    if (playerLevel >= kMaxPlayerLevel) {
        playerLevel = kMaxPlayerLevel;
        playerXp = 0.f;
        playerXpMax = static_cast<float>(xpForLevel(playerLevel));
    }

    if (playerXp < 0.f)
        playerXp = 0.f;
}

void Game::boostToLevel(int targetLevel) {
    int cappedLevel = std::min(targetLevel, kMaxPlayerLevel);
    if (cappedLevel <= playerLevel)
        return;
    int levelsToGain = cappedLevel - playerLevel;
    playerLevel = cappedLevel;
    playerHpMax += kHpGainPerLevel * static_cast<float>(levelsToGain);
    playerHp = playerHpMax;
    playerXp = 0.f;
    playerXpMax = static_cast<float>(xpForLevel(playerLevel));
    pendingLevelUps = 0;
    xpGainDisplay.active = false;
}

void Game::startQuest(const Story::QuestDefinition& quest) {
    auto existing = std::find_if(questLog.begin(), questLog.end(), [&](const QuestLogEntry& entry) {
        return entry.name == quest.name;
    });
    if (existing != questLog.end())
        return;

    QuestLogEntry entry{
        quest.name,
        quest.giver,
        quest.goal,
        quest.xpReward,
        quest.loot,
        false,
        false,
        false,
        1.f
    };
    questLog.push_back(entry);
    questFoldButtonBounds.resize(questLog.size());
    if (!questTutorialCompleted && !questTutorialPending)
        questTutorialPending = true;

    questPopup.entry = entry;
    questPopup.phase = QuestPopupState::Phase::Entering;
    questPopup.clock.restart();
    questPopup.message = "New Quest: " + entry.name;
    if (questStartSound) {
        questStartSound->stop();
        questStartSound->play();
    }
}

void Game::completeQuest(const Story::QuestDefinition& quest) {
    auto existing = std::find_if(questLog.begin(), questLog.end(), [&](const QuestLogEntry& entry) {
        return entry.name == quest.name;
    });
    if (existing == questLog.end() || existing->rewardGranted)
        return;

        existing->completed = true;
        existing->rewardGranted = true;
        existing->collapsed = true;
        existing->foldProgress = 0.f;
    grantXp(existing->xpReward);

    questPopup.entry = *existing;
    questPopup.phase = QuestPopupState::Phase::Entering;
    questPopup.clock.restart();
    questPopup.message = "Finished Quest: " + existing->name;
    if (questEndSound) {
        questEndSound->stop();
        questEndSound->play();
    }
}
