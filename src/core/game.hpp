#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window.hpp>
#include <optional>
#include <memory>
#include <cmath>
#include <vector>
#include <utility>
#include <string>
#include <array>
#include <limits>
#include "story/storyIntro.hpp"
#include "ui/nineSliceBox.hpp"
#include "resources/resources.hpp"
#include "core/state.hpp"
#include "story/textStyles.hpp"
#include "rendering/locations.hpp"
#include "ui/confirmationUI.hpp"
#include "ui/quizGenerator.hpp"

struct SoundFadeState {
    bool active = false;
    float duration = 1.f;
    float startVolume = 0.f;
    float targetVolume = 0.f;
    sf::Clock clock;
};

struct Game {
    enum class TeleportPhase {
        None,
        FadeOut,
        Cooldown,
        FadeIn
    };

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

    struct ItemIcon {
        explicit ItemIcon(const sf::Texture& tex) : sprite(tex) {}
        ItemIcon(ItemIcon&&) noexcept = default;
        ItemIcon& operator=(ItemIcon&&) noexcept = default;
        ItemIcon(const ItemIcon&) = delete;
        ItemIcon& operator=(const ItemIcon&) = delete;
        sf::Sprite sprite;
    };

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
    };

    struct FinalChoiceData {
        bool active = false;
        std::array<std::string, 3> options;
        std::array<sf::FloatRect, 3> optionBounds;
        int hoveredIndex = -1;
    };

    Game();
    void run();
    void updateLayout();

    void startTypingSound();
    void stopTypingSound();
    void setCurrentLocation(const Location* location);
    void startTitleScreenMusic();
    void fadeOutTitleScreenMusic(float duration);

        // === Public game data ===
        sf::RenderWindow window;
        Resources resources;
        GameState state = GameState::IntroScreen;

        // === Dialogues ===
        size_t dialogueIndex = 0;
        const std::vector<DialogueLine>* currentDialogue = &intro;
        std::optional<TextStyles::SpeakerId> lastSpeaker;

        sf::RectangleShape nameBox;
        sf::RectangleShape textBox;
        sf::RectangleShape locationBox;
        sf::RectangleShape itemBox;
        sf::RectangleShape weaponPanel;
        NineSliceBox uiFrame{12};

        std::optional<sf::Sprite> background;
        std::optional<sf::Sprite> returnSprite;

        std::string visibleText;
        std::size_t charIndex = 0;
        sf::Clock typewriterClock;
        sf::Clock uiGlowClock;

        std::optional<sf::Sound> textBlipSound;
        std::optional<sf::Sound> enterSound;
        std::optional<sf::Sound> acquireSound;
        std::optional<sf::Sound> confirmSound;
        std::optional<sf::Sound> rejectSound;
        std::optional<sf::Sound> startGameSound;
        std::optional<sf::Sound> teleportStartSound;
        std::optional<sf::Sound> teleportMiddleSound;
        std::optional<sf::Sound> teleportStopSound;
        std::optional<sf::Sound> quizLoggingSound;
        std::optional<sf::Sound> quizCorrectSound;
        std::optional<sf::Sound> quizIncorrectSound;
        std::optional<sf::Sound> quizStartSound;
        std::optional<sf::Sound> quizQuestionStartSound;
        std::optional<sf::Sound> quizQuestionThinkingSound;
        std::optional<sf::Sound> quizEndSound;
        std::optional<sf::Sound> locationMusic;
        std::optional<LocationId> locationMusicId;
        SoundFadeState locationMusicFade;
        std::optional<sf::Sound> titleScreenSound;
        SoundFadeState titleScreenFade;

        ConfirmationPrompt confirmationPrompt;

        std::string playerName;
        std::string nameInput;
        bool askingName = false;

        sf::Clock cursorBlinkClock;
        bool cursorVisible = true;
        float cursorBlinkInterval = 0.5f;

        sf::Clock returnBlinkClock;
        bool returnVisible = true;
        float returnBlinkInterval = 0.4f;

        sf::Clock introPromptBlinkClock;
        bool introPromptVisible = false;
        float introPromptBlinkInterval = 1.2f;
        float introPromptFade = 0.f;
        bool introPromptBlinkActive = false;
        bool introPromptInputEnabled = false;
        bool introPromptFadingIn = true;

        std::string currentProcessedLine;

        sf::Clock introClock;
        float introFadeDuration = 1.0f;
        bool introFadeOutActive = false;
        float introFadeOutDuration = 3.0f;

        bool introTitleFadeOutActive = false;
        float introTitleFadeOutDuration = 1.0f;
        bool introTitleHidden = false;
        sf::Clock introTitleFadeClock;

        bool introDialogueFinished = false;
        bool uiFadeOutActive = false;
        float uiFadeOutDuration = 1.0f;
        sf::Clock uiFadeClock;

        bool uiFadeInQueued = false;
        bool uiFadeInActive = false;
        bool startGonadDialoguePending = false;
        float uiFadeInDuration = 1.0f;

        bool backgroundFadeInActive = false;
        bool backgroundVisible = false;
        sf::Clock backgroundFadeClock;

        bool titleDropStarted = false;
        bool titleDropComplete = false;
        sf::Clock titleDropClock;

        std::vector<Location> locations;
        const Location* currentLocation = nullptr;
        std::optional<LocationId> keyboardMapHover;
        std::optional<LocationId> mouseMapHover;
        std::array<sf::FloatRect, 5> mapLocationHitboxes{};
        std::array<bool, 5> locationCompleted{};
        std::optional<LocationId> lastCompletedLocation;
        std::string lastDragonName;
        int dragonStoneCount = 0;

        std::vector<WeaponOption> weaponOptions;
        int hoveredWeaponIndex = -1;
        int selectedWeaponIndex = -1;
        bool weaponItemAdded = false;

        std::vector<DragonPortrait> dragonPortraits;
        DragonShowcaseState dragonShowcase;
        std::vector<ItemIcon> itemIcons;
        std::array<bool, 4> dragonstoneIconAdded{};
        QuizData quiz;
        FinalChoiceData finalChoice;
        std::vector<DialogueLine> transientDialogue;
        bool transientReturnToMap = false;
        bool holdMapDialogue = false;
        bool pendingTeleportToGonad = false;
        bool teleportActive = false;
        TeleportPhase teleportPhase = TeleportPhase::None;
        sf::Clock teleportClock;
        float teleportFadeOutDuration = 3.0f;
        float teleportCooldownDuration = 2.0f;
        float teleportFadeInDuration = 1.0f;
        std::optional<LocationId> teleportTarget;
        bool teleportMiddleStarted = false;
        bool teleportStopPlayed = false;
        bool finalEncounterPending = false;
        bool finalEncounterActive = false;
        bool finalEndingPending = false;
        bool endSequenceActive = false;
        bool endFadeOutActive = false;
        bool endFadeInActive = false;
        bool endScreenVisible = false;
        sf::Clock endClock;
        float endFadeOutDuration = 2.0f;
        float endFadeInDuration = 3.0f;
        bool startGameSoundPlayed = false;
        bool titleScreenMusicStarted = false;

        void addItemIcon(const sf::Texture& texture);
        void addDragonstoneIcon(LocationId id);
    void startTravel(LocationId id);
    void beginTeleport(LocationId id);
    void updateTeleport();
    void updateEndSequence();
    sf::Color frameBaseColor() const;
    sf::Color frameColor(float uiAlphaFactor) const;

    private:
        void loadWeaponOptions();
        void loadDragonPortraits();
        void startLocationMusic(LocationId id);
        void fadeOutLocationMusic(float duration);
        void updateSoundFades();
};
