#include "helper/layoutHelpers.hpp"
#include "core/game.hpp"
#include <algorithm>
#include "helper/weaponHelpers.hpp"
#include "helper/dragonHelpers.hpp"

namespace ui {
namespace layout {
namespace {
void layoutItemIcons(Game& game) {
    auto& icons = game.itemController.icons();
    if (icons.empty())
        return;

    constexpr std::size_t kMaxItems = 5;
    constexpr float padding = 12.f;

    float innerWidth = game.itemBox.getSize().x - (padding * 2.f);
    float innerHeight = game.itemBox.getSize().y - (padding * 2.f);
    if (innerWidth <= 0.f || innerHeight <= 0.f)
        return;

    float slotHeight = innerHeight / static_cast<float>(kMaxItems);

    for (std::size_t i = 0; i < icons.size(); ++i) {
        auto& item = icons[i];
        const sf::Texture& tex = item.sprite.getTexture();
        item.sprite.setScale({ 1.f, 1.f });

        auto texSize = tex.getSize();
        float scaleX = (innerWidth * 0.8f) / static_cast<float>(texSize.x);
        float scaleY = (slotHeight * 0.8f) / static_cast<float>(texSize.y);
        float scale = std::min(scaleX, scaleY);
        item.sprite.setScale({ scale, scale });

        auto localBounds = item.sprite.getLocalBounds();
        item.sprite.setOrigin({
            localBounds.position.x + (localBounds.size.x / 2.f),
            localBounds.position.y + (localBounds.size.y / 2.f)
        });

        sf::Vector2f center{
            game.itemBox.getPosition().x + padding + (innerWidth / 2.f),
            game.itemBox.getPosition().y + padding + (slotHeight * (static_cast<float>(i) + 0.5f))
        };
        item.sprite.setPosition(center);
    }
}
} // namespace

void updateLayout(Game& game) {
    float w = game.window.getSize().x;
    float h = game.window.getSize().y;

    float marginX = w * 0.05f;      // 5% vom Rand (64)
    float marginY = h * 0.05f;      // 5% vom Rand (36)

    float boxHeight = h * 0.25f;    // beide Boxen: 25% Höhe (180)
    float nameWidth = w * 0.20f;    // NameBox: 20% Breite (256)
    float textWidth = w * 0.70f;    // TextBox: 70% Breite (896)
    float locationWidth = w * 0.20f; // LocationBox 30% Breite
    float itemWidth = (w * 0.20f) * 0.5f; // ItemBox: 50% der bisherigen Breite
    constexpr float kStatusBarHeight = 16.f;
    constexpr float kStatusBarPadding = 12.f;
    constexpr float kStatusRowSpacing = kStatusBarHeight + 12.f;
    float statusHeight = (kStatusRowSpacing + kStatusBarHeight) + (kStatusBarPadding * 2.f);

    // === NameBox: links unten, 5% Abstand ===
    game.nameBox.setSize({ nameWidth, boxHeight });
    game.nameBox.setPosition({
        marginX,
        h - boxHeight - marginY
    });

    // === TextBox: rechts unten, 5% Abstand ===
    game.textBox.setSize({ textWidth - (marginX / 2), boxHeight });
    game.textBox.setPosition({
        marginX + game.nameBox.getSize().x + (marginX / 2),
        h - boxHeight - marginY
    });

    game.locationBox.setSize({ locationWidth, boxHeight / 2 });
    game.locationBox.setPosition({
        marginX,
        marginY
    });

    // statusHeight already computed above
    constexpr float statusMargin = 12.f;
    game.playerStatusBox.setSize({ nameWidth, statusHeight });
    game.playerStatusBox.setPosition({
        marginX,
        game.nameBox.getPosition().y - statusHeight - statusMargin
    });

    // === ItemBox: rechts oben, gleicher Abstand zu Top/Right wie TextBox ===
    float itemHeight = boxHeight * 2.25f; // nochmals 50% höher als zuvor
    game.itemBox.setSize({ itemWidth, itemHeight });
    game.itemBox.setPosition({
        w - marginX - itemWidth,
        marginY
    });

    float weaponPanelHeight = game.nameBox.getSize().y * 0.9f;
    float weaponPanelWidth = (game.textBox.getPosition().x + game.textBox.getSize().x) - game.nameBox.getPosition().x - itemWidth - 20.f; // kürzer um ItemBox-Breite
    weaponPanelWidth = std::max(0.f, weaponPanelWidth);
    float weaponPanelY = game.playerStatusBox.getPosition().y - weaponPanelHeight - (marginY * 0.5f);

    game.weaponPanel.setSize({ weaponPanelWidth, weaponPanelHeight });
    game.weaponPanel.setPosition({ game.nameBox.getPosition().x, weaponPanelY });

    layoutItemIcons(game);

    // === 9-Slice-Textur laden ===
    game.uiFrame.load("assets/textures/boxborder.png");

    // Layout the UI-specific elements (weapon / dragon)
    ui::weapons::layoutWeaponSelection(game);
    ui::dragons::layoutDragonPortraits(game);
}

} // namespace layout
} // namespace ui
