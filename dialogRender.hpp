#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <utility>
#include <vector>
#include "game.hpp"
#include "speaker.hpp"
#include "storyIntro.hpp"

struct ColoredTextSegment {
    std::string text;
    sf::Color color = sf::Color::White;
};

inline std::vector<ColoredTextSegment> buildColoredSegments(const std::string& text) {
    std::vector<ColoredTextSegment> segments;

    if (text.empty())
        return segments;

    const std::array<Speaker, 7> speakersToColor{
        Speaker::StoryTeller,
        Speaker::NoNameNPC,
        Speaker::Player,
        Speaker::FireDragon,
        Speaker::WaterDragon,
        Speaker::AirDragon,
        Speaker::EarthDragon
    };

    std::vector<std::pair<std::string, sf::Color>> tokens;
    tokens.reserve(speakersToColor.size());
    for (Speaker speaker : speakersToColor) {
        std::string name = speakerToName(speaker);
        if (!name.empty())
            tokens.emplace_back(name, colorForSpeaker(speaker));
    }

    std::size_t cursor = 0;
    while (cursor < text.size()) {
        std::size_t matchPos = std::string::npos;
        int matchIndex = -1;

        for (std::size_t i = 0; i < tokens.size(); ++i) {
            const auto& token = tokens[i];
            std::size_t pos = text.find(token.first, cursor);
            if (pos != std::string::npos && (matchIndex == -1 || pos < matchPos)) {
                matchPos = pos;
                matchIndex = static_cast<int>(i);
            }
        }

        if (matchIndex == -1)
            break;

        if (matchPos > cursor)
            segments.push_back({ text.substr(cursor, matchPos - cursor), sf::Color::White });

        const auto& token = tokens[matchIndex];
        segments.push_back({ token.first, token.second });
        cursor = matchPos + token.first.size();
    }

    if (cursor < text.size())
        segments.push_back({ text.substr(cursor), sf::Color::White });

    if (segments.empty())
        segments.push_back({ text, sf::Color::White });

    return segments;
}

inline void drawColoredSegments(
    sf::RenderTarget& target,
    const sf::Font& font,
    const std::vector<ColoredTextSegment>& segments,
    sf::Vector2f startPos,
    unsigned int characterSize
) {
    if (segments.empty())
        return;

    const float baseLineStartX = startPos.x;
    sf::Vector2f cursor = startPos;
    sf::Text metrics(font, sf::String(), characterSize);
    metrics.setString("Hg");
    float lineSpacing = metrics.getLineSpacing();

    for (const auto& segment : segments) {
        if (segment.text.empty())
            continue;

        std::size_t offset = 0;
        while (offset <= segment.text.size()) {
            std::size_t newlinePos = segment.text.find('\n', offset);
            std::string part = (newlinePos == std::string::npos)
                ? segment.text.substr(offset)
                : segment.text.substr(offset, newlinePos - offset);

            if (!part.empty()) {
                sf::Text drawable(font, sf::String(), characterSize);
                drawable.setFillColor(segment.color);
                drawable.setString(part);
                drawable.setPosition(cursor);
                target.draw(drawable);
                cursor.x += drawable.getLocalBounds().size.x;
            }

            if (newlinePos == std::string::npos)
                break;

            offset = newlinePos + 1;
            cursor.x = baseLineStartX;
            cursor.y += lineSpacing;
        }
    }
}

inline void renderDialogue(Game& game) {
    game.window.draw(game.nameBox);
    game.window.draw(game.textBox);

    if (!game.currentDialogue || game.dialogueIndex >= game.currentDialogue->size())
        return;
    
    const auto& line = (*game.currentDialogue)[game.dialogueIndex];

    std::string fullText = injectSpeakerNames(line.text, game);

    float delay = 0.02f;
    if (!game.askingName && game.charIndex < fullText.size()) {
        if (game.typewriterClock.getElapsedTime().asSeconds() >= delay) {
            game.visibleText += fullText[game.charIndex];
            game.charIndex++;
            game.typewriterClock.restart();
        }
    }

    auto info = getSpeakerInfo(line.speaker);

    sf::Text nameText{game.font, "", 28};

    if (!info.name.empty()) {
        nameText.setFillColor(info.color);
        nameText.setString(info.name);

        auto namePos = game.nameBox.getPosition();
        nameText.setPosition({ namePos.x + 20.f, namePos.y + 20.f });
        game.window.draw(nameText);
    }

    auto textPos = game.textBox.getPosition();
    sf::Vector2f basePos{ textPos.x + 20.f, textPos.y + 20.f };

    auto segments = buildColoredSegments(game.visibleText);
    drawColoredSegments(game.window, game.font, segments, basePos, 28);

}