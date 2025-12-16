// === C++ Libraries ===
#include <cstdint>     // Uses fixed-size ints when adjusting alpha channels for sprites.
#include <algorithm>   // Applies std::transform/std::clamp while computing button states.
#include <cctype>      // Converts letters to lower-case when matching hotkeys.
#include <optional>    // Returns optional hotkey selections and index lookups.
// === SFML Libraries ===
#include <SFML/Window/Mouse.hpp>     // Reads mouse movement and clicks for hover/selection logic.
#include <SFML/Window/Keyboard.hpp>  // Processes keyboard hotkeys to pick weapons.
// === Header Files ===
#include "confirmationUI.hpp"    // Shows confirm prompts when choosing a weapon.
#include "uiEffects.hpp"         // Draws the glowing weapon panel background.
#include "uiVisibility.hpp"      // Determines fade state while the weapon UI is shown.
#include "weaponSelectionUI.hpp" // Declares draw/handle functions implemented here.
#include "core/game.hpp"         // Reads/writes Game weapon state, resources, and dialogue progression.
#include "helper/colorHelper.hpp"// Applies palette colors to outlines and selection highlights.
#include "story/textStyles.hpp"  // Names dragons when announcing choices.
#include "story/storyIntro.hpp"  // Accesses dialogue branches triggered by weapon picks.
#include "story/dialogInput.hpp" // Re-uses waitForEnter helpers when restarting dialogue after selection.

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

    char weaponHotkey(const Game::WeaponOption& option) {
        if (option.displayName.empty())
            return '\0';
        return static_cast<char>(std::tolower(static_cast<unsigned char>(option.displayName.front())));
    }

    std::optional<char> keyToWeaponHotkey(sf::Keyboard::Scan code) {
        switch (code) {
            case sf::Keyboard::Scan::H: return 'h';
            case sf::Keyboard::Scan::K: return 'k';
            case sf::Keyboard::Scan::S: return 's';
            default: return std::nullopt;
        }
    }

    int findWeaponIndexForHotkey(const Game& game, char hotkey) {
        for (std::size_t i = 0; i < game.weaponOptions.size(); ++i) {
            if (weaponHotkey(game.weaponOptions[i]) == hotkey)
                return static_cast<int>(i);
        }
        return -1;
    }

    void triggerWeaponSelection(Game& game, int index) {
        if (index < 0 || static_cast<std::size_t>(index) >= game.weaponOptions.size())
            return;

        game.selectedWeaponIndex = index;
        if (game.state != GameState::WeaponSelection)
            return;

        const auto& option = game.weaponOptions[index];
        std::string nameOnly = option.displayName;
        std::string label = "Ahh so this is " + nameOnly + "?";

        std::string description;
        std::string lower = nameOnly;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        if (lower.find("holmabir") != std::string::npos) {
            description = "This is a mighty Longsword!";
        } else if (lower.find("kattkavar") != std::string::npos) {
            description = "This is a mighty Sword and Shield!";
        } else if (lower.find("stiggedin") != std::string::npos) {
            description = "These are mighty Daggers!";
        }

        if (!description.empty())
            label += "\n" + description;

        showConfirmationPrompt(
            game,
            label,
            [](Game& confirmedGame) {
                if (!confirmedGame.weaponItemAdded && confirmedGame.selectedWeaponIndex >= 0 && static_cast<std::size_t>(confirmedGame.selectedWeaponIndex) < confirmedGame.weaponOptions.size()) {
                    const auto& tex = confirmedGame.weaponOptions[confirmedGame.selectedWeaponIndex].texture;
                    confirmedGame.itemController.addIcon(tex);
                    confirmedGame.weaponItemAdded = true;
                }
                startDragonDialogue(confirmedGame);
            },
            [](Game& retryGame) {
                retryGame.selectedWeaponIndex = -1;
                retryGame.visibleText = injectSpeakerNames((*retryGame.currentDialogue)[retryGame.dialogueIndex].text, retryGame);
                retryGame.charIndex = retryGame.visibleText.size();
                retryGame.typewriterClock.restart();
            }
        );
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

        sf::Color frameColor = game.frameColor(uiAlphaFactor);
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
        triggerWeaponSelection(game, clickedIndex);
    }
    else if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        if (auto hotkey = keyToWeaponHotkey(key->scancode)) {
            int desiredIndex = findWeaponIndexForHotkey(game, *hotkey);
            triggerWeaponSelection(game, desiredIndex);
        }
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
            outline.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, uiAlphaFactor));
            outline.setOutlineThickness(2.f);
            target.draw(outline);
        }

        sf::Text nameText{ game.resources.titleFont, option.displayName, static_cast<unsigned int>(labelSize) };
        nameText.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, uiAlphaFactor));

        if (isSelected)
            nameText.setStyle(sf::Text::Bold);

        auto nameBounds = nameText.getLocalBounds();
        sf::Vector2f labelPos = option.labelPosition;
        nameText.setOrigin({ 0.f, nameBounds.position.y });
        nameText.setPosition({ labelPos.x - (nameBounds.size.x / 2.f), labelPos.y });

        target.draw(nameText);

        if (!option.displayName.empty()) {
            std::size_t underlineIndex = 0; // underline the first character
            sf::Vector2f firstPos = nameText.findCharacterPos(static_cast<unsigned>(underlineIndex));
            sf::Vector2f nextPos = nameText.findCharacterPos(static_cast<unsigned>(std::min(option.displayName.size(), underlineIndex + 1)));
            float underlineStartX = firstPos.x;
            float underlineEndX = nextPos.x;
            if (underlineEndX <= underlineStartX)
                underlineEndX = underlineStartX + nameBounds.size.x * 0.08f;

            float underlineY = nameText.getPosition().y + nameBounds.size.y + 3.f;
            float underlineThickness = 2.f;
            sf::Color underlineColor = ColorHelper::Palette::SoftYellow;
            underlineColor.a = static_cast<std::uint8_t>(underlineColor.a * uiAlphaFactor);

            sf::RectangleShape underline({ underlineEndX - underlineStartX, underlineThickness });
            underline.setPosition({ underlineStartX, underlineY });
            underline.setFillColor(underlineColor);
            target.draw(underline);
        }
    }
}
