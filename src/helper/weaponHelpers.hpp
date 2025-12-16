#pragma once
// === Header Files ===
#include "core/game.hpp"  // Exposes Game so the helpers can modify weapon options and panel layout.

namespace ui {
namespace weapons {
    // Loads weapon textures and metadata from the assets folder.
    void loadWeaponOptions(Game& game);
    // Positions/scales the weapon sprites inside the selection panel.
    void layoutWeaponSelection(Game& game);
}
}
