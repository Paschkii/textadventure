#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window.hpp>
#include <optional>
#include <memory>
#include <cmath>
#include "story/storyIntro.hpp"
#include "ui/nineSliceBox.hpp"
#include "resources/resources.hpp"
#include "core/state.hpp"
#include "story/textStyles.hpp"

struct Game {
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
        bool introPromptVisible = true;
        float introPromptBlinkInterval = 0.8f;

        std::string currentProcessedLine;

        sf::Clock introClock;
        float introFadeDuration = 1.0f;
        bool introFadeOutActive = false;
        float introFadeOutDuration = 3.0f;

        bool introDialogueFinished = false;
        bool uiFadeOutActive = false;
        float uiFadeOutDuration = 1.0f;
        sf::Clock uiFadeClock;

        bool backgroundFadeInActive = false;
        bool backgroundVisible = false;
        sf::Clock backgroundFadeClock;

        bool titleDropStarted = false;
        bool titleDropComplete = false;
        sf::Clock titleDropClock;
};