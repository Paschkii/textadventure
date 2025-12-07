#include "resources.hpp"
#include <iostream>

bool Resources::loadAll()
{
    // ---------------------------
    //       === Fonts ===
    // ---------------------------
    if (!uiFont.openFromFile("assets/fonts/uiFont.ttf")) { // Font: Text Font
        std::cout << "Failed to load uiFont.ttf\n"; return false;
    }
    if (!titleFont.openFromFile("assets/fonts/titleFont.otf")) { // Font: Front Layer
        std::cout << "Failed to load titleFont.otf\n"; return false;
    }
    if (!titleFontExtrude.openFromFile("assets/fonts/titleFontExtrude.otf")) { // Font: Back Layer
        std::cout << "Failed to load titleFontExtrude.otf\n"; return false;
    }
    if (!introFont.openFromFile("assets/fonts/introFont.ttf")) { // Font: Important Message
        std::cout << "Failed to load introFont.ttf\n"; return false;
    }
    // ---------------------------
    //      === Textures ===
    // ---------------------------
    if (!introBackground.loadFromFile("assets/gfx/background.png")) { // Background Texture for Title
        std::cout << "Failed to load Background Texture\n"; return false;
    }

    // ----------------------------
    //      Character Textures
    // ----------------------------
    if (!airDragon.loadFromFile("assets/gfx/characters/gustavo_windimaess.png")) { // Char: Air Dragon » Gustavo Windimaess
        std::cout << "Failed to load Air Dragon Texture\n"; return false;
    }
    if (!earthDragon.loadFromFile("assets/gfx/characters/grounded_claymore.png")) { // Char: Earth Dragon » Grounded Claymore
        std::cout << "Failed to load Earth Dragon Texture\n"; return false;
    }
    if (!fireDragon.loadFromFile("assets/gfx/characters/rowsted_sheacane.png")) { // Char: Fire Dragon » Rowsted Shecane
        std::cout << "Failed to load Fire Dragon Texture\n"; return false;
    }
    if (!waterDragon.loadFromFile("assets/gfx/characters/flawtin_seamen.png")) { // Char: Water Dragon » Flawtin Seamen
        std::cout << "Failed to load Water Dragon Texture\n"; return false;
    }

    // ---------------------------
    //        Map Textures
    // ---------------------------
    if (!locationGonadColored.loadFromFile("assets/gfx/map/gonad_colored.png")) { // Location: Gonad (Colored)
        std::cout << "Failed to load Gonad Texture (Colored)\n"; return false;
    }
    if (!locationGonadSepia.loadFromFile("assets/gfx/map/gonad_sepia.png")) { // Location: Gonad (Sepia)
        std::cout << "Failed to load Gonad Texture (Sepia)\n"; return false;
    }
    if (!locationAerobronchiColored.loadFromFile("assets/gfx/map/aerobronchi_colored.png")) { // Location: Aerobronchi (Colored)
        std::cout << "Failed to load Aerobronchi Texture (Colored)\n"; return false;
    }
    if (!locationAerobronchiSepia.loadFromFile("assets/gfx/map/aerobronchi_sepia.png")) { // Location: Aerobronchi (Sepia)
        std::cout << "Failed to load Aerobronchi Texture (Sepia)\n"; return false;
    }
    if (!locationBlyathyroidColored.loadFromFile("assets/gfx/map/blyathyroid_colored.png")) { // Location: Blyathyroid (Colored)
    std::cout << "Failed to load Blyathyroid Texture (Colored)\n"; return false;
    }
    if (!locationBlyathyroidSepia.loadFromFile("assets/gfx/map/blyathyroid_sepia.png")) { // Location: Blyathyroid (Sepia)
        std::cout << "Failed to load Blyathyroid Texture (Sepia)\n"; return false;
    }
    if (!locationCladrenalColored.loadFromFile("assets/gfx/map/cladrenal_colored.png")) { // Location: Cladrenal (Colored)
        std::cout << "Failed to load Cladrenal Texture (Colored)\n"; return false;
    }
    if (!locationCladrenalSepia.loadFromFile("assets/gfx/map/cladrenal_sepia.png")) { // Location: Cladrenal (Sepia)
        std::cout << "Failed to load Cladrenal Texture (Sepia)\n"; return false;
    }
    if (!locationLacrimereColored.loadFromFile("assets/gfx/map/lacrimere_colored.png")) { // Location: Lacrimere (Colored)
        std::cout << "Failed to load Lacrimere Texture (Colored)\n"; return false;
    }
    if (!locationLacrimereSepia.loadFromFile("assets/gfx/map/lacrimere_sepia.png")) { // Location: Lacrimere (Sepia)
        std::cout << "Failed to load Lacrimere Texture (Sepia)\n"; return false;
    }
    if (!locationMasterBatesHideoutColored.loadFromFile("assets/gfx/map/master_bates_stronghold_colored.png")) { // Location: Stronghold (Colored)
        std::cout << "Failed to load Master Bates Texture (Colored)\n"; return false;
    }
    if (!locationMasterBatesHideoutSepia.loadFromFile("assets/gfx/map/master_bates_stronghold_sepia.png")) { // Location: Stronghold (Sepia)
        std::cout << "Failed to load Master Bates Texture (Sepia)\n"; return false;
    }
    if (!mapBackground.loadFromFile("assets/gfx/map/map_background.png")) { // Simple Map Background
        std::cout << "Failed to load Map Background Texture\n"; return false;
    }
    if (!returnSymbol.loadFromFile("assets/textures/returnSymbol.png")) { // Return Symbol
        std::cout << "Failed to load Return Symbol Texture\n"; return false;
    }
    if (!boxBorder.loadFromFile("assets/textures/boxBorder.png")) { // boxBordet Asset
        std::cout << "Failed to load Box Border Texture\n"; return false;
    }

    // ---------------------------
    //       === Sounds ===
    // ---------------------------
    if (!acquire.loadFromFile("assets/audio/acquire.wav")) { // Acquiring Item
        std::cout << "Failed to load Acquire Sound\n"; return false;
    }
    if (!confirm.loadFromFile("assets/audio/confirm.mp3")) { // Confirm Choice
        std::cout << "Failed to load Confirm Sound\n"; return false;
    }
    if (!enterKey.loadFromFile("assets/audio/enterKey.wav")) { // Pressing Enter Key
        std::cout << "Failed to load Enter Key Sound\n"; return false;
    }
    if (!typewriter.loadFromFile("assets/audio/typewriter.mp3")) { // isTyping Sound
        std::cout << "Failed to load Typewriter Sound\n"; return false;
    }
    if (!reject.loadFromFile("assets/audio/reject.mp3")) { // Reject Choice
        std::cout << "Failed to load Reject Sound\n"; return false;
    }
    if (!startGame.loadFromFile("assets/audio/startGame.wav")) { // Start Game
        std::cout << "Failed to load Start Game Sound\n"; return false;
    }

    return true;
}