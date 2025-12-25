#pragma once
// === Header Files ===
#include "core/game.hpp"  // Accesses Game so portraits and layout helpers can mutate its sprite lists.

namespace ui {
namespace dragons {
    // Loads portrait sprites and names from the shared resources into Game.
    void loadDragonPortraits(Game& game);
    // Positions the dragon portraits evenly inside the weapon panel.
    void layoutDragonPortraits(Game& game);
}
}
