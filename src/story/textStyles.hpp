#pragma once
// === C++ Libraries ===
#include <string>                // Used for speaker names and runtime player name.
// === SFML Libraries ===
#include <SFML/Graphics.hpp>     // Pushes speaker colors onto sf::Text and other drawables.
// === Header Files ===
#include "helper/colorHelper.hpp" // Supplies the shared palette for speaker/UI colors.

namespace TextStyles {

// Identifiers for every speaker whose dialogue can be styled.
enum class SpeakerId {
    Unknown,
    Player,
    DragonbornSibling,
    StoryTeller,
    NoNameNPC,
    NoNameWanderer,
    VillageElder,
    VillageWanderer,
    Blacksmith,
    FireDragon,
    WaterDragon,
    AirDragon,
    EarthDragon,
    MasterBates,
    NoahBates
};

// Holds the display name and color used for a speaker.
struct SpeakerStyle {
    std::string name;
    sf::Color color;
};

namespace Palette = ColorHelper::Palette;

// UI-specific colors derived from the shared palette.
namespace UI {
    inline const sf::Color Background = Palette::BlueDark;
    inline const sf::Color Border = Palette::FrameGoldDark;
    inline const sf::Color Text = Palette::Normal;
    inline const sf::Color Highlight = Palette::SoftYellow;
    inline const sf::Color Panel = Palette::Blue;
    inline const sf::Color PanelDark = Palette::BlueDark;
    inline const sf::Color PanelLight = Palette::BlueLight;
}

// Helpers used to resolve speaker metadata.
SpeakerId speakerFromName(const std::string& name);
SpeakerStyle speakerStyle(SpeakerId speaker);
void applySpeakerStyle(sf::Text& text, SpeakerId speaker);

} // namespace TextStyles

// Aktueller Spielername (wird während des Spiels gesetzt)
// Updated at runtime to hold the player’s chosen name.
inline std::string playerDisplayName;
inline std::string dragonbornSiblingDisplayName;
