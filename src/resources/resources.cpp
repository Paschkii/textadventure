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
    //         Item Textures
    // ----------------------------
    if (!dragonstoneAir.loadFromFile("assets/dragonstones/dragonstoneAir.png")) { // Item: Air Dragonstone
        std::cout << "Failed to load Air Dragonstone Texture\n"; return false;
    }
    if (!dragonstoneEarth.loadFromFile("assets/dragonstones/dragonstoneEarth.png")) { // Item: Earth Dragonstone
        std::cout << "Failed to load Earth Dragonstone Texture\n"; return false;
    }
    if (!dragonstoneFire.loadFromFile("assets/dragonstones/dragonstoneFire.png")) { // Item: Fire Dragonstone
        std::cout << "Failed to load Fire Dragonstone Texture\n"; return false;
    }
    if (!dragonstoneWater.loadFromFile("assets/dragonstones/dragonstoneWater.png")) { // Item: Water Dragonstone
        std::cout << "Failed to load Water Dragonstone Texture\n"; return false;
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
    // ----------------------------
    //      Character Portraits
    // ----------------------------
    if (!portraitStoryTeller.loadFromFile("assets/gfx/characters/tory_tailor.png")) { // Portrait: Tory Tailor
        std::cout << "Failed to load Tory Tailor Portrait\n"; return false;
    }
    if (!portraitVillageNPC.loadFromFile("assets/gfx/characters/noah_lott.png")) { // Portrait: Noah Lott
        std::cout << "Failed to load Noah Lott Portrait\n"; return false;
    }
    if (!portraitNoahBates.loadFromFile("assets/gfx/characters/noah_bates.png")) { // Portrait: Noah Bates
        std::cout << "Failed to load Noah Bates Portrait\n"; return false;
    }
    if (!portraitPlayer.loadFromFile("assets/gfx/characters/dragonborn_male.png")) { // Portrait: Player (Dragonborn)
        std::cout << "Failed to load Player Portrait\n"; return false;
    }
    if (!portraitFireDragon.loadFromFile("assets/gfx/characters/rowsted_sheacane_portrait.png")) { // Portrait: Rowsted Sheacane
        std::cout << "Failed to load Fire Dragon Portrait\n"; return false;
    }
    if (!portraitWaterDragon.loadFromFile("assets/gfx/characters/flawtin_seamen_portrait.png")) { // Portrait: Flawtin Seamen
        std::cout << "Failed to load Water Dragon Portrait\n"; return false;
    }
    if (!portraitAirDragon.loadFromFile("assets/gfx/characters/gustavo_windimaess_portrait.png")) { // Portrait: Gustavo Windimaess
        std::cout << "Failed to load Air Dragon Portrait\n"; return false;
    }
    if (!portraitEarthDragon.loadFromFile("assets/gfx/characters/grounded_claymore_portrait.png")) { // Portrait: Grounded Claymore
        std::cout << "Failed to load Earth Dragon Portrait\n"; return false;
    }
    if (!portraitMasterBates.loadFromFile("assets/gfx/characters/master_bates.png")) { // Portrait: Master Bates
        std::cout << "Failed to load Master Bates Portrait\n"; return false;
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
    if (!locationSeminiferousColored.loadFromFile("assets/gfx/map/seminiferous_colored.png")) { // Location: Seminiferous (Colored)
        std::cout << "Failed to load Seminiferous Texture (Colored)\n"; return false;
    }
    if (!locationSeminiferousSepia.loadFromFile("assets/gfx/map/seminiferous_sepia.png")) { // Location: Seminiferous (Sepia)
        std::cout << "Failed to load Seminiferous Texture (Sepia)\n"; return false;
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
    if (!teleportStart.loadFromFile("assets/audio/teleport_start.mp3")) { // Teleport Start
        std::cout << "Failed to load Teleport Start Sound\n"; return false;
    }
    if (!teleportMiddle.loadFromFile("assets/audio/teleport_middle.mp3")) { // Teleport Middle
        std::cout << "Failed to load Teleport Middle Sound\n"; return false;
    }
    if (!teleportStop.loadFromFile("assets/audio/teleport_stop.mp3")) { // Teleport Stop
        std::cout << "Failed to load Teleport Stop Sound\n"; return false;
    }
    if (!locationMusicAerobronchi.loadFromFile("assets/audio/locationMusicAerobronchi.mp3")) { // Aerobronchi Location Sound
        std::cout << "Failed to load Aerobronchi Location Music\n"; return false;
    }
    if (!locationMusicBlyathyroid.loadFromFile("assets/audio/locationMusicBlyathyroid.mp3")) { // Blyathyroid Location Sound
        std::cout << "Failed to load Blyathyroid Location Music\n"; return false;
    }
    if (!locationMusicCladrenal.loadFromFile("assets/audio/locationMusicCladrenal.mp3")) { // Cladrenal Location Sound
        std::cout << "Failed to load Cladrenal Location Music\n"; return false;
    }
    if (!locationMusicGonad.loadFromFile("assets/audio/locationMusicGonad.mp3")) { // Gonad Location Sound
        std::cout << "Failed to load Gonad Location Music\n"; return false;
    }
    if (!locationMusicLacrimere.loadFromFile("assets/audio/locationMusicLacrimere.mp3")) { // Lacrimere Location Sound
        std::cout << "Failed to load Lacrimere Location Music\n"; return false;
    }
    if (!locationMusicSeminiferous.loadFromFile("assets/audio/locationMusicSeminiferous.mp3")) { // Seminiferous Location Sound
        std::cout << "Failed to load Seminiferous Location Music\n"; return false;
    }
    if (!titleScreen.loadFromFile("assets/audio/titleScreen.mp3")) { // Title Screen Sound
        std::cout << "Failed to load Title Screen Music\n"; return false;
    }
    if (!quizAnswerCorrect.loadFromFile("assets/audio/quizAnswerCorrect.mp3")) { // Answer Correct
        std::cout << "Failed to load Answer Correct Sound\n"; return false;
    }
    if (!quizAnswerIncorrect.loadFromFile("assets/audio/quizAnswerIncorrect.mp3")) { // Answer Incorrect
        std::cout << "Failed to load Answer Incorrect Sound\n"; return false;
    }
    if (!quizLoggingAnswer.loadFromFile("assets/audio/quizLoggingAnswer.mp3")) { // Logging Answer
        std::cout << "Failed to load Logging Answer Sound\n"; return false;
    }
    if (!quizJokerAskAudience.loadFromFile("assets/audio/quizJokerAskAudience.mp3")) { // Ask Audience
        std::cout << "Failed to load Ask Audience Sound\n"; return false;
    }
    if (!quizJokerCallFriend.loadFromFile("assets/audio/quizJokerCallFriend.mp3")) { // Call Friend
        std::cout << "Failed to load Call Friend Sound\n"; return false;
    }
    if (!quizJokerFiftyFifty.loadFromFile("assets/audio/quizJokerFiftyFifty.mp3")) { // Fifty Fifty
        std::cout << "Failed to load Fifty Fifty Sound\n"; return false;
    }
    if (!quizQuestionStart.loadFromFile("assets/audio/quizQuestionStart.mp3")) { // Question Start
        std::cout << "Failed to load Question Start Sound\n"; return false;
    }
    if (!quizQuestionThinking.loadFromFile("assets/audio/quizQuestionThinking.mp3")) { // Question Thinking
        std::cout << "Failed to load Question Thinking Sound\n"; return false;
    }
    if (!quizStart.loadFromFile("assets/audio/quizStart.mp3")) { // Quiz Start
        std::cout << "Failed to load Quiz Start Sound\n"; return false;
    }
    if (!quizEnd.loadFromFile("assets/audio/quizEnd.mp3")) { // Quiz End
        std::cout << "Failed to load Sound\n"; return false;
    }


    return true;
}
