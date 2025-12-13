#include "helper/dragonHelpers.hpp"
#include <algorithm>

namespace ui {
namespace dragons {

void loadDragonPortraits(Game& game) {
    game.dragonPortraits.clear();

    struct PortraitInfo {
        const sf::Texture* texture;
        std::string name;
    };

    const std::vector<PortraitInfo> portraits = {
        { &game.resources.fireDragon, "Rowsted Sheacane" },
        { &game.resources.waterDragon, "Flawtin Seamen" },
        { &game.resources.earthDragon, "Grounded Claymore" },
        { &game.resources.airDragon, "Gustavo Windimaess" }
    };

    game.dragonPortraits.reserve(portraits.size());

    for (const auto& portrait : portraits) {
        if (!portrait.texture)
            continue;

        game.dragonPortraits.emplace_back(*portrait.texture, portrait.name);
    }
}

void layoutDragonPortraits(Game& game) {
    if (game.dragonPortraits.empty())
        return;

    constexpr float padding = 24.f;
    constexpr float labelHeight = 32.f;

    float availableWidth = game.weaponPanel.getSize().x - (padding * 2.f);
    float availableHeight = game.weaponPanel.getSize().y - (padding * 2.f) - labelHeight;

    if (availableWidth <= 0.f || availableHeight <= 0.f)
        return;

    float slotWidth = availableWidth / static_cast<float>(game.dragonPortraits.size());
    float spriteAreaHeight = availableHeight;

    for (std::size_t i = 0; i < game.dragonPortraits.size(); ++i) {
        auto& entry = game.dragonPortraits[i];
        float slotCenterX = game.weaponPanel.getPosition().x + padding + (slotWidth * (static_cast<float>(i) + 0.5f));
        float spriteCenterY = game.weaponPanel.getPosition().y + padding + (spriteAreaHeight * 0.5f);

        const sf::Texture& texture = entry.sprite.getTexture();

        entry.sprite.setScale({ 1.f, 1.f });
        auto texSize = texture.getSize();
        float scaleX = (slotWidth * 0.7f) / static_cast<float>(texSize.x);
        float scaleY = (spriteAreaHeight * 0.8f) / static_cast<float>(texSize.y);
        float scale = std::min(scaleX, scaleY);
        entry.baseScale = scale;
        entry.sprite.setScale({ scale, scale });

        auto localBounds = entry.sprite.getLocalBounds();
        entry.sprite.setOrigin({
            localBounds.position.x + (localBounds.size.x / 2.f),
            localBounds.position.y + (localBounds.size.y / 2.f)
        });
        entry.centerPosition = { slotCenterX, spriteCenterY };
        entry.sprite.setPosition(entry.centerPosition);
        entry.bounds = entry.sprite.getGlobalBounds();

        entry.labelPosition = {
            slotCenterX,
            game.weaponPanel.getPosition().y + padding + spriteAreaHeight + 4.f
        };
    }
}

} // namespace dragons
} // namespace ui
