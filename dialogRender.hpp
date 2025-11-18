#pragma once
#include <SFML/Graphics.hpp>
#include "game.hpp"
#include "speaker.hpp"
#include "storyIntro.hpp"

inline void renderDialogue(Game& game) {
    game.window.draw(game.nameBox);
    game.window.draw(game.textBox);

    if (!game.currentDialogue || game.dialogueIndex >= game.currentDialogue->size())
        return;
    
    const auto& line = (*game.currentDialogue)[game.dialogueIndex];

    std::string fullText = line.text;

    if (game.currentDialogue == &intro
        && game.dialogueIndex == 4         // die „Ahhhh, so your name is “-Zeile
        && !game.playerName.empty()) {

        fullText += game.playerName + "!";
    }

    float delay = 0.02f;
    if (!game.askingName && game.charIndex < fullText.size()) {
        if (game.typewriterClock.getElapsedTime().asSeconds() >= delay) {
            game.visibleText += fullText[game.charIndex];
            game.charIndex++;
            game.typewriterClock.restart();
        }
    }

    auto info = getSpeakerInfo(line.speaker);

    std::string processed = injectSpeakerNames(game.visibleText, game);

    sf::Text nameText{game.font, "", 28};
    sf::Text dialogText{game.font, "", 28};
    sf::Text inputText{game.font, "", 28};

    if (!info.name.empty()) {
        nameText.setFillColor(info.color);
        nameText.setString(info.name);

        auto namePos = game.nameBox.getPosition();
        nameText.setPosition({ namePos.x + 20.f, namePos.y + 20.f });
        game.window.draw(nameText);
    }

    dialogText.setFillColor(sf::Color::White);
    dialogText.setString(processed);

    auto textPos = game.textBox.getPosition();
    sf::Vector2f basePos{ textPos.x + 20.f, textPos.y + 20.f };

    const std::string& name = game.playerName;

    // Wenn kein Name oder im Text (noch) nicht vorhanden → alles weiß
    if (name.empty()) {
        sf::Text t{ game.font, processed, 28 };
        t.setFillColor(sf::Color::White);
        t.setPosition(basePos);
        game.window.draw(t);
    } else {
        std::size_t pos = processed.find(name);
        if (pos == std::string::npos) {
            // Name noch nicht komplett in visibleText → einfach weiß
            sf::Text t{ game.font, processed, 28 };
            t.setFillColor(sf::Color::White);
            t.setPosition(basePos);
            game.window.draw(t);
        } else {
            std::string before = processed.substr(0, pos);
            std::string middle = processed.substr(pos, name.size());
            std::string after  = processed.substr(pos + name.size());

            // 1) Text vor dem Namen (weiß)
            sf::Text tBefore{ game.font, before, 28 };
            tBefore.setFillColor(sf::Color::White);
            tBefore.setPosition(basePos);
            game.window.draw(tBefore);

            auto bBefore = tBefore.getLocalBounds();
            basePos.x += bBefore.size.x; // „width“ in SFML 3

            // 2) Name farbig (Speaker::Player-Farbe)
            sf::Text tName{ game.font, middle, 28 };
            tName.setFillColor(colorForSpeaker(Speaker::Player)); // speaker.hpp
            tName.setPosition(basePos);
            game.window.draw(tName);

            auto bName = tName.getLocalBounds();
            basePos.x += bName.size.x;

            // 3) Rest wieder weiß
            sf::Text tAfter{ game.font, after, 28 };
            tAfter.setFillColor(sf::Color::White);
            tAfter.setPosition(basePos);
            game.window.draw(tAfter);
        }
    }
}