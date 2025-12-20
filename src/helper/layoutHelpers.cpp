// === C++ Libraries ===
#include <algorithm>  // Uses std::max when computing panel widths and positioning elements.
#include <stdexcept>  // Catches texture loading errors so we can try alternate asset paths.
#include <string>     // Builds fallback asset path strings when the main copy is missing.
// === Header Files ===
#include "core/game.hpp"  // Accesses UI boxes, weapon/dragon lists, and window size info.
#include "helper/weaponHelpers.hpp"  // Invokes weapon layout helpers at the end of updateLayout.
#include "helper/dragonHelpers.hpp"  // Relies on dragon layout helpers for portrait positioning.
#include "helper/layoutHelpers.hpp"  // Declares updateLayout implemented in this translation unit.

namespace ui {
namespace layout {
namespace {
// Positions each collected item icon vertically inside the item box.
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

// Updates all major UI boxes, reloads the frame, and triggers weapon/dragon layouts.
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

    // === NameBox: links unten, 5% Abstand ===
    game.nameBox.setSize({ nameWidth, boxHeight });
    game.nameBox.setPosition({
        marginX,
        h - boxHeight - marginY
    });

    constexpr float kPlayerStatusBarHeight = 16.f;
    constexpr float kPlayerStatusVerticalPadding = 8.f;
    constexpr float kPlayerStatusRowSpacing = 5.f;
    constexpr float kPlayerStatusBoxHeight = (kPlayerStatusBarHeight * 2.f) + (kPlayerStatusVerticalPadding * 2.f) + kPlayerStatusRowSpacing;
    float statusWidth = std::max(0.f, game.nameBox.getSize().x * 0.9f);
    float statusX = game.nameBox.getPosition().x + (game.nameBox.getSize().x - statusWidth) * 0.5f;
    float statusY = game.nameBox.getPosition().y - kPlayerStatusBoxHeight - 10.f;
    game.playerStatusBox.setSize({ statusWidth, kPlayerStatusBoxHeight });
    game.playerStatusBox.setPosition({ statusX, statusY });

    // === TextBox: rechts unten, 5% Abstand ===
    game.textBox.setSize({ textWidth - (marginX / 2), boxHeight });
    game.textBox.setPosition({
        marginX + game.nameBox.getSize().x + (marginX / 2),
        h - boxHeight - marginY
    });

    game.locationBox.setSize({ locationWidth, boxHeight / 2 });
    game.locationBox.setPosition({
        marginX + 30.f,
        marginY
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
    try {
        game.uiFrame.load("assets/textures/boxborder.png");
    }
    catch (const std::runtime_error&) {
        game.uiFrame.load(std::string(TEXTADVENTURE_ASSET_DIR) + "/textures/boxborder.png");
    }

    // Layout the UI-specific elements (weapon / dragon)
    ui::weapons::layoutWeaponSelection(game);
    ui::dragons::layoutDragonPortraits(game);

    float buttonTarget = std::clamp(w * 0.08f, 48.f, 72.f);
    auto texSize = game.resources.menuButton.getSize();
    float maxDim = static_cast<float>(std::max(texSize.x, texSize.y));
    float uniformScale = (maxDim > 0.f) ? buttonTarget / maxDim : 1.f;
    sf::Vector2f finalSize{
        static_cast<float>(texSize.x) * uniformScale,
        static_cast<float>(texSize.y) * uniformScale
    };
    finalSize.x *= 0.5f;
    finalSize.y *= 0.5f;
    float textBoxRight = game.textBox.getPosition().x + game.textBox.getSize().x;
    float buttonX = textBoxRight - finalSize.x - 8.f;
    float buttonY = game.textBox.getPosition().y - finalSize.y - 12.f;
    game.menuButton.setSize(finalSize);
    game.menuButton.setPosition({ buttonX, buttonY });

    constexpr float marginPercent = 0.05f;
    float horizontalMargin = w * marginPercent;
    float verticalMargin = h * marginPercent;

    constexpr float kTabHeight = 70.f;
    constexpr float kTabSpacing = 12.f;
    float tabAreaWidth = (w - horizontalMargin * 2.f - kTabSpacing * 5.f) / 6.f;
    float tabY = verticalMargin + 8.f;
    for (std::size_t idx = 0; idx < game.menuTabBounds.size(); ++idx) {
        float x = horizontalMargin + static_cast<float>(idx) * (tabAreaWidth + kTabSpacing);
        game.menuTabBounds[idx] = sf::FloatRect({ x, tabY }, { tabAreaWidth, kTabHeight });
    }

    float innerX = horizontalMargin;
    float innerY = tabY + kTabHeight + 16.f;
    float innerWidth = w - (horizontalMargin * 2.f);
    float innerHeight = h - innerY - verticalMargin;
    game.menuPanel.setPosition(sf::Vector2f{ innerX, innerY });
    game.menuPanel.setSize({ innerWidth, innerHeight });
}

} // namespace layout
} // namespace ui
