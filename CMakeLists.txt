cmake_minimum_required(VERSION 3.16)
project(textadventure LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics Window System Audio REQUIRED)

# Alle Pfade RELATIV zum Projektordner, OHNE führenden Slash
add_executable(Glandular
    src/core/main.cpp
    src/core/game.cpp

    src/ui/introScreen.cpp
    src/ui/introTitle.cpp
    src/ui/confirmationUI.cpp
    src/ui/dialogDrawElements.cpp
    src/ui/dialogUI.cpp
    src/ui/uiEffects.cpp
    src/ui/uiVisibility.cpp
    src/helper/weaponHelpers.cpp
    src/helper/dragonHelpers.cpp
    src/helper/layoutHelpers.cpp
    src/ui/mapSelectionUI.cpp
    src/ui/quizGenerator.cpp
    src/ui/quizUI.cpp
    src/ui/weaponSelectionUI.cpp

    src/helper/textColorHelper.cpp
    src/rendering/textLayout.cpp
    src/rendering/locations.cpp

    src/story/textStyles.cpp

    src/resources/resources.cpp
)

target_include_directories(Glandular PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Assets kopieren
add_custom_target(copy_textadventure_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            ${CMAKE_BINARY_DIR}/assets
)

add_dependencies(Glandular copy_textadventure_assets)

target_link_libraries(Glandular PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)