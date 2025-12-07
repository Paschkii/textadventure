#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <string>

struct Resources {
    // Fonts
    sf::Font uiFont; // Font for Dialogue Texts
    sf::Font introFont; // Font for the Important Message
    sf::Font titleFont; // FrontLayer for introBackground
    sf::Font titleFontExtrude; // BackLayer for introBackground

    // Textures
    sf::Texture introBackground; // Background
    sf::Texture returnSymbol; // Blinking return Symbol
    sf::Texture boxBorder; // Box Asset
    // Character Textures
    sf::Texture airDragon; // Air Dragon Texture
    sf::Texture earthDragon; // Earth Dragon Texture
    sf::Texture fireDragon; // Fire Dragon Texture
    sf::Texture waterDragon; // Water Dragon Texture
    // Map Textures
    sf::Texture locationGonadColored; // Gonad
    sf::Texture locationGonadSepia; // Gonad
    sf::Texture locationBlyathyroidColored; // Blyathyroid
    sf::Texture locationBlyathyroidSepia; // Blyathyroid
    sf::Texture locationLacrimereColored; // Lacrimere
    sf::Texture locationLacrimereSepia; // Lacrimere
    sf::Texture locationCladrenalColored; // Cladrenal
    sf::Texture locationCladrenalSepia; // Cladrenal
    sf::Texture locationAerobronchiColored; // Aerobronchi
    sf::Texture locationAerobronchiSepia; // Aerobronchi
    sf::Texture locationMasterBatesHideoutColored; // Master Bates Stronghold
    sf::Texture locationMasterBatesHideoutSepia; // Master Bates Stronghold
    sf::Texture mapBackground; // Map Background

    // Sound Buffers
    sf::SoundBuffer acquire; // Sound played, when aquiring items
    sf::SoundBuffer confirm; // Sound played, when confirming choices
    sf::SoundBuffer enterKey; // Sound played when Enter Key is pressed
    sf::SoundBuffer typewriter; // Typewriter Sound » isTyping
    sf::SoundBuffer reject; // Sound played when rejecting choices
    sf::SoundBuffer startGame; // Sound played when starting the Game

    // Central Loader
    bool loadAll();
};