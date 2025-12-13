#pragma once
#include <array>
#include <random>
#include <string>
#include <vector>

namespace quiz {

enum class Category {
    Name,
    Multiplication,
    Division,
    Addition,
    Subtraction
};

struct Question {
    std::string prompt;
    std::array<std::string, 4> options{};
    int correctIndex = 0;
    Category category = Category::Multiplication;
    std::string explanation;
};

// Build a full quiz consisting of four questions (one per category) with
// randomized order and variant.
std::vector<Question> generateNumberQuiz(std::mt19937& rng);

// Generate a single question for a given category (random variant).
Question generateQuestion(Category category, std::mt19937& rng);

// Helper to stringify the category for diagnostics or debugging.
std::string toString(Category category);

} // namespace quiz
