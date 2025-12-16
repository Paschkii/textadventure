#pragma once
// === C++ Libraries ===
#include <cstddef>  // Exposes std::size_t for the sampling API.
#include <random>   // Accepts an external RNG reference for deterministic sampling.
#include <vector>   // Returns a collection of generated quiz questions.

#include "ui/quizGenerator.hpp" // Reuses the shared Question definition.

namespace sillyRiddles {

// Returns up to |count| freshly randomized silly riddles.
std::vector<quiz::Question> sample(std::mt19937& rng, std::size_t count);

} // namespace sillyRiddles
