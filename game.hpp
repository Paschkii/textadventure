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

class State;

class Game {
    public:
        Game();
        void run();
        void updateLayout();

        friend void waitForEnter(Game& game, const DialogueLine& line);
        friend void renderDialogue(Game& game);
        friend std::string injectSpeakerNames(const std::string& text, const Game& game);
        friend void renderIntroScreen(Game& game);
        friend void drawIntroTitle(Game& game, sf::RenderTarget& target);
        friend void drawDialogueUI(Game& game, sf::RenderTarget& target);
        friend void drawLocationBox(Game& game, sf::RenderTarget& target);

        void changeState(std::unique_ptr<State> newState);
        // === Dialogues ===
        size_t dialogueIndex = 0;
        const std::vector<DialogueLine> *currentDialogue = &intro;

    private:
        void startTypingSound();
        void stopTypingSound();

        Resources resources;

        sf::RenderWindow window;

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

        bool showingIntroScreen = true;

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