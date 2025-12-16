#pragma once
// === C++ Libraries ===
#include <vector>                    // Returns vectors of DialogueLine for locations.
// === Header Files ===
#include "rendering/locations.hpp"   // Uses LocationId to select the proper dialogue.
#include "story/dialogueLine.hpp"    // Supplies DialogueLine definitions for each location.

namespace story {

// Retrieves the static dialogue vector associated with a map location.
const std::vector<DialogueLine>* locationDialogueFor(LocationId id);

} // namespace story
