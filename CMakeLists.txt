cmake_minimum_required(VERSION 3.16)
project(textadventure LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics Window System Audio REQUIRED)

# Alle Pfade RELATIV zum Projektordner, OHNE führenden Slash
add_executable(Glandular
    src/core/main.cpp
    src/core/game.cpp
    src/core/teleportController.cpp
    src/core/endSequenceController.cpp
    src/core/ranking.cpp
    src/core/itemController.cpp
    src/core/itemActivation.cpp

    src/ui/introScreen.cpp
    src/ui/creditsUI.cpp
    src/ui/battleUI.cpp
    src/ui/introTitle.cpp
    src/ui/menuUI.cpp
    src/ui/confirmationUI.cpp
    src/ui/dialogDrawElements.cpp
    src/ui/dialogUI.cpp
    src/ui/brokenWeaponPreview.cpp
    src/ui/genderSelectionUI.cpp
    src/ui/uiEffects.cpp
    src/ui/uiVisibility.cpp
    src/helper/weaponHelpers.cpp
    src/helper/dragonHelpers.cpp
    src/helper/layoutHelpers.cpp
    src/helper/healingPotion.cpp
    src/helper/soundHelper.cpp
    src/audio/audioManager.cpp
    src/ui/mapSelectionUI.cpp
    src/ui/quizGenerator.cpp
    src/ui/sillyRiddles.cpp
    src/ui/quizUI.cpp
    src/ui/treasureChestUI.cpp
    src/ui/rankingUI.cpp
    src/ui/weaponSelectionUI.cpp

    src/helper/textColorHelper.cpp
    src/rendering/textLayout.cpp
    src/rendering/locations.cpp

    src/story/textStyles.cpp
    src/story/locationDialogues.cpp
    src/items/itemRegistry.cpp

    src/resources/resources.cpp
)

target_include_directories(Glandular PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(Glandular PRIVATE TEXTADVENTURE_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")

# Assets kopieren
add_custom_target(copy_textadventure_assets ALL
    COMMAND ${CMAKE_COMMAND}
            -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -DDEST_DIR=${CMAKE_BINARY_DIR}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/scripts/copy_assets_with_rankings.cmake"
)

add_dependencies(Glandular copy_textadventure_assets)

target_link_libraries(Glandular PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)
