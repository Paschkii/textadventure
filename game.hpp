#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window.hpp>
#include <memory>
#include <cmath>
#include "storyIntro.hpp"
#include "nineSliceBox.hpp"
#include "resources.hpp"
#include "state.hpp"
#include "textStyles.hpp"

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
        NineSliceBox uiFrame{12};

        sf::Sprite background;
        sf::Sprite returnSprite;

        std::string visibleText;
        std::size_t charIndex = 0;
        sf::Clock typewriterClock;
        sf::Clock uiGlowClock;

        sf::Sound textBlipSound;
        sf::Sound enterSound;

        std::string playerName;
        std::string nameInput;
        bool askingName = false;

        sf::Clock cursorBlinkClock;
        bool cursorVisible = true;
        float cursorBlinkInterval = 0.5f;

        sf::Clock returnBlinkClock;
        bool returnVisible = true;
        float returnBlinkInterval = 0.4f;

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
        sf::Clock titleDropClock;
};