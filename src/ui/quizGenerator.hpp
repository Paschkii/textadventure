#pragma once
// === C++ Libraries ===
#include <array>   // Stores the fixed-answer slots for each quiz question.
#include <random>  // Generates randomness for question variants and option ordering.
#include <string>  // Holds prompts, explanations, and generated text for questions.
#include <vector>  // Returns collections of generated quiz questions.

namespace quiz {

enum class Category {
    Name,
    Multiplication,
    Division,
    Addition,
    Subtraction,
    Silly
};

struct Question {
    std::string prompt;
    std::array<std::string, 4> options{};
    int correctIndex = 0;
    Category category = Category::Multiplication;
    std::string explanation;
    bool acceptAnyAnswer = false;
};

// Build a full quiz consisting of four questions (one per category) with
// randomized order and variant.
std::vector<Question> generateNumberQuiz(std::mt19937& rng);

// Generate a single question for a given category (random variant).
Question generateQuestion(Category category, std::mt19937& rng);

// Helper to stringify the category for diagnostics or debugging.
std::string toString(Category category);

} // namespace quiz
