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
#include "story/storyIntro.hpp"
#include "ui/nineSliceBox.hpp"
#include "resources/resources.hpp"
#include "core/state.hpp"
#include "story/textStyles.hpp"
#include "rendering/locations.hpp"
#include "ui/confirmationUI.hpp"

struct Game {
    struct WeaponOption {
        sf::Texture texture;
        std::optional<sf::Sprite> sprite;
        std::string displayName;
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

    Game();
    void run();
    void updateLayout();

    void startTypingSound();
    void stopTypingSound();

        // === Public game data ===
        sf::RenderWindow window;
        Resources resources;
        GameState state = GameState::IntroScreen;

        // === Dialogues ===
        size_t dialogueIndex = 0;
        const std::vector<DialogueLine>* currentDialogue = &intro;

        sf::RectangleShape nameBox;
        sf::RectangleShape textBox;
        sf::RectangleShape locationBox;
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

        std::vector<WeaponOption> weaponOptions;
        int hoveredWeaponIndex = -1;
        int selectedWeaponIndex = -1;

        std::vector<DragonPortrait> dragonPortraits;

    private:
        void loadWeaponOptions();
        void layoutWeaponSelection();
        void loadDragonPortraits();
        void layoutDragonPortraits();
};