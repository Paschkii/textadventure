#pragma once
// === C++ Libraries ===
#include <array>     // Needed for the fixed-size option and hitbox collections.
#include <cstdint>
#include <cstddef>   // Provides std::size_t for the many index and size fields.
#include <limits>    // Supplies std::numeric_limits used when tracking dialogue indexes.
#include <optional>  // Stores optional state like hovered locations and cached sounds.
#include <string>    // Holds player names, dialogue text, and UI labels.
#include <utility>   // Enables std::move in constructors such as DragonPortrait.
#include <vector>    // Manages dynamic lists of dialogue lines, locations, and UI elements.
#include <chrono>    // Tracks when the current session started for the ranking system.
#include <random>    // Prepares the RNG used inside quiz data.
// === SFML Libraries ===
#include <SFML/Audio.hpp>           // Declares sf::Sound and other audio assets stored in the class.
#include <SFML/Graphics.hpp>        // Defines sprites, shapes, the window, and color helpers used throughout.
#include <SFML/System/Clock.hpp>    // Provides sf::Clock for all the timing members.
// === Header Files ===
#include "audio/audioManager.hpp"               // Controls music/sfx players owned by the Game.
#include "core/endSequenceController.hpp"       // Handles the final-sequence controller member.
#include "core/itemController.hpp"              // Manages the in-game items referenced by Game.
#include "core/state.hpp"                       // Defines GameState values used throughout.
#include "core/teleportController.hpp"          // Provides the teleport controller member and friend hook.
#include "resources/resources.hpp"              // Supplies the Resources member with textures and sounds.
#include "rendering/locations.hpp"              // Provides Location and LocationId for map logic.
#include "story/storyIntro.hpp"                 // Supplies the intro dialogue referenced by default state.
#include "story/textStyles.hpp"                 // Defines TextStyles::SpeakerId and style helpers.
#include "ui/confirmationUI.hpp"                // Brings the ConfirmationPrompt used in the game state.
#include "ui/nineSliceBox.hpp"                  // Declares the NineSliceBox frame used for the UI.
#include "ui/quizGenerator.hpp"                 // Defines quiz::Question for the quiz data structure.
#include "core/ranking.hpp"                     // Tracks leaderboard entries persisted on disk.
#include "ui/rankingUI.hpp"                     // Renders the ranking overlay once the ending completes.

struct Game {
    friend void core::handleTravel(Game& game, LocationId id);

    enum class DragonbornGender {
        Female,
        Male
    };

    // Represents one selectable weapon entry in the selection panel.
    struct WeaponOption {
        sf::Texture texture;
        std::optional<sf::Sprite> sprite;
        std::string displayName;
        int hotkeyNumber = 0;
        sf::FloatRect bounds;
        sf::Vector2f labelPosition;

        WeaponOption() = default;
        WeaponOption(WeaponOption&&) noexcept = default;
        WeaponOption& operator=(WeaponOption&&) noexcept = default;
        WeaponOption(const WeaponOption&) = delete;
        WeaponOption& operator=(const WeaponOption&) = delete;
    };

    // Wraps the data needed to show a dragon illustration on the UI.
    struct DragonPortrait {
        DragonPortrait(const sf::Texture& texture, std::string name)
        : sprite(texture), displayName(std::move(name)) {}

        DragonPortrait(DragonPortrait&&) noexcept = default;
        DragonPortrait& operator=(DragonPortrait&&) noexcept = default;
        DragonPortrait(const DragonPortrait&) = delete;
        DragonPortrait& operator=(const DragonPortrait&) = delete;
        
        sf::Sprite sprite;
        std::string displayName;
        sf::FloatRect bounds;
        sf::Vector2f labelPosition;
        sf::Vector2f centerPosition;
        float baseScale = 1.f;
    };

    // Tracks which dragon portrait is currently highlighted and the phase of its fade animation.
    struct DragonShowcaseState {
        enum class Phase {
            Hidden,
            FadingIn,
            Visible,
            FadingOut
        };

        Phase phase = Phase::Hidden;
        std::optional<std::size_t> activeDragon;
        std::optional<std::size_t> pendingDragon;
        std::size_t lastProcessedDialogueIndex = std::numeric_limits<std::size_t>::max();
        sf::Clock fadeClock;
    };

    // Stores the state and progression for the quiz minigame.
    struct QuizData {
        bool active = false;
        LocationId targetLocation = LocationId::Gonad;
        std::string dragonName;
        std::vector<quiz::Question> questions;
        std::size_t currentQuestion = 0;
        std::array<sf::FloatRect, 4> optionBounds;
        int hoveredIndex = -1;
        std::size_t questionIndex = 0;
        const std::vector<DialogueLine>* quizDialogue = nullptr;
        bool pendingSuccess = false;
        std::vector<DialogueLine> feedbackDialogue;
        bool feedbackActive = false;
        bool pendingQuestionAdvance = false;
        bool pendingRetry = false;
        bool pendingFinish = false;
        enum class SelectionPhase {
            Idle,
            Logging,
            Blinking
        };
        // Captures the speaker/text that should be shown during pending feedback screens.
        struct PendingFeedback {
            TextStyles::SpeakerId speaker = TextStyles::SpeakerId::NoNameNPC;
            std::string text;
            std::string followup;
            bool advance = false;
            bool retry = false;
            bool finish = false;
        };
        PendingFeedback pendingFeedback;
        bool pendingFeedbackActive = false;
        SelectionPhase selectionPhase = SelectionPhase::Idle;
        int selectionIndex = -1;
        bool selectionCorrect = false;
        bool pendingQuestionStartAnnouncement = false;
        sf::Clock selectionClock;
        sf::Clock blinkClock;
        // Holds timing/dialogue state for the quiz intro announcement sequence.
        struct IntroState {
            bool active = false;
            sf::Clock clock;
            std::size_t questionIndex = 0;
            LocationId targetLocation = LocationId::Gonad;
            const std::vector<DialogueLine>* dialogue = nullptr;
        } intro;
        bool quizAutoStarted = false;
        float locationMusicVolumeBeforeQuiz = 100.f;
        bool locationMusicMuted = false;
        bool questionStartSuppressed = false;
        bool suppressNextQuestionStartRestart = false;
        enum class QuestionAudioPhase {
            Idle,
            QuestionStart,
            Thinking
        };
        QuestionAudioPhase questionAudioPhase = QuestionAudioPhase::Idle;
        sf::Clock questionStartClock;
        bool finalCheerActive = false;
        sf::Clock finalCheerClock;
        bool finalCheerTriggered = false;
        std::optional<quiz::Question> pendingSillyReplacement;
        std::mt19937 rng;
    };

    // Holds the three final-choice buttons shown in the climactic scene.
    struct FinalChoiceData {
        bool active = false;
        std::array<std::string, 3> options;
        std::array<sf::FloatRect, 3> optionBounds;
        int hoveredIndex = -1;
    };

    // Sets up all shared resources and UI state for the game.
    Game();
    // Drives the main event/render loop while the window is open.
    void run();
    // Recalculates UI layout whenever the window size changes.
    void updateLayout();
    // Begins the session timer used to measure the player's completion time.
    void beginSessionTimer();
    // Records the ranking entry for the current session and returns the finished rank.
    int recordSessionRanking();

    // Begins the typing sound effect tied to dialogue output.
    void startTypingSound();
    // Stops the typing sound so it does not continue indefinitely.
    void stopTypingSound();
    // Sets the active location and starts its music.
    void setCurrentLocation(const Location* location);
    // Plays the title screen music.
    void startTitleScreenMusic();
    // Fades out the title screen music over the given time.
    void fadeOutTitleScreenMusic(float duration);

        // === Public game data ===
        sf::RenderWindow window;                            // Main SFML window for rendering.
        Resources resources;                                // Central texture/audio assets store.
        AudioManager audioManager;                          // Music and sound effect manager.
        core::TeleportController teleportController;        // Handles teleport animations.
        core::EndSequenceController endSequenceController;  // Final-overlay sequence control.
        GameState state = GameState::IntroScreen;           // Current UI/game mode.

        // === Dialogues ===
        size_t dialogueIndex = 0;                                   // Current line index inside the active dialogue.
        const std::vector<DialogueLine>* currentDialogue = &intro;  // Active dialogue pool.
        std::optional<TextStyles::SpeakerId> lastSpeaker;           // Last speaker shown.

        sf::RectangleShape nameBox;                      // Draws the speaker name frame.
        sf::RectangleShape playerStatusBox;               // Shows HP/XP bars above the name box.
        sf::RectangleShape optionsBox;                    // Highlights hovered intro options.
        sf::RectangleShape introOptionBackdrop;           // Backdrop behind intro options.
        float playerHp = 5.f;                            // Player HP value for the status bar.
        float playerHpMax = 500.f;                       // Maximum HP used for the ratio display.
        float playerXp = 0.f;                            // Player XP value for the status bar.
        float playerXpMax = 100.f;                       // XP required for the next level.
        sf::RectangleShape textBox;                      // Outline around dialogue text.
        sf::RectangleShape locationBox;                  // Box showing the current location.
        sf::RectangleShape itemBox;                      // Outline for the item list.
        sf::RectangleShape weaponPanel;                  // Weapon selector background.
        sf::RectangleShape menuButton;                   // Button that opens the in-game menu.
        sf::RectangleShape menuPanel;                    // Panel shown when the menu is active.
        std::array<sf::FloatRect, 6> menuTabBounds{};    // Click/touch areas for menu tabs.
        NineSliceBox uiFrame{12};                        // Decorative frame around UI elements.
        bool menuActive = false;                         // Determines whether the menu overlay is visible.
        bool menuButtonHovered = false;                   // Tracks hover state for the menu button.
        int menuHoveredTab = -1;                          // Hovered tab index.
        int menuActiveTab = 0;                            // Active tab index when the menu is open.

        std::optional<sf::Sprite> background;             // Background art for the current scene.
        std::optional<sf::Sprite> returnSprite;           // Icon drawn when returning to map.

        std::string visibleText;                       // Currently rendered portion of the active line.
        std::size_t charIndex = 0;                     // Visible character count.
        sf::Clock typewriterClock;                     // Drives the typewriter animation speed.
        sf::Clock uiGlowClock;                         // Timer for glowing UI effects.

        std::optional<sf::Sound> enterSound;                // SFX when pressing Enter to advance.
        std::optional<sf::Sound> confirmSound;              // Confirmation prompt acceptance.
        std::optional<sf::Sound> rejectSound;               // Negative response effect.
        std::optional<sf::Sound> startGameSound;            // Plays when the adventure begins.
        std::optional<sf::Sound> quizLoggingSound;          // Played during quiz logging steps.
        std::optional<sf::Sound> quizCorrectSound;          // Positive quiz feedback.
        std::optional<sf::Sound> quizIncorrectSound;        // Negative quiz feedback.
        std::optional<sf::Sound> quizStartSound;            // Triggered when quiz starts.
        std::optional<sf::Sound> quizQuestionStartSound;    // Played on question reveal.
        std::optional<sf::Sound> quizQuestionThinkingSound; // Sounds during thinking phase.
        std::optional<sf::Sound> quizEndSound;              // Reward/jingle once quiz concludes.
        std::optional<sf::Sound> buttonHoverSound;          // Plays when hovering interactive buttons.
        std::optional<sf::Sound> introTitleHoverSound;      // Plays when hovering intro title options.

        ConfirmationPrompt confirmationPrompt;            // Wrapped modal yes/no dialog.

        std::string playerName;                        // Actual player name chosen from input.
        std::string nameInput;                         // Ongoing characters while typing.
        bool askingName = false;                       // True while waiting for name input.

        sf::Clock cursorBlinkClock;                    // Cursor blink timer.
        bool cursorVisible = true;                     // Should the name cursor be drawn?
        float cursorBlinkInterval = 0.5f;              // Blink speed in seconds.

        sf::Clock returnBlinkClock;                    // Blinking timer for the return icon.
        bool returnVisible = true;                     // Return prompt currently shown?
        float returnBlinkInterval = 0.4f;              // Return icon blink interval.

        std::string currentProcessedLine;               // Full string after layout formatting.

        sf::Clock introClock;                           // Drives intro fade timers.
        float introFadeDuration = 1.0f;                 // Duration for the intro fade.
        bool introFadeOutActive = false;                // Intro fade currently running.
        float introFadeOutDuration = 3.0f;              // Duration for the intro exit fade.

        bool introTitleFadeOutActive = false;           // Title drop animation active.
        float introTitleFadeOutDuration = 1.0f;         // Length of the title drop.
        bool introTitleHidden = false;                  // Hides the title while animating.
        sf::Clock introTitleFadeClock;                  // Title drop timer.

        bool introTitleRevealStarted = false;
        bool introTitleRevealComplete = false;
        float introTitleRevealDuration = 1.4f;
        sf::Clock introTitleRevealClock;
        bool introTitleOptionsFadeTriggered = false;
        bool introTitleOptionsFadeActive = false;
        float introTitleOptionsFadeDuration = 0.9f;
        float introTitleOptionsFadeProgress = 0.f;
        sf::Clock introTitleOptionsFadeClock;
        std::array<sf::FloatRect, 5> introTitleOptionBounds{};
        int introTitleHoveredOption = -1;

        bool genderSelectionActive = false;
        int genderSelectionHovered = -1;
        std::array<sf::FloatRect, 2> genderSelectionBounds{};
        DragonbornGender playerGender = DragonbornGender::Male;
        struct GenderSelectionAnimation {
            enum class Phase {
                Idle,
                Approaching,
                Reverting,
                FadingOut
            };

            Phase phase = Phase::Idle;
            int selection = -1;
            sf::Clock clock;
            float approachDuration = 0.8f;
            float fadeDuration = 0.5f;
            bool labelsHidden = false;
        };
        GenderSelectionAnimation genderAnimation;

        bool introDialogueFinished = false;              // Intro dialogue completed flag.
        bool uiFadeOutActive = false;                    // UI fade-out currently running.
        float uiFadeOutDuration = 1.0f;                  // Duration of the UI fade-out.
        sf::Clock uiFadeClock;                           // Controls UI fade timing.

        bool uiFadeInQueued = false;                     // Next fade-in was requested.
        bool uiFadeInActive = false;                     // UI is currently fading in.
        bool pendingIntroDialogue = false;               // Queue the intro dialogue after the title.
        bool pendingGonadDialogue = false;                // Queue Gonad dialogue after the intro.
        float uiFadeInDuration = 1.0f;                   // Fade-in duration.

        bool backgroundFadeInActive = false;              // Background fade animation running.
        bool backgroundVisible = false;                   // Should the background be drawn?
        sf::Clock backgroundFadeClock;                    // Timer for background fading.

        std::vector<Location> locations;                    // All locations available for travel.
        const Location* currentLocation = nullptr;          // Currently active location pointer.
        std::optional<LocationId> keyboardMapHover;         // Location hovered via keyboard navigation.
        std::optional<LocationId> mouseMapHover;            // Location under the mouse cursor.
        std::array<sf::FloatRect, 5> mapLocationHitboxes{}; // Hitboxes for clickable locations.
        std::array<bool, 5> locationCompleted{};            // Flags per location completion.
        std::optional<LocationId> lastCompletedLocation;    // Last location that was cleared.
        std::string lastDragonName;                         // Tracks the last seen dragon name.
        int dragonStoneCount = 0;                           // How many dragon stones the player has.

        std::vector<WeaponOption> weaponOptions;        // Weapon widgets for the selection screen.
        int hoveredWeaponIndex = -1;                    // Hovered weapon index.
        int selectedWeaponIndex = -1;                   // Currently selected weapon slot.
        bool weaponItemAdded = false;                   // Did we already add the weapon item?

        std::vector<DragonPortrait> dragonPortraits;    // Portraits used in the dragon showcase UI.
        DragonShowcaseState dragonShowcase;             // State machine for the showcase animation.
        core::ItemController itemController;            // Controls collected items.
        QuizData quiz;                                  // Manages quiz mode state and lines.
        FinalChoiceData finalChoice;                    // Final choice UI state.
        std::vector<DialogueLine> transientDialogue;    // Temporary dialogue content.
        bool transientReturnToMap = false;              // Return map triggered when transient dialogue ends.
        bool holdMapDialogue = false;                   // Prevents map dialogue updates.
        bool pendingTeleportToGonad = false;            // Teleport to Gonad next frame.
        bool finalEncounterPending = false;             // Indicates final encounter is queued.
        bool finalEncounterActive = false;              // Final encounter currently running.
        bool finalEndingPending = false;                // Ending sequence is next.
        bool startGameSoundPlayed = false;              // Ensures start sound plays once.
        struct RankingSession {
            bool started = false;
            sf::Clock timer;
            std::chrono::system_clock::time_point startTime;
        };
        RankingSession rankingSession;
        int totalRiddleFaults = 0;                      // Tracks how many incorrect quiz answers occurred.
        core::RankingManager rankingManager;             // Persists ranking entries to disk.
        std::uint64_t lastRecordedEntryId = 0;           // Identifies the latest saved session.
        int lastRecordedRank = -1;                       // 1-based rank of the latest run.
        ui::ranking::OverlayState rankingOverlay;        // State for drawing the leaderboard.
    // Hand off location travel to the dedicated handler.
    void startTravel(LocationId id);
    // Starts the teleport animation and audio transition.
    void beginTeleport(LocationId id);
    // Steps the teleport phase machine forward each frame.
    void updateTeleport();
    // Chooses the UI frame base color for the current location.
    sf::Color frameBaseColor() const;
    // Applies the requested alpha factor to the frame color.
    sf::Color frameColor(float uiAlphaFactor) const;

    private:
        // Fills the list of selectable weapon options.
        void loadWeaponOptions();
        // Prepares the dragon portraits displayed in the showcase.
        void loadDragonPortraits();
};
