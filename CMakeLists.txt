cmake_minimum_required(VERSION 3.16)
project(textadventure LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Alle Pfade RELATIV zum Projektordner, OHNE führenden Slash
add_executable(textadventure
    src/core/main.cpp
    src/core/game.cpp

    src/ui/introScreen.cpp
    src/ui/introTitle.cpp
    src/ui/dialogUI.cpp
    src/ui/uiVisibility.cpp
    src/ui/mapSelectionUI.cpp
    src/ui/quizUI.cpp
    src/ui/weaponSelectionUI.cpp

    src/rendering/textColorHelper.cpp
    src/rendering/textLayout.cpp

    src/story/textStyles.cpp

    src/resources/resources.cpp
)

target_include_directories(textadventure PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Assets kopieren (das kannst du so lassen)
add_custom_target(copy_textadventure_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            ${CMAKE_BINARY_DIR}/assets
)

add_dependencies(textadventure copy_textadventure_assets)