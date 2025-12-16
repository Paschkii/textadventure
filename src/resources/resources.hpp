#pragma once
// === C++ Libraries ===
#include <map>      // Stores lookup tables for textures or sounds if needed in future helpers.
#include <string>   // Holds asset path strings when loading resources (used by loadAll implementation).
// === SFML Libraries ===
#include <SFML/Graphics.hpp>  // Declares sf::Texture and sf::Font types that Resources stores.
#include <SFML/Audio.hpp>     // Declares sf::SoundBuffer objects held in the resource bundle.

// Bundles every shared texture/font/sound buffer used by the game.
struct Resources {
    // Fonts
    sf::Font uiFont; // Font for Dialogue Texts
    sf::Font quizFont; // Font used specifically for the quiz UI
    sf::Font introFont; // Font for the Important Message
    sf::Font titleFont; // FrontLayer for introBackground
    sf::Font titleFontExtrude; // BackLayer for introBackground

    // Textures
    sf::Texture introBackground; // Background
    sf::Texture returnSymbol; // Blinking return Symbol
    sf::Texture boxBorder; // Box Asset
    sf::Texture dividerLeft; // Divider asset used in location header
    sf::Texture dividerRight; // Divider asset used in location header
    sf::Texture menuButton; // Menu button graphic used for the overlay toggle
    sf::Texture buttonInventory;
    sf::Texture buttonCharacter;
    sf::Texture buttonMap;
    sf::Texture buttonQuests;
    sf::Texture buttonSettings;
    sf::Texture buttonRankings;
    // Item Textures
    sf::Texture dragonstoneAir; // Air Dragon Stone
    sf::Texture dragonstoneEarth; // Earth Dragon Stone
    sf::Texture dragonstoneFire; // Fire Dragon Stone
    sf::Texture dragonstoneWater; // Water Dragon Stone
    // Weapon Textures
    sf::Texture weaponHolmabir;
    sf::Texture weaponHolmabirBroken;
    sf::Texture weaponKattkavar;
    sf::Texture weaponKattkavarBroken;
    sf::Texture weaponStiggedin;
    sf::Texture weaponStiggedinBroken;
    // Character Textures
    sf::Texture airDragon; // Air Dragon Texture
    sf::Texture earthDragon; // Earth Dragon Texture
    sf::Texture fireDragon; // Fire Dragon Texture
    sf::Texture waterDragon; // Water Dragon Texture
    sf::Texture dragonbornMaleSprite; // Dragonborn Male Sprite for selection
    sf::Texture dragonbornFemaleSprite; // Dragonborn Female Sprite for selection
    sf::Texture dragonbornMaleHoveredSprite; // Hovered Dragonborn Male Sprite
    sf::Texture dragonbornFemaleHoveredSprite; // Hovered Dragonborn Female Sprite
    // Character Portraits
    sf::Texture portraitStoryTeller; // Tory Tailor Portrait
    sf::Texture portraitVillageElder; // Noah Lott Portrait
    sf::Texture portraitVillageWanderer; // Wanda Rinn Portrait
    sf::Texture portraitNoahBates; // Noah bates Portrait
    sf::Texture portraitFireDragon; // Rowsted Sheacane Portrait
    sf::Texture portraitWaterDragon; // Flawtin Seamen Portrait
    sf::Texture portraitAirDragon; // Gustavo Windimaess Portrait
    sf::Texture portraitEarthDragon; // Grounded Claymore Portrait
    sf::Texture portraitMasterBates; // Master Bates Portrait
    sf::Texture portraitDragonbornMale; // Dragonborn Male Portrait
    sf::Texture portraitDragonbornFemale; // Dragonborn Female Portrait
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
    sf::Texture locationSeminiferousColored; // Seminiferous
    sf::Texture locationSeminiferousSepia; // Seminiferous
    sf::Texture mapBackground; // Map Background

    // Sound Buffers
    sf::SoundBuffer acquire; // Sound played, when aquiring items
    sf::SoundBuffer confirm; // Sound played, when confirming choices
    sf::SoundBuffer enterKey; // Sound played when Enter Key is pressed
    sf::SoundBuffer typewriter; // Typewriter Sound » isTyping
    sf::SoundBuffer reject; // Sound played when rejecting choices
    sf::SoundBuffer startGame; // Sound played when starting the Game
    sf::SoundBuffer teleportStart; // Sound played at the beginning of a teleport
    sf::SoundBuffer teleportMiddle; // Sound played during teleportation
    sf::SoundBuffer teleportStop; // Sound played when teleportation ends
    sf::SoundBuffer locationMusicGonad; // Sound played while in Gonad
    sf::SoundBuffer locationMusicBlyathyroid; // Sound played while in Blyathyroid
    sf::SoundBuffer locationMusicLacrimere; // Sound played while in Lacrimere
    sf::SoundBuffer locationMusicCladrenal; // Sound played while in Cladrenal
    sf::SoundBuffer locationMusicAerobronchi; // Sound played while in Aerobronchi
    sf::SoundBuffer locationMusicSeminiferous; // Sound played while in Seminiferous
    sf::SoundBuffer introTitle; // Sound played while on the intro title
    sf::SoundBuffer introDialogue; // Sound played during the intro dialogue
    sf::SoundBuffer buttonHovered; // Sound played when hovering buttons
    sf::SoundBuffer quizAnswerCorrect;
    sf::SoundBuffer quizAnswerIncorrect;
    sf::SoundBuffer quizLoggingAnswer;
    sf::SoundBuffer quizJokerAskAudience;
    sf::SoundBuffer quizJokerCallFriend;
    sf::SoundBuffer quizJokerFiftyFifty;
    sf::SoundBuffer quizQuestionStart;
    sf::SoundBuffer quizQuestionThinking;
    sf::SoundBuffer quizStart;
    sf::SoundBuffer quizEnd;
    sf::SoundBuffer titleButtons; // Sound played when hovering intro title options

    // Central Loader
    // Loads all assets from disk and returns true on success.
    bool loadAll();
};
