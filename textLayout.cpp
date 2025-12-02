#include "textLayout.hpp"

#include <algorithm>
#include <cctype>
#include <string>

sf::Vector2f drawColoredSegments(
    sf::RenderTarget& target,
    const sf::Font& font,
    const std::vector<ColoredTextSegment>& segments,
    sf::Vector2f startPos,
    unsigned int characterSize,
    float maxWidth,
    float alphaFactor
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
                sf::Color drawableColor = segment.color;
                drawableColor.a = static_cast<std::uint8_t>(static_cast<float>(drawableColor.a) * alphaFactor);
                drawable.setFillColor(drawableColor);

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