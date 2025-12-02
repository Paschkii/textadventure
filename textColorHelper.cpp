#include "textColorHelper.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <utility>
#include "textStyles.hpp"

namespace {
std::size_t longestPartialSpeakerPrefix(
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
} // namespace

std::vector<ColoredTextSegment> buildColoredSegments(const std::string& text) {
    std::vector<ColoredTextSegment> segments;

    if (text.empty())
        return segments;

    const std::array<TextStyles::SpeakerId, 8> speakersToColor{
        TextStyles::SpeakerId::Narrator,
        TextStyles::SpeakerId::StoryTeller,
        TextStyles::SpeakerId::NoName,
        TextStyles::SpeakerId::Player,
        TextStyles::SpeakerId::FireDragon,
        TextStyles::SpeakerId::WaterDragon,
        TextStyles::SpeakerId::AirDragon,
        TextStyles::SpeakerId::EarthDragon
    };

    std::vector<std::pair<std::string, sf::Color>> tokens;
    tokens.reserve(speakersToColor.size());
    for (auto speaker : speakersToColor) {
        TextStyles::SpeakerStyle style = TextStyles::speakerStyle(speaker);
        if (!style.name.empty())
            tokens.emplace_back(style.name, style.color);
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