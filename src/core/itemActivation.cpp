#include "core/itemActivation.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>


namespace core {
namespace itemActivation {

namespace {

constexpr std::array<const char*, 4> kElements = {
    "air",
    "earth",
    "fire",
    "water"
};

constexpr std::array<const char*, 5> kEmblemKeys = {
    "emblem_soul",
    "emblem_body",
    "emblem_resolve",
    "emblem_mind",
    "emblem_ascension"
};

constexpr std::array<const char*, 5> kTrophyKeys = {
    "dragoncup_air",
    "dragoncup_earth",
    "dragoncup_fire",
    "dragoncup_water",
    "dragoncup_umbra"
};

std::optional<std::size_t> artifactTypeIndex(std::string_view key) {
    auto delim = key.find('_');
    if (delim == std::string_view::npos)
        return std::nullopt;
    std::string_view prefix = key.substr(0, delim);
    if (prefix == "dragonscale")
        return 0;
    if (prefix == "dragonstone")
        return 1;
    if (prefix == "dragonclaw")
        return 2;
    if (prefix == "luckycharm")
        return 3;
    return std::nullopt;
}

std::optional<std::size_t> artifactElementIndex(std::string_view key) {
    auto delim = key.find_last_of('_');
    if (delim == std::string_view::npos)
        return std::nullopt;
    std::string_view suffix = key.substr(delim + 1);
    for (std::size_t index = 0; index < kElements.size(); ++index) {
        if (suffix == kElements[index])
            return index;
    }
    return std::nullopt;
}

template <std::size_t Count>
std::optional<std::size_t> ceremonialIndex(
    std::string_view key,
    const std::array<const char*, Count>& keys
) {
    for (std::size_t idx = 0; idx < keys.size(); ++idx) {
        if (key == keys[idx])
            return idx;
    }
    return std::nullopt;
}

const sf::Texture* textureForItemKeyImpl(const Game& game, std::string_view key) noexcept {
    if (key == "holmabir")
        return &game.resources.weaponHolmabir;
    if (key == "kattkavar")
        return &game.resources.weaponKattkavar;
    if (key == "stiggedin")
        return &game.resources.weaponStiggedin;
    if (key == "weapon_holmabir_broken")
        return &game.resources.weaponHolmabirBroken;
    if (key == "weapon_kattkavar_broken")
        return &game.resources.weaponKattkavarBroken;
    if (key == "weapon_stiggedin_broken")
        return &game.resources.weaponStiggedinBroken;
    if (key == "dragonstone_air")
        return &game.resources.dragonstoneAir;
    if (key == "dragonstone_earth")
        return &game.resources.dragonstoneEarth;
    if (key == "dragonstone_fire")
        return &game.resources.dragonstoneFire;
    if (key == "dragonstone_water")
        return &game.resources.dragonstoneWater;
    if (key == "dragonscale_air")
        return &game.resources.artifactDragonscaleAir;
    if (key == "dragonscale_earth")
        return &game.resources.artifactDragonscaleEarth;
    if (key == "dragonscale_fire")
        return &game.resources.artifactDragonscaleFire;
    if (key == "dragonscale_water")
        return &game.resources.artifactDragonscaleWater;
    if (key == "dragonclaw_air")
        return &game.resources.artifactDragonclawAir;
    if (key == "dragonclaw_earth")
        return &game.resources.artifactDragonclawEarth;
    if (key == "dragonclaw_fire")
        return &game.resources.artifactDragonclawFire;
    if (key == "dragonclaw_water")
        return &game.resources.artifactDragonclawWater;
    if (key == "luckycharm_air")
        return &game.resources.artifactLuckyCharmAir;
    if (key == "luckycharm_earth")
        return &game.resources.artifactLuckyCharmEarth;
    if (key == "luckycharm_fire")
        return &game.resources.artifactLuckyCharmFire;
    if (key == "luckycharm_water")
        return &game.resources.artifactLuckyCharmWater;
    if (key == "ring_fire")
        return &game.resources.equipmentRingFire;
    if (key == "ring_air")
        return &game.resources.equipmentRingAir;
    if (key == "ring_water")
        return &game.resources.equipmentRingWater;
    if (key == "ring_earth")
        return &game.resources.equipmentRingEarth;
    if (key == "dragoncup_air")
        return &game.resources.trophyDragoncupAir;
    if (key == "dragoncup_earth")
        return &game.resources.trophyDragoncupEarth;
    if (key == "dragoncup_fire")
        return &game.resources.trophyDragoncupFire;
    if (key == "dragoncup_water")
        return &game.resources.trophyDragoncupWater;
    if (key == "dragoncup_umbra")
        return &game.resources.trophyDragoncupUmbra;
    if (key == "emblem_soul")
        return &game.resources.emblemSoul;
    if (key == "emblem_body")
        return &game.resources.emblemBody;
    if (key == "emblem_resolve")
        return &game.resources.emblemResolve;
    if (key == "emblem_mind")
        return &game.resources.emblemMind;
    if (key == "emblem_ascension")
        return &game.resources.emblemAscension;
    if (key == "map_glandular")
        return &game.resources.mapGlandular;
    if (key == "umbra_ussea_one")
        return &game.resources.umbraUsseaOne;
    if (key == "umbra_ussea_two")
        return &game.resources.umbraUsseaTwo;
    if (key == "umbra_ussea_three")
        return &game.resources.umbraUsseaThree;
    if (key == "umbra_ussea_four")
        return &game.resources.umbraUsseaFour;
    if (key == "umbra_ussea_complete")
        return &game.resources.umbraUsseaComplete;
    if (key == "heal_potion")
        return &game.resources.healPotion;
    return nullptr;
}

bool equipArtifactImpl(Game& game, const std::string& key) {
    auto typeIndex = artifactTypeIndex(key);
    auto elementIndex = artifactElementIndex(key);
    if (!typeIndex || !elementIndex)
        return false;
    auto& activeSlot = game.artifactSlots.slots[*typeIndex][*elementIndex];
    if (activeSlot && *activeSlot == key)
        return false;
    if (!game.itemController.removeItem(key))
        return false;
    if (activeSlot) {
        if (const sf::Texture* oldTexture = textureForItemKeyImpl(game, *activeSlot))
            game.itemController.addItem(*oldTexture, *activeSlot);
    }
    activeSlot = key;
    return true;
}

bool equipWeaponImpl(Game& game, const std::string& key) {
    const sf::Texture* left = nullptr;
    const sf::Texture* right = nullptr;
    if (key == "stiggedin") {
        left = &game.resources.weaponStiggedinLeft;
        right = &game.resources.weaponStiggedin;
    }
    else if (key == "kattkavar") {
        left = &game.resources.weaponKattkavarLeft;
        right = &game.resources.weaponKattkavar;
    }
    else if (key == "holmabir") {
        left = &game.resources.weaponHolmabir;
        right = &game.resources.weaponHolmabir;
    }
    if (!left && !right)
        return false;
    game.equippedWeapons.leftHand = left;
    game.equippedWeapons.rightHand = right;
    game.equippedWeapons.leftKey = key;
    game.equippedWeapons.rightKey = key;
    game.itemController.removeItem(key);
    return true;
}

bool equipRingImpl(Game& game, const std::string& key) {
    auto& slots = game.ringEquipment.slots;
    if (std::find(slots.begin(), slots.end(), key) != slots.end())
        return false;
    if (game.ringEquipment.nextSlotIndex >= slots.size())
        return false;
    slots[game.ringEquipment.nextSlotIndex] = key;
    ++game.ringEquipment.nextSlotIndex;
    return true;
}

bool equipCeremonialImpl(Game& game, std::string_view key) {
    if (auto idx = ceremonialIndex(key, kEmblemKeys)) {
        if (game.emblemSlots[*idx] && *game.emblemSlots[*idx] == key)
            return false;
        game.emblemSlots[*idx] = std::string(key);
        return true;
    }
    if (auto idx = ceremonialIndex(key, kTrophyKeys)) {
        if (game.trophySlots[*idx] && *game.trophySlots[*idx] == key)
            return false;
        game.trophySlots[*idx] = std::string(key);
        return true;
    }
    return false;
}

} // namespace

const sf::Texture* textureForItemKey(const Game& game, const std::string& key) noexcept {
    return textureForItemKeyImpl(game, key);
}

bool activateItem(Game& game, const std::string& key) {
    if (key.find("broken") != std::string::npos)
        return false;
    if (key.rfind("ring_", 0) == 0)
        return equipRingImpl(game, key);
    if (artifactTypeIndex(key) && artifactElementIndex(key))
        return equipArtifactImpl(game, key);
    if (equipCeremonialImpl(game, key)) {
        game.itemController.playAcquireSound();
        return true;
    }
    return equipWeaponImpl(game, key);
}

} // namespace itemActivation
} // namespace core
