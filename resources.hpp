#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <string>

struct Resources {
    // Fonts
    sf::Font uiFont;
    sf::Font introFont;
    sf::Font titleFont;
    sf::Font titleFontExtrude;

    // Textures
    sf::Texture introBackground; // Background
    // Character Textures
    sf::Texture air_dragon;
    sf::Texture earth_dragon;
    sf::Texture fire_dragon;
    sf::Texture water_dragon;
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
    sf::Texture mapBackground;

    // Sound Buffers
    sf::SoundBuffer typewriter;
    sf::SoundBuffer enterKey;
    sf::SoundBuffer acquire;
    sf::SoundBuffer confirm;

    // Zentraler Loader
    bool loadAll();
};