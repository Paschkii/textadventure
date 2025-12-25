#pragma once

#include <array>

namespace ResourceFiles {
namespace Items {
    inline constexpr const char* kHealPotionTexture = "assets/gfx/items/healpotion.png";
    inline constexpr const char* kHealPotionSound = "assets/audio/heal.mp3";
    inline constexpr std::array<const char*, 1> kItemTextures{ {
        kHealPotionTexture
    } };
    inline constexpr std::array<const char*, 1> kItemSounds{ {
        kHealPotionSound
    } };
}
}
