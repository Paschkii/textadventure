// === C++ Libraries ===
#include <iostream>
#include <random>
#include <string>
#include <filesystem>
// === SFML Libraries ===
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
// === Header Files ===
#include "game.hpp"
#include "ui/quizGenerator.hpp"
#include "ui/quizUI.hpp"

// Bootstraps the application: fixes the working directory, handles dev flags, then runs Game.
int main(int argc, char** argv) {
    namespace fs = std::filesystem;

    // Wichtig: damit "assets/..." beim Start via Finder/open gefunden wird
    try {
        fs::path exePath = fs::canonical(argv[0]);
        fs::current_path(exePath.parent_path());
    } catch (...) {
        // falls canonical() aus irgendeinem Grund nicht geht: egal, dann läuft's wie bisher
    }

    // Exit early if the quiz dev-mode flag is present so we can skip the full game.
    if (runQuizDevMode(argc, argv))
        return 0;

    Game game;
    game.run();
    return 0;
}
