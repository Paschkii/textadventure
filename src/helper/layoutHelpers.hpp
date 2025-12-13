#pragma once

#include "core/game.hpp"

namespace ui {
namespace layout {
    // Update layout of top-level UI elements (nameBox, textBox, locationBox, itemBox, weaponPanel)
    // Implementation: src/helper/layoutHelpers.cpp
    void updateLayout(Game& game);
}
}
