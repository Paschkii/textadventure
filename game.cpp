#include "game.hpp"
#include "dialogueLine.hpp"
#include "dialogInput.hpp"
#include "dialogRender.hpp"
#include "storyIntro.hpp"
#include "speaker.hpp"
#include <iostream>

constexpr unsigned int windowWidth = 1280;
constexpr unsigned int windowHeight = 720;
constexpr unsigned int fpsLimit = 60;

Game::Game()
: window(sf::VideoMode({windowWidth, windowHeight}), "Glandular", sf::Style::Titlebar | sf::Style::Close)
, font("assets/fonts/test.ttf")
, enterBuffer("assets/audio/enterkey.wav")
, enterSound(enterBuffer)
, textBlipBuffer("assets/audio/textblip.mp3")
, textBlipSound(textBlipBuffer)
{
    if (!enterBuffer.loadFromFile("assets/audio/enterkey.wav")) {
        std::cerr << "Failed to load enter sound." << std::endl;
    }
    else {
        enterSound.setBuffer(enterBuffer);
    }

    if (!textBlipBuffer.loadFromFile("assets/audio/textblip.mp3")) {
        std::cerr << "Failed to load text blip sound." << std::endl;
    }
    else {
        textBlipSound.setBuffer(textBlipBuffer);
        textBlipSound.setLooping(true);
    }
    
    // === Framerate limitieren ===
    window.setFramerateLimit(fpsLimit);
    // === NameBox Style setzen ===
    nameBox.setFillColor(sf::Color::Transparent);
    nameBox.setOutlineColor(sf::Color::White);
    nameBox.setOutlineThickness(2.f);
    // === TextBox Style setzen ===
    textBox.setFillColor(sf::Color::Transparent);
    textBox.setOutlineColor(sf::Color::White);
    textBox.setOutlineThickness(2.f);

    currentDialogue = &intro;
}

// === Layout updaten je nach Fenstergröße ===
void Game::updateLayout() {
    float w = window.getSize().x;
    float h = window.getSize().y;

    float marginX = w * 0.05f;      // 5% vom Rand (64)
    float marginY = h * 0.05f;      // 5% vom Rand (36)

    float boxHeight = h * 0.25f;    // beide Boxen: 25% Höhe (180)
    float nameWidth = w * 0.20f;    // NameBox: 20% Breite (256)
    float textWidth = w * 0.70f;    // TextBox: 70% Breite (896)

    // === NameBox: links unten, 5% Abstand ===
    nameBox.setSize({ nameWidth, boxHeight });
    nameBox.setPosition({
        marginX,
        h - boxHeight - marginY
    });

    // === TextBox: rechts unten, 5% Abstand ===
    textBox.setSize({ textWidth - (marginX / 2), boxHeight });
    textBox.setPosition({
        marginX + nameBox.getSize().x + (marginX / 2),
        h - boxHeight - marginY
    });
}


// === Öffnet das Fenster und führt Tasks aus ===
void Game::run() {
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {

            if (askingName && event->is<sf::Event::TextEntered>()) {
                const auto& text = event->getIf<sf::Event::TextEntered>();
                char32_t code = text->unicode;

                if (code == U'\r' || code == U'\n') {}
                else if (code == U'\b') {
                    if (!nameInput.empty())
                        nameInput.pop_back();
                }
                else if (code >= 32 && code < 127) {
                    nameInput.push_back(static_cast<char>(code));
                }
            }

            if (auto key = event->getIf<sf::Event::KeyReleased>()) {
                if (key->scancode == sf::Keyboard::Scan::Enter) {

                    enterSound.stop();
                    enterSound.play();

                    waitForEnter(*this, (*currentDialogue)[dialogueIndex]);
                }
            }

            if (event->is<sf::Event::Closed>())
                window.close();
        }

        updateLayout();

        window.clear();
        renderDialogue(*this);
        window.display();
    }
}

void Game::startTypingSound() {
    if (textBlipSound.getStatus() != sf::Sound::Status::Playing) {
        std::cout << "start on " << this << " / sound " << &textBlipSound << "\n";
        textBlipSound.play();
    }
}

void Game::stopTypingSound() {
    if (textBlipSound.getStatus() == sf::Sound::Status::Playing) {
        std::cout << "end on " << this << " / sound " << &textBlipSound << "\n";
        textBlipSound.stop();
    }
}