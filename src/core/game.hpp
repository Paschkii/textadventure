#pragma once
// === C++ Libraries ===
#include <array>     // Needed for the fixed-size option and hitbox collections.
#include <cstdint>
#include <cstddef>   // Provides std::size_t for the many index and size fields.
#include <limits>    // Supplies std::numeric_limits used when tracking dialogue indexes.
#include <memory>
#include <optional>  // Stores optional state like hovered locations and cached sounds.
#include <string>    // Holds player names, dialogue text, and UI labels.
#include <utility>   // Enables std::move in constructors such as DragonPortrait.
#include <vector>    // Manages dynamic lists of dialogue lines, locations, and UI elements.
#include <unordered_set>
#include <unordered_map>
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
#include "story/quests.hpp"                     // Brings quest definitions used for logging and triggering.
#include "story/textStyles.hpp"                 // Defines TextStyles::SpeakerId and style helpers.
#include "ui/confirmationUI.hpp"                // Brings the ConfirmationPrompt used in the game state.
#include "ui/creditsUI.hpp"                     // Stores the credits sequence state.
#include "ui/nineSliceBox.hpp"                  // Declares the NineSliceBox frame used for the UI.
#include "ui/quizGenerator.hpp"                 // Defines quiz::Question for the quiz data structure.
#include "ui/mapSelectionUI.hpp"                // Needed for caching map popup metadata.
#include "items/itemRegistry.hpp"
#include "core/ranking.hpp"                     // Tracks leaderboard entries persisted on disk.
#include "ui/rankingUI.hpp"                     // Renders the ranking overlay once the ending completes.

struct BattleDemoState {
    enum class Phase {
        PlayerChoice,
        PlayerAction,
        EnemyAction,
        Victory,
        Complete
    };
    enum class IntroPhase {
        Blinking,
        Shading,
        Complete
    };

    static constexpr std::size_t kSkillSlotCount = 4;
    enum class SkillElement {
        None,
        Air,
        Earth,
        Fire,
        Water
    };

    struct Combatant {
        std::string name;
        int level = 0;
        float hp = 0.f;
        float maxHp = 0.f;
        std::array<std::optional<std::string>, kSkillSlotCount> skills;
    };

    static constexpr std::size_t kActionOptionCount = 4;

    Combatant player{
        "Dragonborn",
        50,
        980.f,
        980.f,
        {{
            std::optional<std::string>{"Air Slash"},
            std::optional<std::string>{"Fire Slash"},
            std::optional<std::string>{"Earth Slash"},
            std::optional<std::string>{"Water Slash"}
        }}
    };
    Combatant enemy{
        "Master Bates",
        100,
        360.f,
        360.f,
        {{
            std::optional<std::string>{"Midnight Release"},
            std::optional<std::string>{"Bad Habit"},
            std::nullopt,
            std::nullopt
        }}
    };
    Phase phase = Phase::PlayerChoice;
    IntroPhase introPhase = IntroPhase::Blinking;
    float introBlinkTimer = 0.f;
    int introBlinkCount = 0;
    bool introBlinkVisible = true;
    float introShadeProgress = 0.f;
    int selectedAction = 0;
    float actionTimer = 0.f;
    float playerActionDelay = 0.75f;
    float enemyActionDelay = 0.65f;
    float victoryHoldTime = 1.4f;
            bool victoryTransitioned = false;
            int enemyMoveIndex = 0;
            float platformEntranceTimer = 0.f;
            float platformEntranceDuration = 1.75f;
            bool actionMenuVisible = false;
    enum class CreatureMenuType {
        None,
        Glandumon,
        Dragons
    };
    CreatureMenuType creatureMenuType = CreatureMenuType::None;
    bool creatureMenuVisible = false;
    int creatureMenuSelection = 0;
    std::vector<sf::FloatRect> creatureMenuEntryBounds;
    sf::FloatRect creatureMenuCancelBounds;
    bool creatureMenuAllowCancel = true;
    std::array<sf::FloatRect, kActionOptionCount> actionOptionBounds;
    bool actionOptionBoundsValid = false;
    std::array<sf::FloatRect, kSkillSlotCount> fightOptionBounds;
    bool fightOptionBoundsValid = false;
    sf::FloatRect fightCancelBounds;
            sf::Clock completionClock;
            struct LogEntry {
                std::string message;
                std::vector<std::pair<std::string, sf::Color>> highlightTokens;
            };
            std::vector<LogEntry> logHistory{
                { "A wild Master Bates appeared!", {} }
            };
            bool fightMenuVisible = false;
            int fightMenuSelection = 0;
            bool fightCancelHighlight = false;
            bool reopenMenuAfterPlayerPulse = false;
            struct HpPulse {
        bool active = false;
        float startHp = 0.f;
        float endHp = 0.f;
        sf::Clock clock;
    };
    HpPulse playerHpPulse;
    HpPulse enemyHpPulse;
    float playerDisplayedHp = player.hp;
    float enemyDisplayedHp = enemy.hp;
    sf::Vector2f cachedPlayerCenter{ 0.f, 0.f };
    sf::Vector2f cachedEnemyCenter{ 0.f, 0.f };
    const sf::Texture* playerBackSprite = nullptr;
    struct SkillEffect {
        enum class Phase {
            Idle,
            Slash,
            Weapon,
            FadeOut
        };
        enum class Target {
            None,
            Player,
            Enemy
        };
        bool active = false;
        Phase phase = Phase::Idle;
        Target target = Target::None;
        SkillElement element = SkillElement::None;
        float blinkTimer = 0.f;
        int blinkCycles = 0;
        bool slashVisible = true;
        float fadeTimer = 0.f;
        float weaponFade = 1.f;
        bool pendingDamage = false;
        float pendingHp = 0.f;
        std::optional<sf::Sound> slashSound;
        std::optional<sf::Sound> elementSound;
            };
            SkillEffect skillEffect;
    struct FriendshipEffect {
        enum class Target {
            None,
            Player,
            Enemy
        };
        bool active = false;
        Target target = Target::None;
        std::array<const sf::Texture*, 3> textures{};
        std::array<sf::Vector2f, 3> offsets{};
        sf::Vector2f startCenter{ 0.f, 0.f };
        sf::Vector2f targetCenter{ 0.f, 0.f };
        enum class Phase {
            Standing,
            Throwing
        } phase = Phase::Standing;
        float timer = 0.f;
        float standDuration = 5.f;
        float throwDuration = 2.f;
        float fade = 1.f;
        std::optional<sf::Sound> sound;
        bool hpPulseTriggered = false;
        bool pendingDamage = false;
        float pendingHp = 0.f;
    };
            FriendshipEffect friendshipEffect;
            struct MasterBatesSkillEffect {
                enum class Stage {
                    Powerup,
                    Skill
                };
                enum class Target {
                    Player,
                    Enemy
                };
                Stage stage = Stage::Powerup;
                bool active = false;
                Target target = Target::Player;
                const sf::Texture* powerupTexture = nullptr;
                const sf::Texture* skillTexture = nullptr;
                const sf::SoundBuffer* skillSoundBuffer = nullptr;
                std::optional<sf::Sound> sound;
                bool pendingDamage = false;
                float pendingHp = 0.f;
            };
            MasterBatesSkillEffect masterBatesSkillEffect;
            struct SwapPrompt {
                bool active = false;
                int selectedButton = 0;
                Combatant candidate;
                const sf::Texture* backSprite = nullptr;
                bool candidateIsDragonborn = false;
            };
            struct SwapAnimation {
                enum class Stage {
                    Inactive,
                    OutgoingWhite,
                    OutgoingSoftRed,
                    IncomingSoftRedToWhite,
                    IncomingWhiteToNormal
                };
                bool active = false;
                Stage stage = Stage::Inactive;
                float timer = 0.f;
                Combatant pendingCombatant;
                const sf::Texture* pendingBackSprite = nullptr;
                bool pendingIsDragonborn = false;
            };
            SwapPrompt swapPrompt;
            SwapAnimation swapAnimation;
            struct MasterBatesEvolution {
                enum class Stage {
                    Idle,
                    FirstSound,
                    FadeWhite,
                    FadePurple,
                    ChickSound,
                    AwwSound
                };
                Stage stage = Stage::Idle;
                bool active = false;
                float timer = 0.f;
                float whiteDuration = 2.f;
                float purpleDuration = 2.f;
                sf::Color tint = sf::Color::White;
                sf::Vector2f shakeOffset{ 0.f, 0.f };
                float shakeTimer = 0.f;
                float shakeInterval = 0.06f;
                float shakeStrength = 6.f;
                std::optional<sf::Sound> sound;
            };
            MasterBatesEvolution masterBatesEvolution;
            bool masterBatesDragonActive = false;
            struct ForcedRetreat {
                bool active = false;
                std::string blockedName;
                bool awaitingSwap = false;
                std::string pendingName;
            };
            ForcedRetreat forcedRetreat;
            bool currentDragonbornActive = true;
            std::unordered_set<std::string> defeatedCreatures;
            int dragonbornLevel = 50;
            std::unordered_map<std::string, float> creatureHp;
            std::unordered_map<std::string, float> creatureMaxHp;
            std::mt19937 rng{ std::random_device{}() };
            std::unique_ptr<sf::Music> battleMusic;
            bool battleMusicPlaying = false;
        };

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

    // Tracks the popup that previews the broken weapons during Perigonal dialogue.
    struct BrokenWeaponPopup {
        enum class Phase {
            Hidden,
            PopupFadingIn,
            WeaponsFadingIn,
            Visible,
            FadingOut
        };

        Phase phase = Phase::Hidden;
        sf::Clock fadeClock;
        float popupAlpha = 0.f;
        float weaponAlpha = 0.f;
        float popupFadeStart = 0.f;
        float weaponFadeStart = 0.f;
    };

    struct WeaponForgingState {
        enum class Phase {
            Idle,
            FadingOut,
            Sleeping,
            FadingIn
        };

        Phase phase = Phase::Idle;
        sf::Clock clock;
        float alpha = 0.f;
        bool autoAdvancePending = false;
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

        struct QuestLogEntry {
            std::string name;
            std::string giver;
            std::string goal;
            int xpReward = 0;
            std::optional<std::string> loot;
            bool completed = false;
            bool rewardGranted = false;
            bool collapsed = false;
            float foldProgress = 1.f;
        };

    struct QuestPopupState {
        enum class Phase {
            Idle,
            Entering,
            Visible,
            Exiting
        };

        Phase phase = Phase::Idle;
        sf::Clock clock;
        std::optional<QuestLogEntry> entry;
        std::string message;
    };

        struct TreasureChestState {
            TreasureChestState()
            : rng(std::random_device{}()) {}

            LocationId targetLocation = LocationId::Gonad;
            std::vector<std::string> rewardKeys;
            std::size_t rewardIndex = 0;
            float chestFade = 0.f;
            bool chestVisible = false;
            bool rewardPopupReady = false;
            bool confirmationHovered = false;
            sf::FloatRect confirmationBounds{};
            std::string confirmationLabel;
            bool sequenceComplete = false;
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
        void setCurrentLocation(const Location* location, bool updateBackground = true);
    // Switches the fullscreen background to the requested texture.
    void setBackgroundTexture(const sf::Texture& texture);
    // Plays the title screen music.
    void startTitleScreenMusic();
    // Fades out the title screen music over the given time.
    void fadeOutTitleScreenMusic(float duration);
    // Grants the provided amount of XP, handling level progression if needed.
    void grantXp(int amount);
    // Instantly boosts the player to the requested level.
    void boostToLevel(int targetLevel);
    void startQuest(const Story::QuestDefinition& quest);
    void completeQuest(const Story::QuestDefinition& quest);

        // === Public game data ===
        sf::RenderWindow window;                            // Main SFML window for rendering.
        Resources resources;                                // Central texture/audio assets store.
        AudioManager audioManager;                          // Music and sound effect manager.
        core::TeleportController teleportController;        // Handles teleport animations.
        core::EndSequenceController endSequenceController;  // Final-overlay sequence control.
        BattleDemoState battleDemo;
        GameState state = GameState::IntroScreen;             // Current UI/game mode.

        // === Dialogues ===
        size_t dialogueIndex = 0;                                   // Current line index inside the active dialogue.
        const std::vector<DialogueLine>* currentDialogue = &intro;  // Active dialogue pool.
        std::optional<TextStyles::SpeakerId> lastSpeaker;           // Last speaker shown.

        sf::RectangleShape nameBox;                      // Draws the speaker name frame.
        sf::RectangleShape playerStatusBox;               // Shows HP/XP bars above the name box.
        bool playerStatusFolded = false;                  // Tracks whether the status box is folded.
        float playerStatusFoldProgress = 1.f;             // Animates fold/unfold transitions.
        sf::FloatRect playerStatusFoldBarBounds;          // Click target for the fold bar.
        sf::RectangleShape optionsBox;                    // Highlights hovered intro options.
        sf::RectangleShape introOptionBackdrop;           // Backdrop behind intro options.
        float playerHp = 5.f;                            // Player HP value for the status bar.
        float playerHpMax = 485.f;                       // Maximum HP used for the ratio display.
        bool inventoryArrowActive = false;                // Signals that the helper arrow should be visible.
        bool inventoryTutorialPending = false;            // Waiting for the menu to open after the arrow line.
        bool inventoryTutorialPopupActive = false;        // Shows the tutorial popup once the menu opens.
        bool inventoryTutorialCompleted = false;          // Prevents re-triggering the tutorial after it ran.
        sf::Clock inventoryArrowBlinkClock;               // Drives the arrow blinking effect.
        bool inventoryArrowVisible = true;                // Tracks the arrow's visible/blink state.
        sf::FloatRect inventoryTutorialButtonBounds;      // Hitbox for the Understood button inside the popup.
        bool inventoryTutorialButtonHovered = false;      // Hover state used for the tutorial button highlight.
        bool questTutorialPending = false;                // Waiting for the quest tab tutorial to open.
        bool questTutorialPopupActive = false;            // Shows the quest tutorial overlay.
        bool questTutorialCompleted = false;              // Prevents repeating the quest tutorial.
        bool questTutorialButtonHovered = false;          // Hover state for the quest tutorial button.
        bool questTutorialClosing = false;                // Signals that the quest tutorial is fading out.
        float questTutorialCloseProgress = 0.f;           // Tracks fade progress while closing.
        sf::Clock questTutorialCloseClock;                // Drives the quest tutorial fade-out timer.
        sf::FloatRect questTutorialButtonBounds;          // Hitbox for the quest tutorial button.
        bool questTutorialAdvancePending = false;          // Signals to advance dialogue after the tutorial closes.
        bool mapItemPopupActive = false;                  // Shows the map-acquisition popup during Gonad part two.
        bool mapItemCollected = false;                    // Ensures the map icon is only added once.
        bool mapTutorialActive = false;                   // Controls the Tory Tailor map tutorial.
        bool mapTutorialAwaitingOk = false;               // Blocks advancement until the Ok button is clicked.
        sf::Vector2f mapTutorialAnchorNormalized{ 0.5f, 0.5f }; // Normalized anchor point for the tutorial popups.
        std::optional<LocationId> mapTutorialHighlight;   // Location forced to highlight while the tutorial runs.
        sf::FloatRect mapTutorialPopupBounds;             // Cached bounds of the current tutorial popup.
        sf::FloatRect mapTutorialOkBounds;                // Hitbox for the popup's Ok button.
        bool mapTutorialOkHovered = false;                // Mouse hover state for the Ok button.
        bool mapInteractionUnlocked = false;               // Gates menu map teleport selection.
        std::optional<MapPopupRenderData> menuMapPopup;    // Cached map popup data from the menu tab.
        bool menuMapUmbraOverlayActive = false;           // Shows the Umbra overlay on the menu map.
        bool menuMapUmbraOverlayFadeInActive = false;      // Animates the overlay fade-in.
        bool menuMapUmbraOverlayHold = false;              // Holds dialogue until the overlay finishes fading.
        sf::Clock menuMapUmbraOverlayClock;                // Tracks the overlay fade timing.
        std::optional<sf::FloatRect> menuMapUmbraOverlayContentBounds; // Cached normalized overlay bounds.
        sf::FloatRect menuMapUmbraOverlayBounds{};         // Clickable bounds for the Umbra overlay.
        bool healingPotionActive = false;                 // Tracks whether a healing sequence is running.
        bool healingPotionReceived = false;               // Ensures the potion is only granted once.
        float healingPotionStartHp = 0.f;                 // HP recorded when the potion started healing.
        sf::Clock healingPotionClock;                     // Drives the healing interpolation timer.
        bool emergencyHealingActive = false;               // Indicates Wanda is patching up the player.
        int emergencyHealCount = 0;                       // Tracks how many emergency heals Wanda performed.
        float playerXp = 0.f;                            // Player XP value for the status bar.
        float playerXpMax = 100.f;                       // XP required for the next level.
        int playerLevel = 1;                             // Current player level used for XP scaling.
        int pendingLevelUps = 0;                         // Levels earned but not yet shown in the UI.
        struct XpGainSegment {
            float startRatio = 0.f;
            float endRatio = 0.f;
            bool waitForLevelUp = false;
        };
        struct XpGainDisplay {
            bool active = false;
            int amount = 0;
            sf::Clock clock;
            std::vector<XpGainSegment> segments;
            std::size_t currentSegment = 0;
            bool waitingForLevelUp = false;
        };
        XpGainDisplay xpGainDisplay;                      // Controls the XP gain visual effect.
        struct LevelUpDisplay {
            bool active = false;
            sf::Clock clock;
        };
        LevelUpDisplay levelUpDisplay;
        float xpBarDisplayRatio = 0.f;                     // Smoothly animates XP bar fill.
        struct HpDamagePulse {
            bool active = false;
            float startHp = 0.f;
            float endHp = 0.f;
            sf::Clock clock;
            bool pendingHealing = false;
        };
        HpDamagePulse hpDamagePulse;                       // Animates HP loss similar to healing.
        struct HpLossDisplay {
            bool active = false;
            int amount = 0;
            sf::Clock clock;
            float duration = 1.f;
        };
        HpLossDisplay hpLossDisplay;                       // Draws the "-100" popup next to the HP bar.
        struct CriticalHpNotice {
            bool active = false;
            float duration = 3.f;
            sf::Clock clock;
            std::string message;
        };
        CriticalHpNotice criticalHpNotice;                 // Shows Wanda's warning when HP hits the floor.
        std::vector<QuestLogEntry> questLog;              // Track quests the player has been awarded.
        QuestPopupState questPopup;                       // Controls the quest popups shown at the top.
        std::vector<sf::FloatRect> questFoldButtonBounds; // Active fold button hitboxes.
        int questFoldHoveredIndex = -1;                   // Hovered fold button index.
        float questActiveScrollOffset = 0.f;              // Vertical offset used when scrolling active quests.
        float questFinishedScrollOffset = 0.f;            // Vertical offset used when scrolling finished quests.
        float questActiveMaxScroll = 0.f;                 // Max scroll range available for active quests.
        float questFinishedMaxScroll = 0.f;               // Max scroll range available for finished quests.
        sf::FloatRect questActiveColumnBounds{};           // Cached bounds for the active quest column.
        sf::FloatRect questFinishedColumnBounds{};         // Cached bounds for the finished quest column.
        sf::RectangleShape textBox;                      // Outline around dialogue text.
        sf::RectangleShape locationBox;                  // Box showing the current location.
        sf::RectangleShape itemBox;                      // Outline for the item list.
        sf::RectangleShape weaponPanel;                  // Weapon selector background.
        sf::RectangleShape menuButton;                   // Button that opens the in-game menu.
        sf::RectangleShape menuPanel;                    // Panel shown when the menu is active.
        std::array<sf::FloatRect, 3> menuTabBounds{};    // Click/touch areas for menu tabs.
        NineSliceBox uiFrame{12};                        // Decorative frame around UI elements.
        bool menuActive = false;                         // Determines whether the menu overlay is visible.
        bool menuButtonHovered = false;                   // Tracks hover state for the menu button.
        int menuHoveredTab = -1;                          // Hovered tab index.
        int menuActiveTab = 0;                            // Active tab index when the menu is open.
        bool menuButtonUnlocked = false;                  // Menu becomes available once Tory points it out.
        bool menuButtonFadeActive = false;                // Tracks whether the button is currently fading in.
        float menuButtonAlpha = 0.f;                      // Fade progress used for button visibility.
        sf::Clock menuButtonFadeClock;                    // Drives the 1-second menu button fade.
        bool forcedDestinationSelection = false;          // Locks the menu on the map until a destination is chosen.

        struct CharacterMenuState {
            enum class Category {
                Equipment = 0,
                Artifacts
            };
            Category activeCategory = Category::Equipment;
            std::array<sf::FloatRect, 2> categoryButtonBounds{};
            int hoveredButton = -1;
            sf::FloatRect cloakButtonBounds{};
            bool cloakButtonHovered = false;
        };
        CharacterMenuState characterMenu;
        struct EquipmentState {
            const sf::Texture* leftHand = nullptr;
            const sf::Texture* rightHand = nullptr;
            std::optional<std::string> leftKey;
            std::optional<std::string> rightKey;
        };
        EquipmentState equippedWeapons;
        struct RingEquipment {
            std::array<std::optional<std::string>, 4> slots{};
            std::size_t nextSlotIndex = 0;
        };
        RingEquipment ringEquipment;
        struct ArtifactSlots {
            std::array<std::array<std::optional<std::string>, 4>, 4> slots{};
        };
        ArtifactSlots artifactSlots;
        std::array<int, 4> artifactCounts{};
        std::array<std::optional<std::string>, 5> emblemSlots{};
        std::array<std::optional<std::string>, 5> trophySlots{};

        std::optional<sf::Sprite> background;             // Background art for the current scene.
        std::optional<sf::Sprite> returnSprite;           // Icon drawn when returning to map.
        const sf::Texture* queuedBackgroundTexture = nullptr; // Next background to fade in.

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
        std::optional<sf::Sound> menuOpenSound;             // Plays when the menu opens.
        std::optional<sf::Sound> menuCloseSound;            // Plays when the menu closes.
        std::optional<sf::Sound> healPotionSound;           // Plays when the healing potion restores HP.
        std::optional<sf::Sound> forgeSound;                // Plays while the blacksmith rests.
        std::optional<sf::Sound> levelUpSound;              // Triggered when the player levels up.
        std::optional<sf::Sound> questStartSound;           // Plays when a new quest is granted.
        std::optional<sf::Sound> questEndSound;             // Plays when a quest finishes.

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
        std::array<sf::FloatRect, 2> introTitleOptionBounds{};
        int introTitleHoveredOption = -1;

        bool genderSelectionActive = false;
        int genderSelectionHovered = -1;
        std::array<sf::FloatRect, 2> genderSelectionBounds{};
        DragonbornGender playerGender = DragonbornGender::Female;
        bool cloakEquipped = false;
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
            float approachDuration = 0.95f;
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
        bool pendingPerigonalDialogue = false;           // Queue the perigonal dialogue before Gonad.
        bool pendingGonadPartOneDialogue = false;        // Queue the first Gonad segment after Perigonal.
        bool pendingBlacksmithDialogue = false;          // Queue the blacksmith segment after Gonad Part One.
        bool pendingGonadPartTwoDialogue = false;        // Queue the second Gonad segment after the blacksmith.
        float uiFadeInDuration = 1.0f;                   // Fade-in duration.

        bool backgroundFadeInActive = false;              // Background fade animation running.
        bool backgroundVisible = false;                   // Should the background be drawn?
        sf::Clock backgroundFadeClock;                    // Timer for background fading.

        std::vector<Location> locations;                    // All locations available for travel.
        const Location* currentLocation = nullptr;          // Currently active location pointer.
        Location blacksmithLocation;                        // Special location used for the forge dialogue.
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
        std::string forgedWeaponName;                    // Display name of the newly forged weapon.
        bool brokenWeaponsStored = false;                // Ensures the broken weapons are recorded only once.
        struct WeaponSelectionPopupEntry {
            std::size_t optionIndex = 0;
            sf::FloatRect bounds;
            sf::Vector2f labelPosition;
        };
        std::vector<WeaponSelectionPopupEntry> weaponSelectionPopupEntries; // Layout used while picking a forged weapon.
        struct InventoryItemSlot {
            sf::FloatRect bounds;
            std::optional<std::size_t> iconIndex;
            std::optional<std::string> key;
        };
        std::vector<DragonPortrait> dragonPortraits;    // Portraits used in the dragon showcase UI.
        DragonShowcaseState dragonShowcase;             // State machine for the showcase animation.
        BrokenWeaponPopup brokenWeaponPopup;             // Handles the broken weapon preview popup.
        WeaponForgingState weaponForging;                // Tracks the 5-second forge rest animation.
        bool forgedWeaponPopupActive = false;             // Controls the new weapon reveal popup.
        core::ItemController itemController;            // Controls collected items.
        std::vector<InventoryItemSlot> inventoryItemSlots;
        int hoveredInventoryItem = -1;
        QuizData quiz;                                  // Manages quiz mode state and lines.
        TreasureChestState treasureChest;
        FinalChoiceData finalChoice;                    // Final choice UI state.
        std::vector<DialogueLine> transientDialogue;    // Temporary dialogue content.
        bool transientReturnToMap = false;              // Return map triggered when transient dialogue ends.
        bool pendingReturnToMenuMap = false;             // Signals a menu map should open instead of the old layout.
        bool holdMapDialogue = false;                   // Prevents map dialogue updates.
        bool pendingTeleportToGonad = false;            // Teleport to Gonad next frame.
        bool pendingTeleportToSeminiferous = false;     // Teleport to Seminiferous once the Umbra sequence ends.
        bool finalEncounterPending = false;             // Indicates final encounter is queued.
        bool finalEncounterActive = false;              // Final encounter currently running.
        bool battleReturnToSeminiferous = false;        // Return to Seminiferous dialogue after battle.
        bool finalEndingPending = false;                // Ending sequence is next.
        bool creditsAfterEndPending = false;            // Credits should start after the end screen.
        bool creditsAfterEndTimerActive = false;        // Timer started once THE END is visible.
        sf::Clock creditsAfterEndClock;                 // Delay before starting credits.
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
        ui::credits::State creditsState;                 // State for the credits sequence.
    // Hand off location travel to the dedicated handler.
    void startTravel(LocationId id);
    void beginForcedDestinationSelection();
    void exitForcedDestinationSelection();
    void setMenuActive(bool active);
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
