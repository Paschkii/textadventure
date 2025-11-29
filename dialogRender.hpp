#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <cctype>
#include <utility>
#include <vector>
#include "game.hpp"
#include "introScreen.hpp"
#include "speaker.hpp"
#include "storyIntro.hpp"

struct ColoredTextSegment {
    std::string text;
    sf::Color color = sf::Color::White;
};

inline std::size_t longestPartialSpeakerPrefix(
    const std::string& text,
    const std::vector<std::pair<std::string, sf::Color>>& tokens,
    sf::Color& outColor
) {
    std::size_t bestLength = 0;

    for (const auto& token : tokens) {
        if (token.first.empty())
            continue;

        unsigned char firstChar = static_cast<unsigned char>(token.first.front());
        if (!std::isalpha(firstChar))
            continue;

        std::size_t maxLength = std::min(token.first.size(), text.size());
        if (maxLength == 0)
            continue;

        for (std::size_t len = 1; len < token.first.size() && len <= maxLength; ++len) {
            if (text.compare(text.size() - len, len, token.first, 0, len) == 0) {
                if (len > bestLength) {
                    bestLength = len;
                    outColor = token.second;
                }
            }
        }
    }

    return bestLength;
}

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

    sf::Color partialColor = sf::Color::White;
    std::size_t partialLength = longestPartialSpeakerPrefix(text, tokens, partialColor);
    if (partialLength > 0) {
        std::string trailing = text.substr(text.size() - partialLength);
        std::size_t remaining = partialLength;

        while (remaining > 0 && !segments.empty()) {
            auto& last = segments.back();
            if (last.text.size() <= remaining) {
                remaining -= last.text.size();
                segments.pop_back();
            } else {
                last.text.erase(last.text.size() - remaining);
                remaining = 0;
            }
        }

        segments.push_back({ trailing, partialColor });
    }

    return segments;
}

inline sf::Vector2f drawColoredSegments(
    sf::RenderTarget& target,
    const sf::Font& font,
    const std::vector<ColoredTextSegment>& segments,
    sf::Vector2f startPos,
    unsigned int characterSize,
    float maxWidth
) {
    if (segments.empty())
        return startPos;

    const float baseLineStartX = startPos.x;
    const float wrapLimit = baseLineStartX + std::max(0.f, maxWidth);
    sf::Vector2f cursor = startPos;
    sf::Text metrics(font, sf::String(), characterSize);
    metrics.setString("Hg");
    float lineSpacing = metrics.getLineSpacing();
    const float lineAdvance = lineSpacing * 40.f;

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
                
                std::size_t partIndex = 0;
                while (partIndex < part.size()) {
                    bool isSpace = std::isspace(static_cast<unsigned char>(part[partIndex]));
                    std::string token;

                    while (partIndex < part.size()) {
                        char c = part[partIndex];
                        bool currentIsSpace = std::isspace(static_cast<unsigned char>(c));
                        if (currentIsSpace != isSpace)
                            break;
                        token.push_back(c);
                        ++partIndex;
                    }

                    if (token.empty())
                        continue;

                    if (isSpace) {
                        drawable.setString(token);
                        float tokenWidth = drawable.getLocalBounds().size.x;

                        if (cursor.x == baseLineStartX)
                            continue;

                        if (cursor.x + tokenWidth > wrapLimit) {
                            cursor.x = baseLineStartX;
                            cursor.y += lineAdvance;
                            continue;
                        }

                        drawable.setPosition(cursor);
                        target.draw(drawable);
                        cursor.x += tokenWidth;
                    } else {
                        auto drawSplitToken = [&](const std::string& word) {
                            std::string currentChunk;

                            for (char c : word) {
                                std::string nextChunk = currentChunk + c;
                                drawable.setString(nextChunk);
                                float chunkWidth = drawable.getLocalBounds().size.x;

                                float availableWidth = wrapLimit - cursor.x;
                                if (availableWidth <= 0.f) {
                                    cursor.x = baseLineStartX;
                                    cursor.y += lineAdvance;
                                    availableWidth = wrapLimit - cursor.x;
                                }

                                if (chunkWidth > availableWidth && !currentChunk.empty()) {
                                    drawable.setString(currentChunk);
                                    drawable.setPosition(cursor);
                                    target.draw(drawable);
                                    cursor.x += drawable.getLocalBounds().size.x;
                                    cursor.x = baseLineStartX;
                                    cursor.y += lineAdvance;
                                    currentChunk.clear();
                                    drawable.setString(nextChunk = std::string(1, c));
                                    chunkWidth = drawable.getLocalBounds().size.x;
                                }

                                if (cursor.x + chunkWidth > wrapLimit && currentChunk.empty()) {
                                    drawable.setPosition(cursor);
                                    target.draw(drawable);
                                    cursor.x += chunkWidth;
                                    continue;
                                }

                                currentChunk = nextChunk;
                            }

                            if (!currentChunk.empty()) {
                                drawable.setString(currentChunk);
                                drawable.setPosition(cursor);
                                target.draw(drawable);
                                cursor.x += drawable.getLocalBounds().size.x;
                            }
                        };

                            drawable.setString(token);
                            float tokenWidth = drawable.getLocalBounds().size.x;

                            float availableWidth = wrapLimit - cursor.x;
                            if (availableWidth <= 0.f) {
                                cursor.x = baseLineStartX;
                                cursor.y += lineAdvance;
                                availableWidth = wrapLimit - cursor.x;
                            }

                        if (tokenWidth <= availableWidth) {
                            drawable.setPosition(cursor);
                            target.draw(drawable);
                            cursor.x += tokenWidth;
                        } else if (tokenWidth <= maxWidth) {
                            cursor.x = baseLineStartX;
                            cursor.y += lineAdvance;
                            drawable.setPosition(cursor);
                            target.draw(drawable);
                            cursor.x += tokenWidth;
                        } else {
                            drawSplitToken(token);
                        }
                    }
                }
            }

            if (newlinePos == std::string::npos)
                break;

            offset = newlinePos + 1;
            cursor.x = baseLineStartX;
            cursor.y += lineAdvance;
        }
    }

    return cursor;
}

inline void renderDialogue(Game& game) {
    if (game.showingIntroScreen) {
        renderIntroScreen(game);
        return;
    }

    float t = game.uiGlowClock.getElapsedTime().asSeconds();
    // float pulse = (std::sin(t * 3.f) + 1.f) * 0.5f; // 0..1
    float flicker = (std::sin(t * 25.f) + std::sin(t * 41.f)) * 0.25f; 

    float alpha = 140.f + flicker * 30.f;   // 140 ± 15 → 125..155

    sf::Color glowColor = ColorHelper::UI::PanelBlueLight;
    glowColor.a = static_cast<std::uint8_t>(std::clamp(alpha, 0.f, 255.f));

    sf::Vector2f pos  = game.textBox.getPosition();
    sf::Vector2f size = game.textBox.getSize();

    // float baseMargin = 6.f;
    // float sizePulse = pulse * 2.f;

    // sf::RectangleShape glow;
    // glow.setPosition(pos - sf::Vector2f(baseMargin + sizePulse, baseMargin + sizePulse));
    // glow.setSize(size + sf::Vector2f((baseMargin + sizePulse) * 2.f, (baseMargin + sizePulse) * 2.f));
    // glow.setFillColor(glowColor);

    // 1. Glow
    // float glowOffset = 2.f + pulse * 1.5f; // zwischen 2px und 3.5px

    game.uiFrame.drawScaled(
        game.window,
        game.textBox.getPosition(),
        game.textBox.getSize(),
        glowColor,
        2.f
    );

    game.uiFrame.drawScaled(
        game.window,
        game.nameBox.getPosition(),
        game.nameBox.getSize(),
        glowColor,
        2.f
    );

    game.uiFrame.draw(game.window, game.nameBox);
    game.uiFrame.draw(game.window, game.textBox);

    if (!game.currentDialogue || game.dialogueIndex >= game.currentDialogue->size())
        return;
    
    const auto& line = (*game.currentDialogue)[game.dialogueIndex];

    std::string fullText = injectSpeakerNames(line.text, game);

    float delay = 0.02f;
    bool isTyping = !game.askingName && game.charIndex < fullText.size();

    if (isTyping) {
        game.startTypingSound();
        if (game.typewriterClock.getElapsedTime().asSeconds() >= delay) {
            game.visibleText += fullText[game.charIndex];
            game.charIndex++;
            game.typewriterClock.restart();
        }
    }
    else {
        game.stopTypingSound();
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

    std::string textToDraw = game.visibleText;
    if (game.askingName) {
        auto newlinePos = textToDraw.find('\n');
        if (newlinePos != std::string::npos)
            textToDraw = textToDraw.substr(0, newlinePos + 1);
    }

    auto segments = buildColoredSegments(textToDraw);
    float maxWidth = game.textBox.getSize().x - 40.f;
    auto cursorPos = drawColoredSegments(game.window, game.font, segments, basePos, 28, maxWidth);

    bool canPressEnter = false;
    if (game.askingName) {
        canPressEnter = !game.nameInput.empty();
    } else {
        if (game.charIndex < fullText.size()) {
            canPressEnter = true;
        } else if (game.currentDialogue && game.dialogueIndex + 1 < game.currentDialogue->size()) {
            canPressEnter = true;
        }
    }

    if (game.askingName) {
        if (game.cursorBlinkClock.getElapsedTime().asSeconds() >= game.cursorBlinkInterval) {
            game.cursorVisible = !game.cursorVisible;
            game.cursorBlinkClock.restart();
        }

        sf::Vector2f inputPos{ textPos.x + 20.f, textPos.y + 60.f };

        sf::Text inputText{game.font, "", 28};
        inputText.setFillColor(sf::Color::White);
        inputText.setString(game.nameInput);

        std::string nameWithCursor = game.nameInput;
        nameWithCursor.push_back(' ');
        inputText.setString(nameWithCursor);

        inputText.setPosition(inputPos);
        game.window.draw(inputText);

        if (game.cursorVisible) {
            sf::Text cursorText{game.font, "_", 28};
            cursorText.setFillColor(sf::Color::White);
            auto cursorPos = inputText.findCharacterPos(game.nameInput.size());
            cursorText.setPosition(cursorPos);
            game.window.draw(cursorText);
        }
    }

    if (!isTyping) {
        float returnPosWidth = game.textBox.getSize().x + 300.f;
        float returnPosHeight = game.textBox.getSize().y + 450.f;
        game.returnSprite.setPosition({returnPosWidth, returnPosHeight});
        if (game.returnBlinkClock.getElapsedTime().asSeconds() >= game.returnBlinkInterval) {
            game.returnVisible = !game.returnVisible;
            game.returnBlinkClock.restart();
        }
        if (game.returnVisible) {
            sf::Color c = game.returnSprite.getColor();
            c.a = 0;
            game.returnSprite.setColor(c);
            c.a = 255;
            game.returnSprite.setColor(c);
            game.window.draw(game.returnSprite);
        }
    }
}