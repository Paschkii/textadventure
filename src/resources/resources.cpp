// === C++ Libraries ===
#include <iostream>  // Logs asset-loading failures during Resources::loadAll.
// === SFML Libraries ===
#include <SFML/Graphics/Shader.hpp>
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
    if (!battleFont.openFromFile("assets/fonts/font.ttf")) { // Font: Battle UI
        std::cout << "Failed to load font.ttf\n"; return false;
    }
    if (!battleFontBold.openFromFile("assets/fonts/fontBold.ttf")) { // Font: Battle UI
        std::cout << "Failed to load font.ttf\n"; return false;
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
    if (!weaponHolmabir.loadFromFile("assets/gfx/weapons/equipment/holmabir.png")) {
        std::cout << "Failed to load Holmabir Equipment Texture\n"; return false;
    }
    if (!weaponHolmabirBroken.loadFromFile("assets/gfx/weapons/broken/holmabir_broken.png")) {
        std::cout << "Failed to load Holmabir Broken Texture\n"; return false;
    }
    if (!weaponKattkavar.loadFromFile("assets/gfx/weapons/equipment/kattkavar_righthand.png")) {
        std::cout << "Failed to load Kattkavar Equipment Texture (righthand)\n"; return false;
    }
    if (!weaponKattkavarLeft.loadFromFile("assets/gfx/weapons/equipment/kattkavar_lefthand.png")) {
        std::cout << "Failed to load Kattkavar Equipment Texture (lefthand)\n"; return false;
    }
    if (!weaponKattkavarBroken.loadFromFile("assets/gfx/weapons/broken/kattkavar_broken.png")) {
        std::cout << "Failed to load Kattkavar Broken Texture\n"; return false;
    }
    if (!weaponStiggedin.loadFromFile("assets/gfx/weapons/equipment/stiggedin_righthand.png")) {
        std::cout << "Failed to load Stiggedin Equipment Texture (righthand)\n"; return false;
    }
    if (!weaponStiggedinLeft.loadFromFile("assets/gfx/weapons/equipment/stiggedin_lefthand.png")) {
        std::cout << "Failed to load Stiggedin Equipment Texture (lefthand)\n"; return false;
    }
    if (!weaponStiggedinBroken.loadFromFile("assets/gfx/weapons/broken/stiggedin_broken.png")) {
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
    if (!portraitVillageElder.loadFromFile("assets/gfx/chars/portraits/portrait_noah_lott.png")) { // Portrait: Noah Lott
        std::cout << "Failed to load Noah Lott Portrait\n"; return false;
    }
    if (!portraitVillageWanderer.loadFromFile("assets/gfx/chars/portraits/portrait_wanda_rinn.png")) { // Portrait: Wanda Rinn
        std::cout << "Failed to load Village Wanderer Portrait\n"; return false;
    }
    if (!portraitBlacksmith.loadFromFile("assets/gfx/chars/portraits/portrait_will_figsid.png")) { // Portrait: Will Figsid
        std::cout << "Failed to load Blacksmith Portrait\n"; return false;
    }
    if (!portraitNoahBates.loadFromFile("assets/gfx/characters/noah_bates.png")) { // Portrait: Noah Bates
        std::cout << "Failed to load Noah Bates Portrait\n"; return false;
    }
    if (!portraitFireDragon.loadFromFile("assets/gfx/chars/portraits/portrait_rowsted_sheacane.png")) { // Portrait: Rowsted Sheacane
        std::cout << "Failed to load Fire Dragon Portrait\n"; return false;
    }
    if (!portraitWaterDragon.loadFromFile("assets/gfx/chars/portraits/portrait_flawtin_seamen.png")) { // Portrait: Flawtin Seamen
        std::cout << "Failed to load Water Dragon Portrait\n"; return false;
    }
    if (!portraitAirDragon.loadFromFile("assets/gfx/chars/portraits/portrait_gustavo_windimaess.png")) { // Portrait: Gustavo Windimaess
        std::cout << "Failed to load Air Dragon Portrait\n"; return false;
    }
    if (!portraitEarthDragon.loadFromFile("assets/gfx/chars/portraits/portrait_grounded_claymore.png")) { // Portrait: Grounded Claymore
        std::cout << "Failed to load Earth Dragon Portrait\n"; return false;
    }
    if (!portraitMasterBates.loadFromFile("assets/gfx/chars/portraits/portrait_master_bates.png")) { // Portrait: Master Bates
        std::cout << "Failed to load Master Bates Portrait\n"; return false;
    }
    if (!portraitMasterBatesDragon.loadFromFile("assets/gfx/chars/portraits/portrait_master_bates_dragon.png")) { // Portrait: Master Bates (dragon form)
        std::cout << "Failed to load Master Bates Dragon Portrait\n"; return false;
    }
    if (!portraitDragonbornMaleCape.loadFromFile("assets/gfx/chars/portraits/portrait_dragonborn_male_cape.png")) {
        std::cout << "Failed to load Dragonborn Male Portrait (Cape)\n"; return false;
    }
    if (!portraitDragonbornMaleNoCape.loadFromFile("assets/gfx/chars/portraits/portrait_dragonborn_male_nocape.png")) {
        std::cout << "Failed to load Dragonborn Male Portrait (No Cape)\n"; return false;
    }
    if (!portraitDragonbornFemaleCape.loadFromFile("assets/gfx/chars/portraits/portrait_dragonborn_female_cape.png")) {
        std::cout << "Failed to load Dragonborn Female Portrait (Cape)\n"; return false;
    }
    if (!portraitDragonbornFemaleNoCape.loadFromFile("assets/gfx/chars/portraits/portrait_dragonborn_female_nocape.png")) {
        std::cout << "Failed to load Dragonborn Female Portrait (No Cape)\n"; return false;
    }

    // ----------------------------
    //     Portrait Backgrounds
    // ----------------------------
    if (!portraitBackgroundAerobronchi.loadFromFile("assets/gfx/chars/portraitsBackground/portraitBackground_aerobronchi.png")) {
        std::cout << "Failed to load Aerobronchi Portrait Background\n"; return false;
    }
    if (!portraitBackgroundBlacksmith.loadFromFile("assets/gfx/chars/portraitsBackground/portraitBackground_blacksmith.png")) {
        std::cout << "Failed to load Blacksmith Portrait Background\n"; return false;
    }
    if (!portraitBackgroundBlyathyroid.loadFromFile("assets/gfx/chars/portraitsBackground/portraitBackground_blyathyroid.png")) {
        std::cout << "Failed to load Blyathyroid Portrait Background\n"; return false;
    }
    if (!portraitBackgroundCladrenal.loadFromFile("assets/gfx/chars/portraitsBackground/portraitBackground_cladrenal.png")) {
        std::cout << "Failed to load Cladrenal Portrait Background\n"; return false;
    }
    if (!portraitBackgroundGonad.loadFromFile("assets/gfx/chars/portraitsBackground/portraitBackground_gonad.png")) {
        std::cout << "Failed to load Gonad Portrait Background\n"; return false;
    }
    if (!portraitBackgroundLacrimere.loadFromFile("assets/gfx/chars/portraitsBackground/portraitBackground_lacrimere.png")) {
        std::cout << "Failed to load Lacrimere Portrait Background\n"; return false;
    }
    if (!portraitBackgroundPetrigonal.loadFromFile("assets/gfx/chars/portraitsBackground/portraitBackground_petrigonal.png")) {
        std::cout << "Failed to load Petrigonal Portrait Background\n"; return false;
    }
    if (!portraitBackgroundSeminiferous.loadFromFile("assets/gfx/chars/portraitsBackground/portraitBackground_seminiferous.png")) {
        std::cout << "Failed to load Seminiferous Portrait Background\n"; return false;
    }
    if (!portraitBackgroundToryTailor.loadFromFile("assets/gfx/chars/portraitsBackground/portraitBackground_torytailor.png")) {
        std::cout << "Failed to load Tory Tailor Portrait Background\n"; return false;
    }

    // ----------------------------
    //       Character Sprites
    // ----------------------------
    if (!spriteDragonbornBack.loadFromFile("assets/gfx/chars/sprites/dragonborn_backsprite.png")) {
        std::cout << "Failed to load Dragonborn Back Sprite\n"; return false;
    }
    if (!spriteDragonbornFemale.loadFromFile("assets/gfx/chars/sprites/dragonborn_female_sprite_cape.png")) {
        std::cout << "Failed to load Dragonborn Female Sprite\n"; return false;
    }
    if (!spriteDragonbornMale.loadFromFile("assets/gfx/chars/sprites/dragonborn_male_sprite_cape.png")) {
        std::cout << "Failed to load Dragonborn Male Sprite\n"; return false;
    }
    if (!spriteDragonbornFemaleNoCape.loadFromFile("assets/gfx/chars/sprites/dragonborn_female_sprite_nocape.png")) {
        std::cout << "Failed to load Dragonborn Female No Cape Sprite\n"; return false;
    }
    if (!spriteDragonbornMaleNoCape.loadFromFile("assets/gfx/chars/sprites/dragonborn_male_sprite_nocape.png")) {
        std::cout << "Failed to load Dragonborn Male No Cape Sprite\n"; return false;
    }
    if (!spriteDragonbornFemaleHovered.loadFromFile("assets/gfx/chars/sprites/dragonborn_female_sprite_cape_hovered.png")) {
        std::cout << "Failed to load Dragonborn Female Hovered Sprite\n"; return false;
    }
    if (!spriteDragonbornMaleHovered.loadFromFile("assets/gfx/chars/sprites/dragonborn_male_sprite_cape_hovered.png")) {
        std::cout << "Failed to load Dragonborn Male Hovered Sprite\n"; return false;
    }
    if (!spriteFlawtinSeamen.loadFromFile("assets/gfx/chars/sprites/flawtin_seamen_sprite.png")) {
        std::cout << "Failed to load Flawtin Seamen Sprite\n"; return false;
    }
    if (!spriteGlandularCape.loadFromFile("assets/gfx/menu/character/glandular_cape.png")) {
        std::cout << "Failed to load Glandular Cape Sprite\n"; return false;
    }
    if (!spriteGroundedClaymore.loadFromFile("assets/gfx/chars/sprites/grounded_claymore_sprite.png")) {
        std::cout << "Failed to load Grounded Claymore Sprite\n"; return false;
    }
    if (!spriteGustavoWindimaess.loadFromFile("assets/gfx/chars/sprites/gustavo_windimaess_sprite.png")) {
        std::cout << "Failed to load Gustavo Windimaess Sprite\n"; return false;
    }
    if (!spriteMasterBates.loadFromFile("assets/gfx/chars/sprites/master_bates_sprite.png")) {
        std::cout << "Failed to load Master Bates Sprite\n"; return false;
    }
    if (!spriteMasterBatesDragon.loadFromFile("assets/gfx/chars/sprites/master_bates_dragon_sprite.png")) {
        std::cout << "Failed to load Master Bates Dragon Sprite\n"; return false;
    }
    if (!spriteNoahLott.loadFromFile("assets/gfx/chars/sprites/noah_lott_sprite.png")) {
        std::cout << "Failed to load Noah Lott Sprite\n"; return false;
    }
    if (!spriteRowstedSheacane.loadFromFile("assets/gfx/chars/sprites/rowsted_sheacane_sprite.png")) {
        std::cout << "Failed to load Rowsted Sheacane Sprite\n"; return false;
    }
    if (!spriteWandaRinn.loadFromFile("assets/gfx/chars/sprites/wanda_rinn_sprite.png")) {
        std::cout << "Failed to load Wanda Rinn Sprite\n"; return false;
    }
    if (!spriteWillFigsid.loadFromFile("assets/gfx/chars/sprites/will_figsid_sprite.png")) {
        std::cout << "Failed to load Will Figsid Sprite\n"; return false;
    }

    // ---------------------------
    //        Map Textures
    // ---------------------------
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
    if (!battleBackgroundLayer.loadFromFile("assets/gfx/battleUI/battlebackground.png")) {
        std::cout << "Failed to load battle background layer\n"; return false;
    }
    if (!battleBeginsGlandular.loadFromFile("assets/gfx/battleUI/battleBeginsGlandular.png")) {
        std::cout << "Failed to load battle begins badge\n"; return false;
    }
    if (!battlePlayerPlatform.loadFromFile("assets/gfx/battleUI/playerPlatform.png")) {
        std::cout << "Failed to load battle player platform\n"; return false;
    }
    if (!battleEnemyPlatform.loadFromFile("assets/gfx/battleUI/enemyPlatform.png")) {
        std::cout << "Failed to load battle enemy platform\n"; return false;
    }
    if (!battleTextBox.loadFromFile("assets/gfx/battleUI/textBox.png")) {
        std::cout << "Failed to load battle text box\n"; return false;
    }
    if (!battleActionBox.loadFromFile("assets/gfx/battleUI/actionBox.png")) {
        std::cout << "Failed to load battle action box\n"; return false;
    }
    if (!battlePlayerBox.loadFromFile("assets/gfx/battleUI/playerBox.png")) {
        std::cout << "Failed to load battle player box\n"; return false;
    }
    if (!battleEnemyBox.loadFromFile("assets/gfx/battleUI/enemyBox.png")) {
        std::cout << "Failed to load battle enemy box\n"; return false;
    }
    if (!battleIconMale.loadFromFile("assets/gfx/battleUI/icon_male.png")) {
        std::cout << "Failed to load battle male icon\n"; return false;
    }
    if (!battleIconFemale.loadFromFile("assets/gfx/battleUI/icon_female.png")) {
        std::cout << "Failed to load battle female icon\n"; return false;
    }
    if (!skillSlashAir.loadFromFile("assets/gfx/menu/skills/skill_slash_air.png")) {
        std::cout << "Failed to load skill slash air texture\n"; return false;
    }
    if (!skillSlashEarth.loadFromFile("assets/gfx/menu/skills/skill_slash_earth.png")) {
        std::cout << "Failed to load skill slash earth texture\n"; return false;
    }
    if (!skillSlashFire.loadFromFile("assets/gfx/menu/skills/skill_slash_fire.png")) {
        std::cout << "Failed to load skill slash fire texture\n"; return false;
    }
    if (!skillSlashWater.loadFromFile("assets/gfx/menu/skills/skill_slash_water.png")) {
        std::cout << "Failed to load skill slash water texture\n"; return false;
    }
    if (!skillWeaponAir.loadFromFile("assets/gfx/menu/skills/skill_weapon_air.png")) {
        std::cout << "Failed to load skill weapon air texture\n"; return false;
    }
    if (!skillWeaponEarth.loadFromFile("assets/gfx/menu/skills/skill_weapon_earth.png")) {
        std::cout << "Failed to load skill weapon earth texture\n"; return false;
    }
    if (!skillWeaponFire.loadFromFile("assets/gfx/menu/skills/skill_weapon_fire.png")) {
        std::cout << "Failed to load skill weapon fire texture\n"; return false;
    }
    if (!skillWeaponWater.loadFromFile("assets/gfx/menu/skills/skill_weapon_water.png")) {
        std::cout << "Failed to load skill weapon water texture\n"; return false;
    }
    if (!hpBadgeShader.loadFromFile("assets/shaders/hp_badge.frag", sf::Shader::Type::Fragment)) {
        std::cout << "Failed to load hp badge shader\n"; return false;
    }
    hpBadgeShader.setUniform("texture", sf::Shader::CurrentTexture);
    hpBadgeShader.setUniform("diagonal", 0.25f);
    if (!treasureChestAir.loadFromFile("assets/gfx/trophies/treasure_chest_air.png")) {
        std::cout << "Failed to load treasure_chest_air.png\n"; return false;
    }
    if (!treasureChestEarth.loadFromFile("assets/gfx/trophies/treasure_chest_earth.png")) {
        std::cout << "Failed to load treasure_chest_earth.png\n"; return false;
    }
    if (!treasureChestFire.loadFromFile("assets/gfx/trophies/treasure_chest_fire.png")) {
        std::cout << "Failed to load treasure_chest_fire.png\n"; return false;
    }
    if (!treasureChestWater.loadFromFile("assets/gfx/trophies/treasure_chest_water.png")) {
        std::cout << "Failed to load treasure_chest_water.png\n"; return false;
    }
    if (!umbraUsseaOne.loadFromFile("assets/gfx/menu/map/Umbra_Ussea_one.png")) {
        std::cout << "Failed to load Umbra_Ussea_one.png\n"; return false;
    }
    if (!umbraUsseaTwo.loadFromFile("assets/gfx/menu/map/Umbra_Ussea_two.png")) {
        std::cout << "Failed to load Umbra_Ussea_two.png\n"; return false;
    }
    if (!umbraUsseaThree.loadFromFile("assets/gfx/menu/map/Umbra_Ussea_three.png")) {
        std::cout << "Failed to load Umbra_Ussea_three.png\n"; return false;
    }
    if (!umbraUsseaFour.loadFromFile("assets/gfx/menu/map/Umbra_Ussea_four.png")) {
        std::cout << "Failed to load Umbra_Ussea_four.png\n"; return false;
    }
    if (!umbraUsseaComplete.loadFromFile("assets/gfx/menu/map/Umbra_Ussea.png")) {
        std::cout << "Failed to load Umbra_Ussea.png\n"; return false;
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
    if (!elementAirButton.loadFromFile("assets/gfx/buttons/element_air.png")) {
        std::cout << "Failed to load Element Air Button\n"; return false;
    }
    if (!elementEarthButton.loadFromFile("assets/gfx/buttons/element_earth.png")) {
        std::cout << "Failed to load Element Earth Button\n"; return false;
    }
    if (!elementFireButton.loadFromFile("assets/gfx/buttons/element_fire.png")) {
        std::cout << "Failed to load Element Fire Button\n"; return false;
    }
    if (!elementWaterButton.loadFromFile("assets/gfx/buttons/element_water.png")) {
        std::cout << "Failed to load Element Water Button\n"; return false;
    }
    if (!equipmentRingAir.loadFromFile("assets/gfx/menu/equipment/ring_air.png")) {
        std::cout << "Failed to load Equipment Ring Air icon\n"; return false;
    }
    if (!equipmentRingEarth.loadFromFile("assets/gfx/menu/equipment/ring_earth.png")) {
        std::cout << "Failed to load Equipment Ring Earth icon\n"; return false;
    }
    if (!equipmentRingFire.loadFromFile("assets/gfx/menu/equipment/ring_fire.png")) {
        std::cout << "Failed to load Equipment Ring Fire icon\n"; return false;
    }
    if (!equipmentRingWater.loadFromFile("assets/gfx/menu/equipment/ring_water.png")) {
        std::cout << "Failed to load Equipment Ring Water icon\n"; return false;
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

    // ----------------------------
    //   Character menu assets
    // ----------------------------
    if (!charMenuLeftHand.loadFromFile("assets/gfx/menu/character/charMenu_lefthand.png")) {
        std::cout << "Failed to load Character menu left hand slot\n"; return false;
    }
    if (!charMenuRightHand.loadFromFile("assets/gfx/menu/character/charMenu_righthand.png")) {
        std::cout << "Failed to load Character menu right hand slot\n"; return false;
    }
    if (!charMenuRing.loadFromFile("assets/gfx/menu/character/charMenu_ring.png")) {
        std::cout << "Failed to load Character menu ring slot\n"; return false;
    }
    if (!charMenuDragonstoneSlot.loadFromFile("assets/gfx/menu/character/charMenu_dragonstone.png")) {
        std::cout << "Failed to load Character menu dragonstone slot\n"; return false;
    }
    if (!charMenuDragonclawSlot.loadFromFile("assets/gfx/menu/character/charMenu_dragonclaw.png")) {
        std::cout << "Failed to load Character menu dragonclaw slot\n"; return false;
    }
    if (!charMenuDragonscaleSlot.loadFromFile("assets/gfx/menu/character/charMenu_dragonscale.png")) {
        std::cout << "Failed to load Character menu dragonscale slot\n"; return false;
    }
    if (!charMenuLuckyCharmSlot.loadFromFile("assets/gfx/menu/character/charMenu_luckycharm.png")) {
        std::cout << "Failed to load Character menu lucky charm slot\n"; return false;
    }
    if (!charMenuFemaleNoCape.loadFromFile("assets/gfx/menu/character/charMenu_female_nocape.png")) {
        std::cout << "Failed to load Character menu female base portrait\n"; return false;
    }
    if (!charMenuMaleNoCape.loadFromFile("assets/gfx/menu/character/charMenu_male_nocape.png")) {
        std::cout << "Failed to load Character menu male base portrait\n"; return false;
    }
    if (!charMenuFemaleCape.loadFromFile("assets/gfx/menu/character/charMenu_female_cape.png")) {
        std::cout << "Failed to load Character menu female cloak portrait\n"; return false;
    }
    if (!charMenuMaleCape.loadFromFile("assets/gfx/menu/character/charMenu_male_cape.png")) {
        std::cout << "Failed to load Character menu male cloak portrait\n"; return false;
    }
    if (!charMenuCloak.loadFromFile("assets/gfx/menu/character/glandular_cape.png")) {
        std::cout << "Failed to load Character menu cloak icon\n"; return false;
    }

    // ----------------------------
    //      Artifact icons
    // ----------------------------
    if (!artifactDragonscaleAir.loadFromFile("assets/gfx/menu/artifacts/dragonscale/dragonscale_air.png")) {
        std::cout << "Failed to load Dragonscale Air icon\n"; return false;
    }
    if (!artifactDragonscaleEarth.loadFromFile("assets/gfx/menu/artifacts/dragonscale/dragonscale_earth.png")) {
        std::cout << "Failed to load Dragonscale Earth icon\n"; return false;
    }
    if (!artifactDragonscaleFire.loadFromFile("assets/gfx/menu/artifacts/dragonscale/dragonscale_fire.png")) {
        std::cout << "Failed to load Dragonscale Fire icon\n"; return false;
    }
    if (!artifactDragonscaleWater.loadFromFile("assets/gfx/menu/artifacts/dragonscale/dragonscale_water.png")) {
        std::cout << "Failed to load Dragonscale Water icon\n"; return false;
    }
    if (!artifactDragonstoneAir.loadFromFile("assets/gfx/menu/artifacts/dragonstone/dragonstone_air.png")) {
        std::cout << "Failed to load Dragonstone Air icon\n"; return false;
    }
    if (!artifactDragonstoneEarth.loadFromFile("assets/gfx/menu/artifacts/dragonstone/dragonstone_earth.png")) {
        std::cout << "Failed to load Dragonstone Earth icon\n"; return false;
    }
    if (!artifactDragonstoneFire.loadFromFile("assets/gfx/menu/artifacts/dragonstone/dragonstone_fire.png")) {
        std::cout << "Failed to load Dragonstone Fire icon\n"; return false;
    }
    if (!artifactDragonstoneWater.loadFromFile("assets/gfx/menu/artifacts/dragonstone/dragonstone_water.png")) {
        std::cout << "Failed to load Dragonstone Water icon\n"; return false;
    }
    if (!artifactDragonclawAir.loadFromFile("assets/gfx/menu/artifacts/dragonclaw/dragonclaw_air.png")) {
        std::cout << "Failed to load Dragonclaw Air icon\n"; return false;
    }
    if (!artifactDragonclawEarth.loadFromFile("assets/gfx/menu/artifacts/dragonclaw/dragonclaw_earth.png")) {
        std::cout << "Failed to load Dragonclaw Earth icon\n"; return false;
    }
    if (!artifactDragonclawFire.loadFromFile("assets/gfx/menu/artifacts/dragonclaw/dragonclaw_fire.png")) {
        std::cout << "Failed to load Dragonclaw Fire icon\n"; return false;
    }
    if (!artifactDragonclawWater.loadFromFile("assets/gfx/menu/artifacts/dragonclaw/dragonclaw_water.png")) {
        std::cout << "Failed to load Dragonclaw Water icon\n"; return false;
    }
    if (!artifactLuckyCharmAir.loadFromFile("assets/gfx/menu/artifacts/luckycharm/luckyCharm_air.png")) {
        std::cout << "Failed to load Lucky Charm Air icon\n"; return false;
    }
    if (!artifactLuckyCharmEarth.loadFromFile("assets/gfx/menu/artifacts/luckycharm/luckyCharm_earth.png")) {
        std::cout << "Failed to load Lucky Charm Earth icon\n"; return false;
    }
    if (!artifactLuckyCharmFire.loadFromFile("assets/gfx/menu/artifacts/luckycharm/luckyCharm_fire.png")) {
        std::cout << "Failed to load Lucky Charm Fire icon\n"; return false;
    }
    if (!artifactLuckyCharmWater.loadFromFile("assets/gfx/menu/artifacts/luckycharm/luckyCharm_water.png")) {
        std::cout << "Failed to load Lucky Charm Water icon\n"; return false;
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
    if (!skillSlash.loadFromFile("assets/audio/skill_slash.mp3")) {
        std::cout << "Failed to load Skill Slash Sound\n"; return false;
    }
    if (!skillAir.loadFromFile("assets/audio/skill_air.mp3")) {
        std::cout << "Failed to load Air Skill Sound\n"; return false;
    }
    if (!skillEarth.loadFromFile("assets/audio/skill_earth.mp3")) {
        std::cout << "Failed to load Earth Skill Sound\n"; return false;
    }
    if (!skillFire.loadFromFile("assets/audio/skill_fire.mp3")) {
        std::cout << "Failed to load Fire Skill Sound\n"; return false;
    }
    if (!skillWater.loadFromFile("assets/audio/skill_water.mp3")) {
        std::cout << "Failed to load Water Skill Sound\n"; return false;
    }
    if (!skillFriendship.loadFromFile("assets/audio/skill_friendship.mp3")) {
        std::cout << "Failed to load Friendship Skill Sound\n"; return false;
    }
    if (!forgeSound.loadFromFile("assets/audio/forgeSound.mp3")) { // Forge Sleep Sound
        std::cout << "Failed to load Forge Sound\n"; return false;
    }
    if (!levelUp.loadFromFile("assets/audio/level_up.mp3")) {
        std::cout << "Failed to load Level Up Sound\n"; return false;
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
    if (!questStart.loadFromFile("assets/audio/quest_start.mp3")) {
        std::cout << "Failed to load Quest Start Sound\n"; return false;
    }
    if (!questEnd.loadFromFile("assets/audio/quest_end.mp3")) {
        std::cout << "Failed to load Quest End Sound\n"; return false;
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
    if (!openMenu.loadFromFile("assets/audio/open_menu.mp3")) {
        std::cout << "Failed to load Open Menu Sound\n"; return false;
    }
    if (!closeMenu.loadFromFile("assets/audio/close_menu.mp3")) {
        std::cout << "Failed to load Close Menu Sound\n"; return false;
    }
    if (!popup.loadFromFile("assets/audio/popup.mp3")) {
        std::cout << "Failed to load Popup Sound\n"; return false;
    }
    if (!xpGain.loadFromFile("assets/audio/xp_gain.mp3")) {
        std::cout << "Failed to load XP Gain Sound\n"; return false;
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
