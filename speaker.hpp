#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "dialogueLine.hpp"

inline std::string playerDisplayName;

// Speaker Class
enum class Speaker { VillageNPC, StoryTeller, NoNameNPC, Player, FireDragon, WaterDragon, AirDragon, EarthDragon };

// Farben, die nicht per Standard in sf::Color:: vorhanden sind
inline const sf::Color purpleblue{ 180, 180, 255 };
inline const sf::Color brown{ 139, 69, 19 };

// inline std::string characterColoredNames(Speaker s) {
//     switch (s) {
//         case Speaker::StoryTeller:  return "<purple>Tory Tailor</>";
//         case Speaker::NoNameNPC:    return "<purple>?????</>";
//         case Speaker::AirDragon:    return "<green>Gustavo Windimaess</>";
//         case Speaker::EarthDragon:  return "<brown>Grounded Claymore</>";
//         case Speaker::FireDragon:   return "<red>Rowsted Sheacane</>";
//         case Speaker::WaterDragon:  return "<blue>Flawtin Seamen</blue>";
//         case Speaker::Player:       return "<cyan>{player}</>";
//         default:                    return "";
//     }
// }

// Funktion, um Speakern Namen zuzuweisen
inline std::string speakerToName(Speaker s) {
    switch (s) {
        case Speaker::StoryTeller:  return "Tory Tailor";
        case Speaker::NoNameNPC:    return "?????";
        case Speaker::AirDragon:    return "Gustavo Windimaess";
        case Speaker::EarthDragon:  return "Grounded Claymore";
        case Speaker::FireDragon:   return "Rowsted Sheacane";
        case Speaker::WaterDragon:  return "Flawtin Seamen";
        case Speaker::Player:       return playerDisplayName;
        case Speaker::VillageNPC:   return "Noah Lott";
        default:                    return "";
    }
}

// Funktion um Namen Farbe zuzuweisen
inline sf::Color colorForSpeaker(Speaker s) {
    switch(s) {
        case Speaker::StoryTeller:  return purpleblue;
        case Speaker::NoNameNPC:    return purpleblue;
        case Speaker::Player:       return sf::Color::Cyan;
        case Speaker::FireDragon:   return sf::Color::Red;
        case Speaker::WaterDragon:  return sf::Color::Blue;
        case Speaker::AirDragon:    return sf::Color::Green;
        case Speaker::EarthDragon:  return brown;
        case Speaker::VillageNPC:   return purpleblue;
        default:                    return sf::Color::White;
    }
}

// Funktion um Namen und Farben in Game direkt zu fetchen
struct Speakerinfo {
    std::string name;
    sf::Color color;
};

inline Speakerinfo getSpeakerInfo(Speaker s) {
    return { speakerToName(s), colorForSpeaker(s)};
}

inline void styleNameTextWithSpeaker (const Speakerinfo& info, sf::Text& text) {
    text.setFillColor(info.color);
    text.setString(info.name);
}