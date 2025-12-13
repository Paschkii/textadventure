#include <iostream>
#include <random>
#include <string>
#include <filesystem>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include "game.hpp"
#include "ui/quizGenerator.hpp"

int main(int argc, char** argv) {
    namespace fs = std::filesystem;

    // Wichtig: damit "assets/..." beim Start via Finder/open gefunden wird
    try {
        fs::path exePath = fs::canonical(argv[0]);
        fs::current_path(exePath.parent_path());
    } catch (...) {
        // falls canonical() aus irgendeinem Grund nicht geht: egal, dann läuft's wie bisher
    }

    if (argc > 1 && std::string(argv[1]) == "--quiz-dev") {
        std::mt19937 rng(std::random_device{}());
        auto questions = quiz::generateNumberQuiz(rng);
        const char labels[] = { 'A', 'B', 'C', 'D' };

        for (std::size_t i = 0; i < questions.size(); ++i) {
            const auto& q = questions[i];
            std::cout << "Question " << (i + 1) << " (" << quiz::toString(q.category) << ")\n";
            std::cout << q.prompt << "\n";
            for (int j = 0; j < 4; ++j) {
                bool correct = j == q.correctIndex;
                std::cout << "  " << labels[j] << ") " << q.options[j];
                if (correct)
                    std::cout << "  <-- correct";
                std::cout << "\n";
            }
            std::cout << "\n";
        }
        return 0;
    }

    Game game;
    game.run();
    return 0;
}