#pragma once
// === C++ Libraries ===
#include <array>
#include <vector>
#include <optional>
// === SFML Libraries ===
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
// === Header Files ===
#include "rendering/locations.hpp"
#include "resources/resources.hpp"

namespace core {

// Represents one collected icon sprite shown in the inventory panel.
struct ItemIcon {
    explicit ItemIcon(const sf::Texture& tex) : sprite(tex) {}
    ItemIcon(ItemIcon&&) noexcept = default;
    ItemIcon& operator=(ItemIcon&&) noexcept = default;
    ItemIcon(const ItemIcon&) = delete;
    ItemIcon& operator=(const ItemIcon&) = delete;

    sf::Sprite sprite;
};

// Tracks collected items, renders their icons, and plays acquisition sounds.
class ItemController {
public:
    ItemController() = default;

    // Hooks the item controller up to shared textures and sounds.
    void init(Resources& resources);
    // Adds a generic icon and plays the pickup jingle.
    void addIcon(const sf::Texture& texture);
    // Adds the dragonstone icon matching the given location once.
    void collectDragonstone(LocationId id);

    std::vector<ItemIcon>& icons() noexcept { return icons_; }
    const std::vector<ItemIcon>& icons() const noexcept { return icons_; }

private:
    // Plays the sound that accompanies any new icon.
    void playAcquireSound();
    std::vector<ItemIcon> icons_;                         // Icons shown in the UI.
    std::array<bool, 4> dragonstoneIconAdded_{};          // Tracks which dragonstones were already added.
    std::optional<sf::Sound> acquireSound_;               // Sound played when collecting an icon.
    std::array<const sf::Texture*, 4> dragonstoneTextures_{}; // Cached dragonstone textures from resources.
};

} // namespace core
