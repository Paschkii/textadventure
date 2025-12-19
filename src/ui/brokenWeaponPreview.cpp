#include "ui/brokenWeaponPreview.hpp"

#include "core/game.hpp"
#include "helper/colorHelper.hpp"
#include "ui/uiEffects.hpp"

#include <array>
#include <algorithm>

namespace {
    constexpr float kPopupFadeInDuration = 0.45f;
    constexpr float kPopupFadeOutDuration = 1.f;
    constexpr float kWeaponFadeInDuration = 0.45f;
    constexpr float kNameAreaHeight = 32.f;
    constexpr float kMargin = 28.f;
    constexpr float kMinPopupWidth = 640.f;
    constexpr float kMaxPopupHeightRatio = 0.6f;
    constexpr float kNameFontSize = 22.f;

    struct WeaponInfo {
        const char* name;
    };

    constexpr std::array<WeaponInfo, 3> kWeapons{{
        { "Holmabir" },
        { "Kattkavar" },
        { "Stiggedin" }
    }};
}

namespace ui {
namespace brokenweapon {

void showPreview(Game& game) {
    auto& preview = game.brokenWeaponPopup;
    preview.phase = Game::BrokenWeaponPopup::Phase::PopupFadingIn;
    preview.fadeClock.restart();
    preview.popupAlpha = 0.f;
    preview.weaponAlpha = 0.f;
    preview.popupFadeStart = 0.f;
    preview.weaponFadeStart = 0.f;
}

void hidePreview(Game& game) {
    auto& preview = game.brokenWeaponPopup;
    if (preview.phase == Game::BrokenWeaponPopup::Phase::Hidden
        || preview.phase == Game::BrokenWeaponPopup::Phase::FadingOut)
    {
        return;
    }

    preview.popupFadeStart = preview.popupAlpha;
    preview.weaponFadeStart = preview.weaponAlpha;
    preview.phase = Game::BrokenWeaponPopup::Phase::FadingOut;
    preview.fadeClock.restart();
}

void updatePreview(Game& game) {
    auto& preview = game.brokenWeaponPopup;
    switch (preview.phase) {
        case Game::BrokenWeaponPopup::Phase::Hidden:
            return;
        case Game::BrokenWeaponPopup::Phase::PopupFadingIn: {
            float progress = std::min(preview.fadeClock.getElapsedTime().asSeconds() / kPopupFadeInDuration, 1.f);
            preview.popupAlpha = progress;
            preview.weaponAlpha = 0.f;
            if (progress >= 1.f) {
                preview.phase = Game::BrokenWeaponPopup::Phase::WeaponsFadingIn;
                preview.fadeClock.restart();
            }
            break;
        }
        case Game::BrokenWeaponPopup::Phase::WeaponsFadingIn: {
            float progress = std::min(preview.fadeClock.getElapsedTime().asSeconds() / kWeaponFadeInDuration, 1.f);
            preview.popupAlpha = 1.f;
            preview.weaponAlpha = progress;
            if (progress >= 1.f) {
                preview.phase = Game::BrokenWeaponPopup::Phase::Visible;
            }
            break;
        }
        case Game::BrokenWeaponPopup::Phase::Visible:
            preview.popupAlpha = 1.f;
            preview.weaponAlpha = 1.f;
            break;
        case Game::BrokenWeaponPopup::Phase::FadingOut: {
            float progress = std::min(preview.fadeClock.getElapsedTime().asSeconds() / kPopupFadeOutDuration, 1.f);
            preview.popupAlpha = preview.popupFadeStart * (1.f - progress);
            preview.weaponAlpha = preview.weaponFadeStart * (1.f - progress);
            if (progress >= 1.f) {
                preview.phase = Game::BrokenWeaponPopup::Phase::Hidden;
                preview.popupAlpha = 0.f;
                preview.weaponAlpha = 0.f;
            }
            break;
        }
    }
}

void drawPreview(Game& game, sf::RenderTarget& target) {
    const auto& preview = game.brokenWeaponPopup;
    if (preview.phase == Game::BrokenWeaponPopup::Phase::Hidden)
        return;

    float popupAlpha = std::clamp(preview.popupAlpha, 0.f, 1.f);
    if (popupAlpha <= 0.f && preview.weaponAlpha <= 0.f)
        return;

    auto view = target.getView();
    float viewWidth = view.getSize().x;
    float viewHeight = view.getSize().y;

    float maxPopupWidth = std::max(kMinPopupWidth, viewWidth - 80.f);
    float popupWidth = std::clamp(viewWidth * 0.78f, kMinPopupWidth, maxPopupWidth);
    float maxPopupHeight = std::max(220.f, viewHeight * kMaxPopupHeightRatio);
    float popupHeight = std::clamp(viewHeight * 0.34f, 220.f, maxPopupHeight);
    float bottomY = game.textBox.getPosition().y - 12.f;
    float popupX = view.getCenter().x - (popupWidth / 2.f);
    float popupY = bottomY - popupHeight;
    if (popupY < 20.f)
        popupY = 20.f;

    sf::RectangleShape background({ popupWidth, popupHeight });
    background.setPosition({ popupX, popupY });
    background.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::BlueAlsoNearBlack, popupAlpha));
    target.draw(background);

    sf::Color frameColor = ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, popupAlpha);
    game.uiFrame.draw(target, background, frameColor);

    uiEffects::drawGlowFrame(
        target,
        game.uiFrame,
        { popupX, popupY },
        { popupWidth, popupHeight },
        uiEffects::computeGlowColor(
            ColorHelper::Palette::BlueLight,
            game.uiGlowClock.getElapsedTime().asSeconds(),
            popupAlpha,
            110.f,
            50.f,
            { 12.f, 28.f },
            0.f
        ),
        6.f
    );

    float contentWidth = popupWidth - (kMargin * 2.f);
    float contentHeight = popupHeight - (kMargin * 2.f);
    if (contentWidth <= 0.f || contentHeight <= 0.f)
        return;

    float spriteAreaHeight = std::max(0.f, contentHeight - kNameAreaHeight);
    float slotWidth = contentWidth / static_cast<float>(kWeapons.size());
    float spriteMaxWidth = slotWidth * 0.9f;
    float spriteCenterY = popupY + kMargin + (spriteAreaHeight * 0.5f);
    float nameY = popupY + kMargin + spriteAreaHeight + (kNameAreaHeight * 0.5f);

    const std::array<const sf::Texture*, 3> textures = {
        &game.resources.weaponHolmabirBroken,
        &game.resources.weaponKattkavarBroken,
        &game.resources.weaponStiggedinBroken
    };

    for (std::size_t index = 0; index < kWeapons.size(); ++index) {
        auto* texture = textures[index];
        const auto& info = kWeapons[index];
        if (!texture)
            continue;

        sf::Sprite sprite{ *texture };
        auto texSize = texture->getSize();
        if (texSize.x == 0 || texSize.y == 0)
            continue;

        float scaleX = spriteMaxWidth / static_cast<float>(texSize.x);
        float scaleY = spriteAreaHeight / static_cast<float>(texSize.y);
        float scale = std::min(scaleX, scaleY);
        sprite.setScale({ scale, scale });

        auto localBounds = sprite.getLocalBounds();
        sprite.setOrigin({
            localBounds.position.x + (localBounds.size.x / 2.f),
            localBounds.position.y + (localBounds.size.y / 2.f)
        });

        float slotCenterX = popupX + kMargin + (slotWidth * (static_cast<float>(index) + 0.5f));
        sprite.setPosition({ slotCenterX, spriteCenterY });

        sf::Color spriteColor = ColorHelper::Palette::Normal;
        spriteColor = ColorHelper::applyAlphaFactor(spriteColor, preview.weaponAlpha);
        sprite.setColor(spriteColor);
        target.draw(sprite);

        sf::Text label{ game.resources.uiFont, info.name, static_cast<unsigned int>(kNameFontSize) };
        label.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, preview.weaponAlpha));
        auto labelBounds = label.getLocalBounds();
        label.setOrigin({
            labelBounds.position.x + (labelBounds.size.x / 2.f),
            labelBounds.position.y + (labelBounds.size.y / 2.f)
        });
        label.setPosition({ slotCenterX, nameY });
        target.draw(label);
    }
}

} // namespace brokenweapon
} // namespace ui
