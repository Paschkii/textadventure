#include "weaponSelectionUI.hpp"
#include "core/game.hpp"
#include "uiEffects.hpp"
#include "uiVisibility.hpp"
#include "rendering/colorHelper.hpp"
#include "story/textStyles.hpp"
#include "story/storyIntro.hpp"
#include <cstdint>
#include <SFML/Window/Mouse.hpp>

namespace {
    void startDragonDialogue(Game& game) {
        game.currentDialogue = &dragon;
        game.dialogueIndex = 0;
        game.visibleText.clear();
        game.charIndex = 0;
        game.typewriterClock.restart();
        game.state = GameState::Dialogue;
    }

    int weaponIndexAt(const Game& game, sf::Vector2f position) {
        for (std::size_t i = 0; i < game.weaponOptions.size(); ++i) {
            if (game.weaponOptions[i].bounds.contains(position))
                return static_cast<int>(i);
        }

        return -1;
    }

    void updateHoveredWeapon(Game& game) {
        auto mousePixel = sf::Mouse::getPosition(game.window);
        sf::Vector2f worldPos = game.window.mapPixelToCoords(mousePixel);
        game.hoveredWeaponIndex = weaponIndexAt(game, worldPos);
    }

    void drawWeaponPanelFrame(Game& game, sf::RenderTarget& target, float uiAlphaFactor, float glowElapsedSeconds) {
        sf::Color glowColor = uiEffects::computeGlowColor(
            ColorHelper::Palette::BlueLight,
            glowElapsedSeconds,
            uiAlphaFactor,
            140.f,
            30.f,
            { 25.f, 41.f }
        );

        uiEffects::drawGlowFrame(
            target,
            game.uiFrame,
            game.weaponPanel.getPosition(),
            game.weaponPanel.getSize(),
            glowColor,
            2.f
        );

        sf::Color frameColor = ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, uiAlphaFactor);
        game.uiFrame.draw(target, game.weaponPanel, frameColor);
    }
}

void handleWeaponSelectionEvent(Game& game, const sf::Event& event) {
    if (event.is<sf::Event::MouseMoved>()) {
        updateHoveredWeapon(game);
    }
    else if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button != sf::Mouse::Button::Left)
            return;

        sf::Vector2f clickPos = game.window.mapPixelToCoords(button->position);
        int clickedIndex = weaponIndexAt(game, clickPos);
        if (clickedIndex >= 0)
            game.selectedWeaponIndex = clickedIndex;

        if (game.selectedWeaponIndex >= 0 && game.state == GameState::WeaponSelection)
            startDragonDialogue(game);
    }
}

void drawWeaponSelectionUI(Game& game, sf::RenderTarget& target) {
    UiVisibility visibility = computeUiVisibility(game, UiElement::WeaponSelectionUI | UiElement::NameBox | UiElement::TextBox | UiElement::LocationBox);

    if (visibility.hidden)
        return;

    updateHoveredWeapon(game);

    float uiAlphaFactor = visibility.alphaFactor;
    float glowElapsedSeconds = game.uiGlowClock.getElapsedTime().asSeconds();

    drawWeaponPanelFrame(game, target, uiAlphaFactor, glowElapsedSeconds);

    constexpr float labelSize = 22.f;
    constexpr float outlinePadding = 6.f;

    for (std::size_t i = 0; i < game.weaponOptions.size(); ++i) {
        auto& option = game.weaponOptions[i];
        if (!option.sprite)
            continue;

        auto& sprite = *option.sprite;
        bool isHovered = game.hoveredWeaponIndex == static_cast<int>(i);
        bool isSelected = game.selectedWeaponIndex == static_cast<int>(i);

        if (isSelected) {
            sf::RectangleShape selectionBg({ option.bounds.size.x + (outlinePadding * 2.f), option.bounds.size.y + (outlinePadding * 2.f) });
            selectionBg.setPosition({ option.bounds.position.x - outlinePadding, option.bounds.position.y - outlinePadding });
            selectionBg.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, uiAlphaFactor * 0.2f));
            selectionBg.setOutlineThickness(0.f);
            target.draw(selectionBg);
        }

        sf::Color spriteColor = sprite.getColor();
        spriteColor.a = static_cast<std::uint8_t>(255.f * uiAlphaFactor);
        sprite.setColor(spriteColor);
        target.draw(sprite);

        if (isHovered) {
            sf::RectangleShape outline({ option.bounds.size.x + (outlinePadding * 2.f), option.bounds.size.y + (outlinePadding * 2.f) });
            outline.setPosition({ option.bounds.position.x - outlinePadding, option.bounds.position.y - outlinePadding });
            outline.setFillColor(sf::Color::Transparent);
            outline.setOutlineColor(ColorHelper::applyAlphaFactor(sf::Color::White, uiAlphaFactor));
            outline.setOutlineThickness(2.f);
            target.draw(outline);
        }

        sf::Text label{ game.resources.titleFont, option.displayName, static_cast<unsigned int>(labelSize) };
        label.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, uiAlphaFactor));
        if (isSelected)
            label.setStyle(sf::Text::Bold);

        auto labelBounds = label.getLocalBounds();
        label.setOrigin({ labelBounds.position.x + (labelBounds.size.x / 2.f), labelBounds.position.y });
        label.setPosition(option.labelPosition);
        target.draw(label);
    }
}