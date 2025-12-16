// === C++ Libraries ===
#include <algorithm>  // Uses std::max/min/shuffle when choosing variants and shuffling answers.
#include <cmath>      // Computes ranges for random adjustments like `randomNear`.
#include <set>        // Tracks used values to ensure each option stays unique during generation.
// === Header Files ===
#include "quizGenerator.hpp"  // Declares quiz helper APIs defined below.

namespace quiz {
namespace {

int randomInRange(int minInclusive, int maxInclusive, std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
    return dist(rng);
}

int randomNear(int center, std::mt19937& rng) {
    int span = std::max(2, std::abs(center) / 5);
    int low = std::max(1, center - span);
    int high = center + span;
    std::uniform_int_distribution<int> dist(low, high);
    return dist(rng);
}

int ensureUniquePositive(int value, std::set<int>& used) {
    int candidate = value;
    while (candidate <= 0 || used.count(candidate)) {
        ++candidate;
    }
    used.insert(candidate);
    return candidate;
}

std::string joinSequence(const std::vector<int>& knownTerms) {
    std::string out;
    for (std::size_t i = 0; i < knownTerms.size(); ++i) {
        out += std::to_string(knownTerms[i]);
        out += ", ";
    }
    out += "?";
    return out;
}

std::array<std::string, 4> shuffleOptions(const std::array<int, 4>& values, int correctIndex, std::mt19937& rng, int& outCorrectIndex) {
    std::array<std::string, 4> labels{};
    std::array<int, 4> indices{ 0, 1, 2, 3 };
    std::shuffle(indices.begin(), indices.end(), rng);

    for (std::size_t i = 0; i < indices.size(); ++i) {
        int srcIndex = indices[i];
        labels[i] = std::to_string(values[srcIndex]);
        if (srcIndex == correctIndex)
            outCorrectIndex = static_cast<int>(i);
    }
    return labels;
}

Question finalizeQuestion(
    Category category,
    const std::vector<int>& knownTerms,
    int correct,
    int wrongA,
    int wrongB,
    int wrongC,
    std::mt19937& rng,
    const std::string& explanation
) {
    std::set<int> used;
    int cleanCorrect = ensureUniquePositive(correct, used);
    int cleanWrongA = ensureUniquePositive(wrongA, used);
    int cleanWrongB = ensureUniquePositive(wrongB, used);
    int cleanWrongC = ensureUniquePositive(wrongC, used);

    std::array<int, 4> values{ cleanCorrect, cleanWrongA, cleanWrongB, cleanWrongC };
    int shuffledCorrectIndex = 0;
    std::array<std::string, 4> options = shuffleOptions(values, 0, rng, shuffledCorrectIndex);

    Question q;
    q.prompt = joinSequence(knownTerms);
    q.options = options;
    q.correctIndex = shuffledCorrectIndex;
    q.category = category;
    q.explanation = explanation;
    return q;
}

Question makeMultiplication(int variant, std::mt19937& rng) {
    switch (variant) {
        case 0: {
            // Constant factor
            int start = randomInRange(1, 10, rng);
            int factorChoices[] = { 2, 3, 4 };
            int factor = factorChoices[randomInRange(0, 2, rng)];

            std::vector<int> terms{ start };
            int current = start;
            for (int i = 0; i < 4; ++i) {
                current *= factor;
                terms.push_back(current);
            }
            int correct = current * factor;
            int lastDelta = terms.back() - terms[terms.size() - 2];
            int altFactor = factor == 2 ? 3 : (factor == 4 ? 3 : factor + 1);
            int wrong1 = terms.back() + lastDelta;
            int wrong2 = terms.back() * altFactor;
            int wrong3 = randomNear(correct, rng);

            std::string expl = "Correct! Multiply by " + std::to_string(factor) + " each time; " + std::to_string(terms.back()) + " multiplied by " + std::to_string(factor) + " equals " + std::to_string(correct) + ".";
            return finalizeQuestion(Category::Multiplication, terms, correct, wrong1, wrong2, wrong3, rng, expl);
        }
        case 1: {
            // Increasing factors (factorial style)
            int start = randomInRange(1, 3, rng);
            std::vector<int> terms{ start };
            int current = start;
            int lastFactor = 2;
            for (int factor = 2; factor <= 5; ++factor) {
                current *= factor;
                terms.push_back(current);
                lastFactor = factor;
            }
            int nextFactor = lastFactor + 1;
            int correct = current * nextFactor;

            int wrong1 = current * lastFactor; // repeats the previous multiplier
            int wrong2 = current * 2; // assumes constant *2
            int wrong3 = correct / 2; // halves the expected jump
            if (wrong3 <= 0)
                wrong3 = correct + 1;

            std::string expl = "Correct! Factors grow by one: last was times " + std::to_string(lastFactor) + ", next is times " + std::to_string(nextFactor) + ", so " + std::to_string(current) + " multiplied by " + std::to_string(nextFactor) + " equals " + std::to_string(correct) + ".";
            return finalizeQuestion(Category::Multiplication, terms, correct, wrong1, wrong2, randomNear(wrong3, rng), rng, expl);
        }
        case 2:
        default: {
            // Alternating factors (e.g., *2, *3, *2, *3)
            int start = randomInRange(2, 6, rng);
            std::array<int, 4> factors{ 2, 3, 2, 3 };
            std::vector<int> terms{ start };
            int current = start;
            for (int f : factors) {
                current *= f;
                terms.push_back(current);
            }
            int nextFactor = factors.front();
            int correct = current * nextFactor;

            int wrong1 = current * factors.back(); // repeat last factor
            int wrong2 = current * (nextFactor + 1); // overshoot
            int wrong3 = randomNear(correct, rng);
            std::string expl = "Correct! Pattern alternates times 2 then times 3; after times " + std::to_string(factors.back()) + " comes times " + std::to_string(nextFactor) + ", so " + std::to_string(current) + " multiplied by " + std::to_string(nextFactor) + " equals " + std::to_string(correct) + ".";
            return finalizeQuestion(Category::Multiplication, terms, correct, wrong1, wrong2, wrong3, rng, expl);
        }
    }
}

int pickDivisibleBase(int divisor, std::mt19937& rng) {
    int base = randomInRange(10, 100, rng);
    int remainder = base % divisor;
    if (remainder == 0)
        return base;
    base += divisor - remainder;
    if (base > 100)
        base -= divisor;
    return std::max(divisor, base);
}

Question makeDivision(int variant, std::mt19937& rng) {
    switch (variant) {
        case 0: {
            // Constant divisor
            int divisorChoices[] = { 2, 3, 4, 5 };
            int divisor = divisorChoices[randomInRange(0, 3, rng)];
            int base = pickDivisibleBase(divisor, rng);

            int start = base;
            for (int i = 0; i < 4; ++i)
                start *= divisor;

            std::vector<int> terms{ start };
            int current = start;
            for (int i = 0; i < 3; ++i) {
                current /= divisor;
                terms.push_back(current);
            }

            int correct = current / divisor;

            int altDiv1 = divisor == 2 ? 3 : 2;
            int altDiv2 = std::min(5, divisor + 1);
            int wrong1 = current / altDiv1;
            int wrong2 = current / altDiv2;
            int wrong3 = randomNear(correct, rng);

            std::string expl = "Correct! Divide by " + std::to_string(divisor) + " each step; " + std::to_string(current) + " divided by " + std::to_string(divisor) + " equals " + std::to_string(correct) + ".";
            return finalizeQuestion(Category::Division, terms, correct, wrong1, wrong2, wrong3, rng, expl);
        }
        case 1: {
            // Growing divisors: /2, /3, /4, next /5
            int base = randomInRange(2, 20, rng) * 5; // always divisible by 5
            int start = base * 2 * 3 * 4;
            std::vector<int> terms{ start };
            int current = start;
            int divisors[] = { 2, 3, 4 };
            for (int d : divisors) {
                current /= d;
                terms.push_back(current);
            }

            int correct = current / 5;
            int wrong1 = current / divisors[2]; // repeat last divisor
            int wrong2 = current / divisors[0]; // fall back to the first divisor
            int wrong3 = randomNear(correct, rng);

            std::string expl = "Correct! Divisors grow: divided by 2, then 3, then 4, so next is divided by 5; " + std::to_string(current) + " divided by 5 equals " + std::to_string(correct) + ".";
            return finalizeQuestion(Category::Division, terms, correct, wrong1, wrong2, wrong3, rng, expl);
        }
        case 2:
        default: {
            // Alternating divisors (e.g., /2, /4, /2, /4, next /2)
            int base = pickDivisibleBase(4, rng); // ensure divisible by 4 and 2
            int start = base * 2 * 4 * 2 * 4;
            std::vector<int> terms{ start };
            int current = start;
            std::array<int, 4> divisors{ 2, 4, 2, 4 };
            for (int d : divisors) {
                current /= d;
                terms.push_back(current);
            }

            int correct = current / divisors.front();
            int wrong1 = current / divisors.back(); // repeat last divisor
            int wrong2 = current / 3; // unexpected divisor
            int wrong3 = randomNear(correct, rng);
            std::string expl = "Correct! Divisors alternate divided by 2 then divided by 4; after divided by " + std::to_string(divisors.back()) + " comes divided by " + std::to_string(divisors.front()) + ", so " + std::to_string(current) + " divided by " + std::to_string(divisors.front()) + " equals " + std::to_string(correct) + ".";
            return finalizeQuestion(Category::Division, terms, correct, wrong1, wrong2, wrong3, rng, expl);
        }
    }
}

Question makeAddition(int variant, std::mt19937& rng) {
    switch (variant) {
        case 0: {
            // Constant step
            int start = randomInRange(5, 50, rng);
            int step = randomInRange(3, 25, rng);

            std::vector<int> terms{ start };
            int current = start;
            for (int i = 0; i < 4; ++i) {
                current += step;
                terms.push_back(current);
            }
            int correct = current + step;
            int wrong1 = current + step - 1;
            int wrong2 = current + step + step / 2;
            int wrong3 = randomNear(correct, rng);

            std::string expl = "Correct! Add " + std::to_string(step) + " each time; " + std::to_string(current) + " plus " + std::to_string(step) + " equals " + std::to_string(correct) + ".";
            return finalizeQuestion(Category::Addition, terms, correct, wrong1, wrong2, wrong3, rng, expl);
        }
        case 1: {
            // Growing step
            int start = randomInRange(10, 60, rng);
            int firstStep = randomInRange(4, 15, rng);
            int stepIncrease = randomInRange(2, 10, rng);

            std::vector<int> terms{ start };
            int current = start;
            int step = firstStep;
            int lastUsedStep = step;
            for (int i = 0; i < 3; ++i) {
                current += step;
                terms.push_back(current);
                lastUsedStep = step;
                step += stepIncrease;
            }
            int correct = current + step;
            int wrong1 = current + lastUsedStep; // assumes no growth
            int wrong2 = current + std::max(1, lastUsedStep - stepIncrease); // rolls back the growth
            int wrong3 = randomNear(correct, rng);

            std::string expl = "Correct! Step grows by " + std::to_string(stepIncrease) + ": last was plus " + std::to_string(lastUsedStep) + ", next is plus " + std::to_string(step) + ", so " + std::to_string(current) + " plus " + std::to_string(step) + " equals " + std::to_string(correct) + ".";
            return finalizeQuestion(Category::Addition, terms, correct, wrong1, wrong2, wrong3, rng, expl);
        }
        case 2:
        default: {
            // Alternating steps (d1, d2, d1, d2, next d1)
            int start = randomInRange(5, 80, rng);
            int stepA = randomInRange(3, 18, rng);
            int stepB = randomInRange(2, 16, rng);

            std::vector<int> terms{ start };
            int current = start;
            std::array<int, 4> steps{ stepA, stepB, stepA, stepB };
            for (int s : steps) {
                current += s;
                terms.push_back(current);
            }
            int correct = current + stepA;
            int wrong1 = current + stepB; // repeats last step
            int wrong2 = current + (stepA + stepB) / 2; // averages
            int wrong3 = randomNear(correct, rng);
            std::string expl = "Correct! Steps alternate plus " + std::to_string(stepA) + " then plus " + std::to_string(stepB) + "; after plus " + std::to_string(stepB) + " comes plus " + std::to_string(stepA) + ", so " + std::to_string(current) + " plus " + std::to_string(stepA) + " equals " + std::to_string(correct) + ".";
            return finalizeQuestion(Category::Addition, terms, correct, wrong1, wrong2, wrong3, rng, expl);
        }
    }
}

Question makeSubtraction(int variant, std::mt19937& rng) {
    switch (variant) {
        case 0: {
            // Constant difference
            int step = randomInRange(5, 20, rng);
            int start = randomInRange(140, 260, rng);

            std::vector<int> terms{ start };
            int current = start;
            for (int i = 0; i < 3; ++i) {
                current -= step;
                terms.push_back(current);
            }
            int correct = current - step;
            int wrong1 = current - step - 1;
            int wrong2 = current - std::max(1, step - 1);
            int wrong3 = randomNear(correct, rng);

            std::string expl = "Correct! Subtract " + std::to_string(step) + " each time; " + std::to_string(current) + " minus " + std::to_string(step) + " equals " + std::to_string(correct) + ".";
            return finalizeQuestion(Category::Subtraction, terms, correct, wrong1, wrong2, wrong3, rng, expl);
        }
        case 1: {
            // Decreasing differences
            int start = randomInRange(120, 200, rng);
            int firstStep = randomInRange(6, 14, rng);

            std::vector<int> terms{ start };
            int current = start;
            int step = firstStep;
            int lastUsedStep = step;
            for (int i = 0; i < 3; ++i) {
                current -= step;
                terms.push_back(current);
                lastUsedStep = step;
                step = std::max(1, step - 1);
            }

            int correct = current - step;
            int wrong1 = current - lastUsedStep; // keeps last difference constant
            int wrong2 = current - std::max(1, lastUsedStep - 1);
            int wrong3 = randomNear(correct, rng);

            std::string expl = "Correct! Differences shrink by 1: last was minus " + std::to_string(lastUsedStep) + ", next is minus " + std::to_string(step) + ", so " + std::to_string(current) + " minus " + std::to_string(step) + " equals " + std::to_string(correct) + ".";
            return finalizeQuestion(Category::Subtraction, terms, correct, wrong1, wrong2, wrong3, rng, expl);
        }
        case 2:
        default: {
            // Alternating differences (s1, s2, s1, s2, next s1)
            int s1 = randomInRange(6, 18, rng);
            int s2 = randomInRange(3, 14, rng);
            int totalNeeded = s1 + s2 + s1 + s2 + s1 + 10;
            int start = randomInRange(totalNeeded + 40, totalNeeded + 120, rng);

            std::vector<int> terms{ start };
            int current = start;
            std::array<int, 4> steps{ s1, s2, s1, s2 };
            for (int s : steps) {
                current -= s;
                terms.push_back(current);
            }
            int correct = current - s1;
            int wrong1 = current - s2; // repeat last subtraction
            int wrong2 = current - std::max(1, (s1 + s2) / 2); // averaged
            int wrong3 = randomNear(correct, rng);
            std::string expl = "Correct! Subtractions alternate minus " + std::to_string(s1) + " then minus " + std::to_string(s2) + "; after minus " + std::to_string(s2) + " comes minus " + std::to_string(s1) + ", so " + std::to_string(current) + " minus " + std::to_string(s1) + " equals " + std::to_string(correct) + ".";
            return finalizeQuestion(Category::Subtraction, terms, correct, wrong1, wrong2, wrong3, rng, expl);
        }
    }
}

} // namespace

std::vector<Question> generateNumberQuiz(std::mt19937& rng) {
    std::vector<Category> categories{
        Category::Multiplication,
        Category::Division,
        Category::Addition,
        Category::Subtraction
    };
    std::shuffle(categories.begin(), categories.end(), rng);

    std::vector<Question> questions;
    questions.reserve(categories.size());

    for (Category cat : categories)
        questions.push_back(generateQuestion(cat, rng));

    return questions;
}

Question generateQuestion(Category category, std::mt19937& rng) {
    int variant = randomInRange(0, 2, rng); // 3 variants per category
    switch (category) {
        case Category::Multiplication:
            return makeMultiplication(variant, rng);
        case Category::Division:
            return makeDivision(variant, rng);
        case Category::Addition:
            return makeAddition(variant, rng);
        case Category::Subtraction:
            return makeSubtraction(variant, rng);
        case Category::Name:
        default:
            return makeAddition(variant, rng); // fallback, shouldn't occur here
    }
}

std::string toString(Category category) {
    switch (category) {
        case Category::Name: return "Name";
        case Category::Multiplication: return "Multiplication";
        case Category::Division: return "Division";
        case Category::Addition: return "Addition";
        case Category::Subtraction: return "Subtraction";
        case Category::Silly: return "Silly";
        default: return "Unknown";
    }
}

} // namespace quiz
