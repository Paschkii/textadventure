#pragma once
// === Header Files ===
#include "core/game.hpp"  // Supplies the Game structure whose UI boxes this helper repositions.

namespace ui {
namespace layout {
    // Update layout of top-level UI elements (nameBox, textBox, locationBox, itemBox, weaponPanel)
    // Implementation: src/helper/layoutHelpers.cpp
    void updateLayout(Game& game);
}
}
