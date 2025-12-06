#include "textStyles.hpp"
#include <algorithm>
#include <cctype>

namespace TextStyles {

    namespace {
        const sf::Color purpleblue{180, 180, 255};
        const sf::Color brown{139, 69, 19};

        bool equalsIgnoreCase(const std::string& a, const std::string& b) {
            return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char lhs, char rhs) {
                return std::tolower(static_cast<unsigned char>(lhs)) == std::tolower(static_cast<unsigned char>(rhs));
            });
        }
    }

    SpeakerId speakerFromName(const std::string& name) {
        if (name.empty())
            return SpeakerId::Unknown;

        if (!playerDisplayName.empty() && equalsIgnoreCase(name, playerDisplayName))
            return SpeakerId::Player;

        if (equalsIgnoreCase(name, "Tory Tailor"))
            return SpeakerId::StoryTeller;
        if (equalsIgnoreCase(name, "?????"))
            return SpeakerId::NoNameNPC;
        if (equalsIgnoreCase(name, "Noah Lott"))
            return SpeakerId::VillageNPC;
        if (equalsIgnoreCase(name, "Rowsted Sheacane"))
            return SpeakerId::FireDragon;
        if (equalsIgnoreCase(name, "Flawtin Seamen"))
            return SpeakerId::WaterDragon;
        if (equalsIgnoreCase(name, "Gustavo Windimaess"))
            return SpeakerId::AirDragon;
        if (equalsIgnoreCase(name, "Grounded Claymore"))
            return SpeakerId::EarthDragon;

        return SpeakerId::Unknown;
    }

    SpeakerStyle speakerStyle(SpeakerId speaker) {
        switch (speaker) {
            case SpeakerId::StoryTeller:
                return { "Tory Tailor", purpleblue };
            case SpeakerId::NoNameNPC:
                return { "?????", purpleblue };
            case SpeakerId::VillageNPC:
                return { "Noah Lott", purpleblue };
            case SpeakerId::Player:
                return { playerDisplayName, sf::Color::Cyan };
            case SpeakerId::FireDragon:
                return { "Rowsted Sheacane", sf::Color::Red };
            case SpeakerId::WaterDragon:
                return { "Flawtin Seamen", sf::Color::Blue };
            case SpeakerId::AirDragon:
                return { "Gustavo Windimaess", sf::Color::Green };
            case SpeakerId::EarthDragon:
                return { "Grounded Claymore", brown };
            default:
                return { "", sf::Color::White };
        }
    }

    void applySpeakerStyle(sf::Text& text, SpeakerId speaker) {
        SpeakerStyle style = speakerStyle(speaker);
        text.setFillColor(style.color);
        text.setString(style.name);
    }

} // namespace TextStyles