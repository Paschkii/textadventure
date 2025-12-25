// === Header Files ===
#include "story/locationDialogues.hpp" // Declares the lookup exposed by this translation unit.
#include "story/storyIntro.hpp"       // Provides the dialogue data tables returned here.

namespace story {

// Returns the dialogue table used when arriving at the given location.
const std::vector<DialogueLine>* locationDialogueFor(LocationId id) {
    switch (id) {
        case LocationId::Blyathyroid:
            return &firedragon;
        case LocationId::Aerobronchi:
            return &airdragon;
        case LocationId::Lacrimere:
            return &waterdragon;
        case LocationId::Cladrenal:
            return &earthdragon;
        case LocationId::Gonad:
            return &gonad;
        default:
            return nullptr;
    }
}

} // namespace story
