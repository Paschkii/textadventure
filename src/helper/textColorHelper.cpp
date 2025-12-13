#include "helper/textColorHelper.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <utility>
#include "story/textStyles.hpp"
#include "story/storyIntro.hpp"
#include "helper/colorHelper.hpp"

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

    struct TokenMatch {
        std::size_t pos;
        std::size_t length;
        sf::Color color;
    };

    const std::array<TextStyles::SpeakerId, 8> speakersToColor{
        TextStyles::SpeakerId::StoryTeller,
        TextStyles::SpeakerId::NoNameNPC,
        TextStyles::SpeakerId::VillageNPC,
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
    const std::size_t speakerTokenCount = tokens.size();

    const auto& storyTokens = StoryIntro::colorTokens();
    tokens.insert(tokens.end(), storyTokens.begin(), storyTokens.end());

    std::vector<TokenMatch> matches;
    matches.reserve(tokens.size());
    for (const auto& token : tokens) {
        std::size_t searchPos = 0;
        while ((searchPos = text.find(token.first, searchPos)) != std::string::npos) {
            bool leftOk = (searchPos == 0) || !std::isalnum(static_cast<unsigned char>(text[searchPos - 1]));
            std::size_t after = searchPos + token.first.size();
            bool rightOk = (after >= text.size()) || !std::isalnum(static_cast<unsigned char>(text[after]));

            if (leftOk && rightOk)
                matches.push_back({ searchPos, token.first.size(), token.second });

            searchPos += token.first.size();
        }
    }

    std::sort(matches.begin(), matches.end(), [](const TokenMatch& a, const TokenMatch& b) {
        if (a.pos == b.pos)
            return a.length > b.length; // prefer longer tokens like "Dragon Scales" over shorter overlaps
        return a.pos < b.pos;
    });

    std::size_t cursor = 0;
    for (const auto& match : matches) {
        if (match.pos < cursor)
            continue;

        if (match.pos > cursor)
            segments.push_back({ text.substr(cursor, match.pos - cursor), ColorHelper::Palette::Normal });

        segments.push_back({ text.substr(match.pos, match.length), match.color });
        cursor = match.pos + match.length;
    }

    if (cursor < text.size())
        segments.push_back({ text.substr(cursor), ColorHelper::Palette::Normal });

    if (segments.empty())
        segments.push_back({ text, ColorHelper::Palette::Normal });

    // Only allow partial matches for speaker names so generic words like
    // "Dragon" do not inherit colors from semantic tokens.
    std::vector<std::pair<std::string, sf::Color>> partialTokens(
        tokens.begin(),
        tokens.begin() + speakerTokenCount
    );

    sf::Color partialColor = ColorHelper::Palette::Normal;
    std::size_t partialLength = longestPartialSpeakerPrefix(text, partialTokens, partialColor);
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
