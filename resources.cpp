#include "resources.hpp"
#include <iostream>

bool Resources::loadAll()
{
    // === Fonts ===
    if (!uiFont.openFromFile("assets/fonts/uiFont.ttf")) {
        std::cout << "Failed to load uiFont.ttf\n"; return false;
    }
    if (!titleFont.openFromFile("assets/fonts/titleFont.otf")) {
        std::cout << "Failed to load titleFont.otf\n"; return false;
    }
    if (!titleFontExtrude.openFromFile("assets/fonts/titleFontExtrude.otf")) {
        std::cout << "Failed to load titleFontExtrude.otf\n"; return false;
    }
    if (!introFont.openFromFile("assets/fonts/introFont.ttf")) {
        std::cout << "Failed to load introFont.ttf\n"; return false;
    }

    // === Textures ===
    if (!introBackground.loadFromFile("assets/gfx/background.png")) {
        std::cout << "Failed to load Background Texture\n"; return false;
    }

    // Character Textures
    if (!air_dragon.loadFromFile("assets/gfx/characters/gustavo_windimaess.png")) {
        std::cout << "Failed to load Air Dragon Texture\n"; return false;
    }
    if (!earth_dragon.loadFromFile("assets/gfx/characters/grounded_claymore.png")) {
        std::cout << "Failed to load Earth Dragon Texture\n"; return false;
    }
    if (!fire_dragon.loadFromFile("assets/gfx/characters/rowsted_sheacane.png")) {
        std::cout << "Failed to load Fire Dragon Texture\n"; return false;
    }
    if (!water_dragon.loadFromFile("assets/gfx/characters/flawtin_seamen.png")) {
        std::cout << "Failed to load Water Dragon Texture\n"; return false;
    }

    // Map Textures
    if (!locationGonadColored.loadFromFile("assets/gfx/map/gonad_colored.png")) {
        std::cout << "Failed to load Gonad Texture (Colored)\n"; return false;
    }
    if (!locationGonadSepia.loadFromFile("assets/gfx/map/gonad_sepia.png")) {
        std::cout << "Failed to load Gonad Texture (Sepia)\n"; return false;
    }
    if (!locationAerobronchiColored.loadFromFile("assets/gfx/map/aerobronchi_colored.png")) {
        std::cout << "Failed to load Aerobronchi Texture (Colored)\n"; return false;
    }
    if (!locationAerobronchiSepia.loadFromFile("assets/gfx/map/aerobronchi_sepia.png")) {
        std::cout << "Failed to load Aerobronchi Texture (Sepia)\n"; return false;
    }
    if (!locationBlyathyroidColored.loadFromFile("assets/gfx/map/blyathyroid_colored.png")) {
    std::cout << "Failed to load Blyathyroid Texture (Colored)\n"; return false;
    }
    if (!locationBlyathyroidSepia.loadFromFile("assets/gfx/map/blyathyroid_sepia.png")) {
        std::cout << "Failed to load Blyathyroid Texture (Sepia)\n"; return false;
    }
    if (!locationCladrenalColored.loadFromFile("assets/gfx/map/cladrenal_colored.png")) {
        std::cout << "Failed to load Cladrenal Texture (Colored)\n"; return false;
    }
    if (!locationCladrenalSepia.loadFromFile("assets/gfx/map/cladrenal_sepia.png")) {
        std::cout << "Failed to load Cladrenal Texture (Sepia)\n"; return false;
    }
    if (!locationLacrimereColored.loadFromFile("assets/gfx/map/lacrimere_colored.png")) {
        std::cout << "Failed to load Lacrimere Texture (Colored)\n"; return false;
    }
    if (!locationLacrimereSepia.loadFromFile("assets/gfx/map/lacrimere_sepia.png")) {
        std::cout << "Failed to load Lacrimere Texture (Sepia)\n"; return false;
    }
    if (!locationMasterBatesHideoutColored.loadFromFile("assets/gfx/map/master_bates_stronghold_colored.png")) {
        std::cout << "Failed to load Master Bates Texture (Colored)\n"; return false;
    }
    if (!locationMasterBatesHideoutSepia.loadFromFile("assets/gfx/map/master_bates_stronghold_sepia.png")) {
        std::cout << "Failed to load Master Bates Texture (Sepia)\n"; return false;
    }
    if (!mapBackground.loadFromFile("assets/gfx/map/map_background.png")) {
        std::cout << "Failed to load Map Background Texture\n"; return false;
    }

    // === Sounds ===
    if (!acquire.loadFromFile("assets/sfx/acquire.wav")) {
        std::cout << "Failed to load typewriter\n"; return false;
    }
    if (!confirm.loadFromFile("assets/sfx/confirm.wav")) {
        std::cout << "Failed to load typewriter\n"; return false;
    }
    if (!enterKey.loadFromFile("assets/sfx/enterKey.wav")) {
        std::cout << "Failed to load typewriter\n"; return false;
    }
    if (!typewriter.loadFromFile("assets/sfx/typewriter.mp3")) {
        std::cout << "Failed to load typewriter\n"; return false;
    }

    return true;
}