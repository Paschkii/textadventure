#include "helper/healingPotion.hpp"

#include "core/game.hpp"

#include <algorithm>

namespace helper {
namespace healingPotion {
namespace {
constexpr float kHealingDuration = 4.f;

bool beginHealing(Game& game, float startHp) {
    if (startHp >= game.playerHpMax) {
        game.playerHp = game.playerHpMax;
        return false;
    }

    game.healingPotionStartHp = startHp;
    game.healingPotionClock.restart();
    game.healingPotionActive = true;
    if (!game.healPotionSound)
        game.healPotionSound.emplace(game.resources.healSound);
    else
        game.healPotionSound->setBuffer(game.resources.healSound);
    game.healPotionSound->play();
    return true;
}
}

bool start(Game& game) {
    if (game.healingPotionReceived)
        return false;

    game.healingPotionReceived = true;
    game.itemController.addItem(game.resources.healPotion, "heal_potion");

    float startHp = std::clamp(game.playerHp, 0.f, game.playerHpMax);
    return beginHealing(game, startHp);
}

bool startEmergency(Game& game) {
    float startHp = std::clamp(game.playerHp, 0.f, game.playerHpMax);
    const bool started = beginHealing(game, startHp);
    if (started)
        game.emergencyHealingActive = true;
        ++game.emergencyHealCount;
    return started;
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
        game.emergencyHealingActive = false;
    }
}

} // namespace healingPotion
} // namespace helper
