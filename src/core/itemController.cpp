// === C++ Libraries ===
#include <algorithm>
// === Header Files ===
#include "core/itemController.hpp"

namespace {
// Converts certain map locations into dragonstone indices for icon lookup.
std::optional<std::size_t> mapLocationToDragonstoneIndex(LocationId id) {
    switch (id) {
        case LocationId::Blyathyroid: return 0;
        case LocationId::Aerobronchi: return 1;
        case LocationId::Lacrimere: return 2;
        case LocationId::Cladrenal: return 3;
        default: return std::nullopt;
    }
}
} // namespace

namespace core {

// Initializes textures and sounds using the shared resources bundle.
void ItemController::init(Resources& resources) {
    acquireSound_.emplace(resources.acquire);
    dragonstoneTextures_[0] = &resources.dragonstoneFire;
    dragonstoneTextures_[1] = &resources.dragonstoneAir;
    dragonstoneTextures_[2] = &resources.dragonstoneWater;
    dragonstoneTextures_[3] = &resources.dragonstoneEarth;
}

// Adds a new icon sprite and plays the pickup sound.
void ItemController::addIcon(const sf::Texture& texture) {
    icons_.emplace_back(texture);
    playAcquireSound();
}

// Adds the dragonstone icon corresponding to the given location if not already present.
void ItemController::collectDragonstone(LocationId id) {
    auto index = mapLocationToDragonstoneIndex(id);
    if (!index)
        return;

    if (*index >= dragonstoneIconAdded_.size())
        return;

    if (dragonstoneIconAdded_[*index])
        return;

    const sf::Texture* texture = dragonstoneTextures_[*index];
    if (!texture)
        return;

    dragonstoneIconAdded_[*index] = true;
    addIcon(*texture);
}

// Plays (and restarts) the acquire sound effect when items are collected.
void ItemController::playAcquireSound() {
    if (!acquireSound_)
        return;

    acquireSound_->stop();
    acquireSound_->play();
}

} // namespace core
