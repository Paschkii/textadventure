#include "helper/healingPotion.hpp"

#include "core/game.hpp"

#include <algorithm>

namespace helper {
namespace healingPotion {
namespace {
constexpr float kHealingDuration = 4.f;
}

bool start(Game& game) {
    if (game.healingPotionReceived)
        return false;

    game.healingPotionReceived = true;
    game.itemController.addIcon(game.resources.healPotion);

    float startHp = std::clamp(game.playerHp, 0.f, game.playerHpMax);
    game.healingPotionStartHp = startHp;
    if (startHp >= game.playerHpMax) {
        game.playerHp = game.playerHpMax;
        return false;
    }

    game.healingPotionClock.restart();
    game.healingPotionActive = true;
    if (!game.healPotionSound)
        game.healPotionSound.emplace(game.resources.healSound);
    else
        game.healPotionSound->setBuffer(game.resources.healSound);
    game.healPotionSound->play();

    return true;
}

void update(Game& game) {
    if (!game.healingPotionActive)
        return;

    float elapsed = game.healingPotionClock.getElapsedTime().asSeconds();
    float progress = std::min(elapsed / kHealingDuration, 1.f);
    game.playerHp = game.healingPotionStartHp + ((game.playerHpMax - game.healingPotionStartHp) * progress);
    if (progress >= 1.f) {
        game.playerHp = game.playerHpMax;
        game.healingPotionActive = false;
    }
}

} // namespace healingPotion
} // namespace helper
