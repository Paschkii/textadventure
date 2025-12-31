// === C++ Libraries ===
#include <algorithm>  // Provides std::equal used in case-insensitive name matching.
#include <cctype>     // Used for std::tolower calls in speaker matching helpers.
// === Header Files ===
#include "textStyles.hpp" // Declares the public speaker utilities defined here.

namespace TextStyles {

    namespace {
        // Case-insensitive comparison used when matching speaker names.
        bool equalsIgnoreCase(const std::string& a, const std::string& b) {
            return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char lhs, char rhs) {
                return std::tolower(static_cast<unsigned char>(lhs)) == std::tolower(static_cast<unsigned char>(rhs));
            });
        }
    }

    // Resolves a string back into the corresponding speaker enum.
    SpeakerId speakerFromName(const std::string& name) {
        if (name.empty())
            return SpeakerId::Unknown;

        if (!playerDisplayName.empty() && equalsIgnoreCase(name, playerDisplayName))
            return SpeakerId::Player;
        if (!dragonbornSiblingDisplayName.empty() && equalsIgnoreCase(name, dragonbornSiblingDisplayName))
            return SpeakerId::DragonbornSibling;

        if (equalsIgnoreCase(name, "Tory Tailor"))
            return SpeakerId::StoryTeller;
        if (equalsIgnoreCase(name, "?????"))
            return SpeakerId::NoNameNPC;
        if (equalsIgnoreCase(name, "??????"))
            return SpeakerId::NoNameWanderer;
        if (equalsIgnoreCase(name, "Noah Lott"))
            return SpeakerId::VillageElder;
        if (equalsIgnoreCase(name, "Wanda Rinn"))
            return SpeakerId::VillageWanderer;
        if (equalsIgnoreCase(name, "Will Figsid"))
            return SpeakerId::Blacksmith;
        if (equalsIgnoreCase(name, "Master Bates"))
            return SpeakerId::MasterBates;
        if (equalsIgnoreCase(name, "Noah Bates"))
            return SpeakerId::NoahBates;
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

    // Returns the display name and color for a speaker identifier.
    SpeakerStyle speakerStyle(SpeakerId speaker) {
        switch (speaker) {
                case SpeakerId::StoryTeller:
                    return { "Tory Tailor", ColorHelper::Palette::PurpleBlue };
                case SpeakerId::NoNameNPC:
                    return { "?????", ColorHelper::Palette::PurpleBlue };
                case SpeakerId::NoNameWanderer:
                    return { "??????", ColorHelper::Palette::PurpleBlue };
                case SpeakerId::VillageElder:
                    return { "Noah Lott", ColorHelper::Palette::PurpleBlue };
                case SpeakerId::VillageWanderer:
                    return { "Wanda Rinn", ColorHelper::Palette::PurpleBlue };
                case SpeakerId::Blacksmith:
                    return { "Will Figsid", ColorHelper::Palette::PurpleBlue };
                case SpeakerId::MasterBates:
                    return { "Master Bates", ColorHelper::Palette::DarkPurple };
                case SpeakerId::NoahBates:
                    return { "Noah Bates", ColorHelper::Palette::PurpleBlue };
                case SpeakerId::Player:
                    return { playerDisplayName, ColorHelper::Palette::PlayerColor };
                case SpeakerId::DragonbornSibling:
                    return { dragonbornSiblingDisplayName, ColorHelper::Palette::SoftRed };
                case SpeakerId::FireDragon:
                    return { "Rowsted Sheacane", ColorHelper::Palette::FireDragon };
                case SpeakerId::WaterDragon:
                    return { "Flawtin Seamen", ColorHelper::Palette::WaterDragon };
                case SpeakerId::AirDragon:
                    return { "Gustavo Windimaess", ColorHelper::Palette::AirDragon };
                case SpeakerId::EarthDragon:
                    return { "Grounded Claymore", ColorHelper::Palette::EarthDragon };
            case SpeakerId::Unknown:
            default:
                return { "", ColorHelper::Palette::Normal };
        }
    }

    // Applies the speaker's display name/color to the provided text object.
    void applySpeakerStyle(sf::Text& text, SpeakerId speaker) {
        SpeakerStyle style = speakerStyle(speaker);
        text.setFillColor(style.color);
        text.setString(style.name);
    }

} // namespace TextStyles
