#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <memory>
#include "storyIntro.hpp"

class State;

class Game {
    public:
        Game();
        void run();
        void updateLayout();

        friend void waitForEnter(Game& game, const DialogueLine& line);
        friend void renderDialogue(Game& game);
        friend std::string injectSpeakerNames(const std::string& text, const Game& game);

        void changeState(std::unique_ptr<State> newState);
        // === Dialogues ===
        size_t dialogueIndex = 0;
        const std::vector<DialogueLine> *currentDialogue = &intro;

    private:
        sf::RenderWindow window;

        sf::Font font;

        sf::SoundBuffer buffer;
        sf::Sound sound;

        sf::RectangleShape nameBox;
        sf::RectangleShape textBox;

        std::string visibleText;
        std::size_t charIndex = 0;
        sf::Clock typewriterClock;

        std::string playerName;
        std::string nameInput;
        bool askingName = false;

        std::string currentProcessedLine;
};