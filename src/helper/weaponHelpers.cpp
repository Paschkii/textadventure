// === C++ Libraries ===
#include <filesystem>  // Iterates over the weapon texture directory when loading options.
#include <algorithm>    // Uses std::sort and std::transform while preparing weapon data.
#include <cctype>       // Applies std::tolower for case-insensitive hotkey detection.
// === Header Files ===
#include "helper/weaponHelpers.hpp"  // Declares the helpers implemented in this file.

namespace ui {
namespace weapons {

namespace {
    // Returns a lowercase copy for hotkey detection.
    std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }
}

// Clears existing options and reloads each weapon from the texture assets folder.
void loadWeaponOptions(Game& game) {
    namespace fs = std::filesystem;

    const fs::path weaponDir{"assets/gfx/weapons"};
    game.weaponOptions.clear();

    if (!fs::exists(weaponDir))
        return;

    std::vector<fs::path> weaponFiles;
    for (const auto& entry : fs::directory_iterator(weaponDir)) {
        if (!entry.is_regular_file())
            continue;

        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (ext != ".png" && ext != ".jpg" && ext != ".jpeg")
            continue;

        weaponFiles.push_back(entry.path());
    }

    std::sort(weaponFiles.begin(), weaponFiles.end());

    game.weaponOptions.reserve(weaponFiles.size());

    const std::string prefix = "Weapon ";
    for (const auto& path : weaponFiles) {
        game.weaponOptions.emplace_back();
        auto& option = game.weaponOptions.back();

        if (!option.texture.loadFromFile(path.string())) {
            game.weaponOptions.pop_back();
            continue;
        }

        std::string stem = path.stem().string();
        auto prefixPos = stem.find(prefix);
        if (prefixPos != std::string::npos)
            stem = stem.substr(prefixPos + prefix.size());

        while (!stem.empty() && stem.front() == ' ')
            stem.erase(stem.begin());

        option.displayName = stem;

        std::string lowerName = toLower(option.displayName);
        if (lowerName == "holmabir") {
            option.hotkeyNumber = 1;
        } else if (lowerName == "kattkavar") {
            option.hotkeyNumber = 2;
        } else if (lowerName == "stiggedin") {
            option.hotkeyNumber = 3;
        }
        
        option.sprite.emplace(option.texture);
        option.sprite->setTexture(option.texture);
    }
}

// Computes sizes/positions for each weapon sprite so they fit in the panel.
void layoutWeaponSelection(Game& game) {
    if (game.weaponOptions.empty()) {
        game.hoveredWeaponIndex = -1;
        game.selectedWeaponIndex = -1;
        return;
    }

    constexpr float padding = 24.f;
    constexpr float labelHeight = 32.f;

    float availableWidth = game.weaponPanel.getSize().x - (padding * 2.f);
    float availableHeight = game.weaponPanel.getSize().y - (padding * 2.f) - labelHeight;

    if (availableWidth <= 0.f || availableHeight <= 0.f)
        return;

    float slotWidth = availableWidth / static_cast<float>(game.weaponOptions.size());
    float spriteAreaHeight = availableHeight;

    for (std::size_t i = 0; i < game.weaponOptions.size(); ++i) {
        auto& option = game.weaponOptions[i];

        float slotCenterX = game.weaponPanel.getPosition().x + padding + (slotWidth * (static_cast<float>(i) + 0.5f));
        float spriteCenterY = game.weaponPanel.getPosition().y + padding + (spriteAreaHeight * 0.5f);

        if (!option.sprite)
            continue;

        auto& sprite = *option.sprite;
        sprite.setScale({ 1.f, 1.f });
        auto texSize = option.texture.getSize();
        float scaleX = (slotWidth * 0.7f) / static_cast<float>(texSize.x);
        float scaleY = (spriteAreaHeight * 0.8f) / static_cast<float>(texSize.y);
        float scale = std::min(scaleX, scaleY);
        sprite.setScale({ scale, scale });

        auto localBounds = sprite.getLocalBounds();
        sprite.setOrigin({
            localBounds.position.x + (localBounds.size.x / 2.f),
            localBounds.position.y + (localBounds.size.y / 2.f)
        });
        sprite.setPosition({ slotCenterX, spriteCenterY });
        option.bounds = sprite.getGlobalBounds();

        option.labelPosition = {
            slotCenterX,
            game.weaponPanel.getPosition().y + padding + spriteAreaHeight + 4.f
        };
    }
}

} // namespace weapons
} // namespace ui
