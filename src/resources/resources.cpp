// === C++ Libraries ===
#include <iostream>  // Logs asset-loading failures during Resources::loadAll.
// === Header Files ===
#include "resources.hpp"  // Declares Resources::loadAll and the stored asset handles.
#include "resources/itemFiles.hpp"

// Attempts to load every font/texture/audio asset required for the game.
bool Resources::loadAll()
{
    // ---------------------------
    //       === Fonts ===
    // ---------------------------
    if (!uiFont.openFromFile("assets/fonts/uiFont.ttf")) { // Font: Text Font
        std::cout << "Failed to load uiFont.ttf\n"; return false;
    }
    if (!quizFont.openFromFile("assets/fonts/quizFont.ttf")) { // Font: Quiz Text
        std::cout << "Failed to load quizFont.ttf\n"; return false;
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
    if (!weaponHolmabir.loadFromFile("assets/gfx/weapons/Weapon Holmabir.png")) { // Weapon: Holmabir
        std::cout << "Failed to load Holmabir Texture\n"; return false;
    }
    if (!weaponHolmabirBroken.loadFromFile("assets/gfx/weapons/Weapon Holmabir Broken.png")) { // Weapon: Holmabir Broken
        std::cout << "Failed to load Holmabir Broken Texture\n"; return false;
    }
    if (!weaponKattkavar.loadFromFile("assets/gfx/weapons/Weapon Kattkavar.png")) { // Weapon: Kattkavar
        std::cout << "Failed to load Kattkavar Texture\n"; return false;
    }
    if (!weaponKattkavarBroken.loadFromFile("assets/gfx/weapons/Weapon Kattkavar Broken.png")) { // Weapon: Kattkavar Broken
        std::cout << "Failed to load Kattkavar Broken Texture\n"; return false;
    }
    if (!weaponStiggedin.loadFromFile("assets/gfx/weapons/Weapon Stiggedin.png")) { // Weapon: Stiggedin
        std::cout << "Failed to load Stiggedin Texture\n"; return false;
    }
    if (!weaponStiggedinBroken.loadFromFile("assets/gfx/weapons/Weapon Stiggedin Broken.png")) { // Weapon: Stiggedin Broken
        std::cout << "Failed to load Stiggedin Broken Texture\n"; return false;
    }
    if (!healPotion.loadFromFile(ResourceFiles::Items::kHealPotionTexture)) { // Item: Healing Potion
        std::cout << "Failed to load Healing Potion Texture\n"; return false;
    }
    if (!mapGlandular.loadFromFile("assets/gfx/items/map_glandular.png")) { // Item: Map of Glandular
        std::cout << "Failed to load Map of Glandular Texture\n"; return false;
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
    if (!dragonbornMaleSprite.loadFromFile("assets/gfx/characters/dragonborn/Dragonborn_Male.png")) { // Selectable Dragonborn Male
        std::cout << "Failed to load Dragonborn Male Sprite\n"; return false;
    }
    if (!dragonbornFemaleSprite.loadFromFile("assets/gfx/characters/dragonborn/Dragonborn_Female.png")) { // Selectable Dragonborn Female
        std::cout << "Failed to load Dragonborn Female Sprite\n"; return false;
    }
    if (!dragonbornMaleHoveredSprite.loadFromFile("assets/gfx/characters/dragonborn/Dragonborn_Male_Hovered.png")) { // Hovered Dragonborn Male
        std::cout << "Failed to load Dragonborn Male Hover Sprite\n"; return false;
    }
    if (!dragonbornFemaleHoveredSprite.loadFromFile("assets/gfx/characters/dragonborn/Dragonborn_Female_Hovered.png")) { // Hovered Dragonborn Female
        std::cout << "Failed to load Dragonborn Female Hover Sprite\n"; return false;
    }
    // ----------------------------
    //      Character Portraits
    // ----------------------------
    if (!portraitStoryTeller.loadFromFile("assets/gfx/characters/tory_tailor.png")) { // Portrait: Tory Tailor
        std::cout << "Failed to load Tory Tailor Portrait\n"; return false;
    }
    if (!portraitVillageElder.loadFromFile("assets/gfx/characters/noah_lott.png")) { // Portrait: Noah Lott
        std::cout << "Failed to load Noah Lott Portrait\n"; return false;
    }
    if (!portraitVillageWanderer.loadFromFile("assets/gfx/characters/wanda_rinn.png")) { // Portrait: Wanda Rinn
        std::cout << "Failed to load Village Wanderer Portrait\n"; return false;
    }
    if (!portraitBlacksmith.loadFromFile("assets/gfx/characters/will_figsid.png")) { // Portrait: Wanda Rinn
        std::cout << "Failed to load Blacksmith Portrait\n"; return false;
    }
    if (!portraitNoahBates.loadFromFile("assets/gfx/characters/noah_bates.png")) { // Portrait: Noah Bates
        std::cout << "Failed to load Noah Bates Portrait\n"; return false;
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
    if (!portraitDragonbornMale.loadFromFile("assets/gfx/characters/dragonborn/Portrait_Dragonborn_Male.png")) { // Portrait: Dragonborn Male
        std::cout << "Failed to load Dragonborn Male Portrait\n"; return false;
    }
    if (!portraitDragonbornFemale.loadFromFile("assets/gfx/characters/dragonborn/Portrait_Dragonborn_Female.png")) { // Portrait: Dragonborn Female
        std::cout << "Failed to load Dragonborn Female Portrait\n"; return false;
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
    if (!menuMapBackground.loadFromFile("assets/gfx/menu/map/Glandular.png")) {
        std::cout << "Failed to load Menu Map Background (Glandular)\n"; return false;
    }
    if (!menuMapGonad.loadFromFile("assets/gfx/menu/map/Gonad.png")) {
        std::cout << "Failed to load Menu Map Overlay (Gonad)\n"; return false;
    }
    if (!menuMapLacrimere.loadFromFile("assets/gfx/menu/map/Lacrimere.png")) {
        std::cout << "Failed to load Menu Map Overlay (Lacrimere)\n"; return false;
    }
    if (!menuMapBlyathyroid.loadFromFile("assets/gfx/menu/map/Blyathyroid.png")) {
        std::cout << "Failed to load Menu Map Overlay (Blyathyroid)\n"; return false;
    }
    if (!menuMapCladrenal.loadFromFile("assets/gfx/menu/map/Cladrenal.png")) {
        std::cout << "Failed to load Menu Map Overlay (Cladrenal)\n"; return false;
    }
    if (!menuMapAerobronchi.loadFromFile("assets/gfx/menu/map/Aerobronchi.png")) {
        std::cout << "Failed to load Menu Map Overlay (Aerobronchi)\n"; return false;
    }
    if (!menuMapPetrigonal.loadFromFile("assets/gfx/menu/map/Perigonal.png")) {
        std::cout << "Failed to load Menu Map Overlay (Petrigonal)\n"; return false;
    }
    if (!backgroundAerobronchi.loadFromFile("assets/gfx/background/background_aerobronchi.png")) {
        std::cout << "Failed to load Background Aerobronchi\n"; return false;
    }
    if (!backgroundBlacksmith.loadFromFile("assets/gfx/background/background_blacksmith.png")) {
        std::cout << "Failed to load Background Blacksmith\n"; return false;
    }
    if (!backgroundBlyathyroid.loadFromFile("assets/gfx/background/background_blyathyoid.png")) {
        std::cout << "Failed to load Background Blyathyroid\n"; return false;
    }
    if (!backgroundCladrenal.loadFromFile("assets/gfx/background/background_cladrenal.png")) {
        std::cout << "Failed to load Background Cladrenal\n"; return false;
    }
    if (!backgroundGonad.loadFromFile("assets/gfx/background/background_gonad.png")) {
        std::cout << "Failed to load Background Gonad\n"; return false;
    }
    if (!backgroundLacrimere.loadFromFile("assets/gfx/background/background_lacrimere.png")) {
        std::cout << "Failed to load Background Lacrimere\n"; return false;
    }
    if (!backgroundPetrigonal.loadFromFile("assets/gfx/background/background_petrigonal.png")) {
        std::cout << "Failed to load Background Petrigonal\n"; return false;
    }
    if (!backgroundSeminiferous.loadFromFile("assets/gfx/background/background_seminiferous.png")) {
        std::cout << "Failed to load Background Seminiferous\n"; return false;
    }
    if (!backgroundToryTailor.loadFromFile("assets/gfx/background/background_torytailor.png")) {
        std::cout << "Failed to load Background ToryTailor\n"; return false;
    }
    if (!bookshelf.loadFromFile("assets/gfx/books/bookshelf.png")) { // Bookshelf Background
        std::cout << "Failed to load Bookshelf Texture\n"; return false;
    }
    if (!bookSingle01.loadFromFile("assets/gfx/books/book_single01.png")) {
        std::cout << "Failed to load book_single01.png\n"; return false;
    }
    if (!bookSingle02.loadFromFile("assets/gfx/books/book_single02.png")) {
        std::cout << "Failed to load book_single02.png\n"; return false;
    }
    if (!bookSingle03.loadFromFile("assets/gfx/books/book_single03.png")) {
        std::cout << "Failed to load book_single03.png\n"; return false;
    }
    if (!bookSingle04.loadFromFile("assets/gfx/books/book_single04.png")) {
        std::cout << "Failed to load book_single04.png\n"; return false;
    }
    if (!bookSingle05.loadFromFile("assets/gfx/books/book_single05.png")) {
        std::cout << "Failed to load book_single05.png\n"; return false;
    }
    if (!bookStapled01.loadFromFile("assets/gfx/books/book_stapled01.png")) {
        std::cout << "Failed to load book_stapled01.png\n"; return false;
    }
    if (!bookStapled02.loadFromFile("assets/gfx/books/book_stapled02.png")) {
        std::cout << "Failed to load book_stapled02.png\n"; return false;
    }
    if (!booksRowed01.loadFromFile("assets/gfx/books/books_rowed01.png")) {
        std::cout << "Failed to load books_rowed01.png\n"; return false;
    }
    if (!booksRowed02.loadFromFile("assets/gfx/books/books_rowed02.png")) {
        std::cout << "Failed to load books_rowed02.png\n"; return false;
    }
    if (!booksRowed03.loadFromFile("assets/gfx/books/books_rowed03.png")) {
        std::cout << "Failed to load books_rowed03.png\n"; return false;
    }
    if (!booksTipped01.loadFromFile("assets/gfx/books/books_tipped01.png")) {
        std::cout << "Failed to load books_tipped01.png\n"; return false;
    }
    if (!booksTipped02.loadFromFile("assets/gfx/books/books_tipped02.png")) {
        std::cout << "Failed to load books_tipped02.png\n"; return false;
    }
    if (!returnSymbol.loadFromFile("assets/textures/returnSymbol.png")) { // Return Symbol
        std::cout << "Failed to load Return Symbol Texture\n"; return false;
    }
    if (!boxBorder.loadFromFile("assets/textures/boxBorder.png")) { // boxBordet Asset
        std::cout << "Failed to load Box Border Texture\n"; return false;
    }
    if (!dividerLeft.loadFromFile("assets/textures/divider_left.png")) {
        std::cout << "Failed to load Divider Left Texture\n"; return false;
    }
    if (!dividerRight.loadFromFile("assets/textures/divider_right.png")) {
        std::cout << "Failed to load Divider Right Texture\n"; return false;
    }
    if (!menuButton.loadFromFile("assets/gfx/buttons/button_menu.png")) {
        std::cout << "Failed to load Menu Button Texture\n"; return false;
    }
    if (!buttonInventory.loadFromFile("assets/gfx/buttons/button_inventory.png")) {
        std::cout << "Failed to load Inventory Button Texture\n"; return false;
    }
    if (!buttonSkills.loadFromFile("assets/gfx/buttons/button_skills.png")) {
        std::cout << "Failed to load Skills Button Texture\n"; return false;
    }
    if (!buttonCharacter.loadFromFile("assets/gfx/buttons/button_character.png")) {
        std::cout << "Failed to load Character Button Texture\n"; return false;
    }
    if (!buttonMap.loadFromFile("assets/gfx/buttons/button_map.png")) {
        std::cout << "Failed to load Map Button Texture\n"; return false;
    }
    if (!buttonQuests.loadFromFile("assets/gfx/buttons/button_quests.png")) {
        std::cout << "Failed to load Quests Button Texture\n"; return false;
    }
    if (!buttonSettings.loadFromFile("assets/gfx/buttons/button_settings.png")) {
        std::cout << "Failed to load Settings Button Texture\n"; return false;
    }
    if (!buttonRankings.loadFromFile("assets/gfx/buttons/button_rankings.png")) {
        std::cout << "Failed to load Rankings Button Texture\n"; return false;
    }
    if (!questbookBackground.loadFromFile("assets/gfx/menu/quests/questbook.png")) {
        std::cout << "Failed to load Questbook Background\n"; return false;
    }
    if (!skilltree.loadFromFile("assets/gfx/menu/skills/skilltree.png")) {
        std::cout << "Failed to load Skilltree Texture\n"; return false;
    }
    if (!skillbar.loadFromFile("assets/gfx/menu/skills/Skillbar_DarkPurple.png")) {
        std::cout << "Failed to load Skillbar Texture\n"; return false;
    }
    if (!buttonHelp.loadFromFile("assets/gfx/buttons/button_help.png")) {
        std::cout << "Failed to load Help Button Texture\n"; return false;
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
    if (!enterKey.loadFromFile("assets/audio/enterKey.mp3")) { // Pressing Enter Key
        std::cout << "Failed to load Enter Key Sound\n"; return false;
    }
    if (!typewriter.loadFromFile("assets/audio/typewriter.mp3")) { // isTyping Sound
        std::cout << "Failed to load Typewriter Sound\n"; return false;
    }
    if (!reject.loadFromFile("assets/audio/reject.mp3")) { // Reject Choice
        std::cout << "Failed to load Reject Sound\n"; return false;
    }
    if (!startGame.loadFromFile("assets/audio/startGame.mp3")) { // Start Game
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
    if (!healSound.loadFromFile(ResourceFiles::Items::kHealPotionSound)) { // Heal Potion Sound
        std::cout << "Failed to load Heal Potion Sound\n"; return false;
    }
    if (!forgeSound.loadFromFile("assets/audio/forgeSound.mp3")) { // Forge Sleep Sound
        std::cout << "Failed to load Forge Sound\n"; return false;
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
    if (!locationMusicPerigonal.loadFromFile("assets/audio/locationMusicPerigonal.mp3")) { // Perigonal Location Sound
        std::cout << "Failed to load Perigonal Location Music\n"; return false;
    }
    if (!locationMusicBlacksmith.loadFromFile("assets/audio/LocationMusicBlacksmith.mp3")) {
        std::cout << "Failed to load Blacksmith Location Music\n"; return false;
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
    if (!introTitle.loadFromFile("assets/audio/introTitle.mp3")) { // Intro Title Sound
        std::cout << "Failed to load Intro Title Music\n"; return false;
    }
    if (!introDialogue.loadFromFile("assets/audio/introDialogue.mp3")) { // Intro Dialogue Sound
        std::cout << "Failed to load Intro Dialogue Music\n"; return false;
    }
    if (!buttonHovered.loadFromFile("assets/audio/hoverButtons.mp3")) { // Button Hover Sound
        std::cout << "Failed to load Button Hover Sound\n"; return false;
    }
    if (!titleButtons.loadFromFile("assets/audio/titleButtons.mp3")) { // Intro Title Button Hover
        std::cout << "Failed to load Title Button Hover Sound\n"; return false;
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

    // All asset loads succeeded.
    return true;
}
