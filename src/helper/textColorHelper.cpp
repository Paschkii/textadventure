// === C++ Libraries ===
#include <algorithm>  // Uses std::sort when ordering matched tokens.
#include <array>      // Holds the fixed set of speakers to color.
#include <cctype>     // Applies std::isalnum to validate token boundaries.
#include <utility>    // Uses std::pair to associate tokens with colors.
// === Header Files ===
#include "story/textStyles.hpp"    // Provides speaker-specific color/style metadata.
#include "story/storyIntro.hpp"    // Adds extra tokens that should be colored for story text.
#include "helper/colorHelper.hpp"  // References the normal palette for default text segments.
#include "helper/textColorHelper.hpp"  // Declares buildColoredSegments implemented below.

// Scans for speaker/story tokens and returns pre-colored segments for rendering.
std::vector<ColoredTextSegment> buildColoredSegments(const std::string& text) {
    std::vector<ColoredTextSegment> segments;

    if (text.empty())
        return segments;

    struct TokenMatch {
        std::size_t pos;
        std::size_t length;
        sf::Color color;
    };

    const std::array<TextStyles::SpeakerId, 9> speakersToColor{
        TextStyles::SpeakerId::StoryTeller,
        TextStyles::SpeakerId::NoNameNPC,
        TextStyles::SpeakerId::VillageElder,
        TextStyles::SpeakerId::VillageWanderer,
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

    auto storyTokens = StoryIntro::colorTokensWithDynamic();
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

    return segments;
}
