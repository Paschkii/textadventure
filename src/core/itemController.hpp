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
    ItemIcon(const sf::Texture& tex, std::string key)
        : sprite(tex), key(std::move(key)) {}
    ItemIcon(ItemIcon&&) noexcept = default;
    ItemIcon& operator=(ItemIcon&&) noexcept = default;
    ItemIcon(const ItemIcon&) = delete;
    ItemIcon& operator=(const ItemIcon&) = delete;

    sf::Sprite sprite;
    std::string key;
};

// Tracks collected items, renders their icons, and plays acquisition sounds.
class ItemController {
public:
    ItemController() = default;

    // Hooks the item controller up to shared textures and sounds.
    void init(Resources& resources);
    // Adds a specific item so it can be looked up by key later.
    void addItem(const sf::Texture& texture, std::string key);
    // Removes the first icon that matches the provided key.
    bool removeItem(const std::string& key);
    // Adds the dragonstone icon matching the given location once.
    void collectDragonstone(LocationId id);
    // Plays the sound that accompanies any new icon.
    void playAcquireSound();

    std::vector<ItemIcon>& icons() noexcept { return icons_; }
    const std::vector<ItemIcon>& icons() const noexcept { return icons_; }

private:
    std::vector<ItemIcon> icons_;                         // Icons shown in the UI.
    std::array<bool, 4> dragonstoneIconAdded_{};          // Tracks which dragonstones were already added.
    std::optional<sf::Sound> acquireSound_;               // Sound played when collecting an icon.
    std::array<const sf::Texture*, 4> dragonstoneTextures_{}; // Cached dragonstone textures from resources.
};

} // namespace core
