#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "helper/colorHelper.hpp"

namespace TextStyles {

enum class SpeakerId {
    Unknown,
    Player,
    StoryTeller,
    NoNameNPC,
    VillageNPC,
    FireDragon,
    WaterDragon,
    AirDragon,
    EarthDragon,
    MasterBates,
    NoahBates
};

struct SpeakerStyle {
    std::string name;
    sf::Color color;
};

namespace Palette = ColorHelper::Palette;

namespace UI {
    inline const sf::Color Background = Palette::BlueDark;
    inline const sf::Color Border = Palette::FrameGoldDark;
    inline const sf::Color Text = Palette::Normal;
    inline const sf::Color Highlight = Palette::SoftYellow;
    inline const sf::Color Panel = Palette::Blue;
    inline const sf::Color PanelDark = Palette::BlueDark;
    inline const sf::Color PanelLight = Palette::BlueLight;
}

SpeakerId speakerFromName(const std::string& name);
SpeakerStyle speakerStyle(SpeakerId speaker);
void applySpeakerStyle(sf::Text& text, SpeakerId speaker);

} // namespace TextStyles

// Aktueller Spielername (wird während des Spiels gesetzt)
inline std::string playerDisplayName;
