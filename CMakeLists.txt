cmake_minimum_required(VERSION 3.16)
project(textadventure LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(textadventure
    main.cpp
    game.cpp
    introScreen.cpp
    introTitle.cpp
    dialogUI.cpp
    textColorHelper.cpp
    # ui.cpp
    # audiomanager.cpp
)

# Assets von source/textadventure/assets -> build/.../textadventure/assets kopieren
add_custom_target(copy_textadventure_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            ${CMAKE_BINARY_DIR}/assets
)

add_dependencies(textadventure copy_textadventure_assets)