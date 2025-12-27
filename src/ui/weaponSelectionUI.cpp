// === C++ Libraries ===
#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
#include <string>
#include <vector>
// === SFML Libraries ===
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
// === Header Files ===
#include "core/game.hpp"
#include "story/dialogInput.hpp"
#include "story/textStyles.hpp"
#include "helper/colorHelper.hpp"
#include "ui/uiVisibility.hpp"
#include "ui/confirmationUI.hpp"
#include "ui/uiEffects.hpp"
#include "ui/weaponSelectionUI.hpp"
#include "ui/popupStyle.hpp"
#include "ui/weaponPopupScale.hpp"

namespace {
    constexpr std::array<const char*, 3> kPopupWeaponKeys{{
        "holmabir",
        "kattkavar",
        "stiggedin"
    }};

    std::string toLower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    std::vector<std::size_t> gatherPopupIndexes(const Game& game) {
        std::vector<std::size_t> indexes;
        indexes.reserve(kPopupWeaponKeys.size());
        for (const char* key : kPopupWeaponKeys) {
            auto lowerKey = std::string(key);
            for (std::size_t i = 0; i < game.weaponOptions.size(); ++i) {
                const auto& option = game.weaponOptions[i];
                if (toLower(option.displayName) == lowerKey) {
                    indexes.push_back(i);
                    break;
                }
            }
        }
        return indexes;
    }

    int popupIndexAt(const Game& game, sf::Vector2f position) {
        for (const auto& entry : game.weaponSelectionPopupEntries) {
            if (entry.bounds.contains(position))
                return static_cast<int>(entry.optionIndex);
        }
        return -1;
    }

    void updateHoveredWeapon(Game& game) {
        auto mousePixel = sf::Mouse::getPosition(game.window);
        sf::Vector2f worldPos = game.window.mapPixelToCoords(mousePixel);
        game.hoveredWeaponIndex = popupIndexAt(game, worldPos);
    }

    int findWeaponIndexForHotkey(const Game& game, char hotkey) {
        for (const auto& entry : game.weaponSelectionPopupEntries) {
            const auto& option = game.weaponOptions[entry.optionIndex];
            if (!option.displayName.empty() && std::tolower(option.displayName.front()) == hotkey)
                return static_cast<int>(entry.optionIndex);
        }
        return -1;
    }

    std::optional<char> keyToWeaponHotkey(sf::Keyboard::Scan code) {
        switch (code) {
            case sf::Keyboard::Scan::H: return 'h';
            case sf::Keyboard::Scan::K: return 'k';
            case sf::Keyboard::Scan::S: return 's';
            default: return std::nullopt;
        }
    }

    void triggerWeaponSelection(Game& game, std::size_t optionIndex) {
        if (optionIndex >= game.weaponOptions.size())
            return;

        game.selectedWeaponIndex = static_cast<int>(optionIndex);
        if (game.state != GameState::WeaponSelection)
            return;

        const auto& option = game.weaponOptions[optionIndex];
        std::string nameOnly = option.displayName;
        std::string label = "So you want me to reforge " + nameOnly + "?";
        std::size_t chosenIndex = optionIndex;

        showConfirmationPrompt(
            game,
            label,
            [chosenIndex](Game& confirmedGame) {
                confirmedGame.state = GameState::Dialogue;
                if (chosenIndex < confirmedGame.weaponOptions.size())
                    confirmedGame.forgedWeaponName = confirmedGame.weaponOptions[chosenIndex].displayName;
                removeBrokenWeaponIcons(confirmedGame);
                advanceDialogueLine(confirmedGame);
            },
            [](Game& retryGame) {
                retryGame.selectedWeaponIndex = -1;
                retryGame.hoveredWeaponIndex = -1;
                retryGame.state = GameState::WeaponSelection;
                retryGame.visibleText = injectSpeakerNames((*retryGame.currentDialogue)[retryGame.dialogueIndex].text, retryGame);
                retryGame.charIndex = retryGame.visibleText.size();
                retryGame.typewriterClock.restart();
            }
        );
    }

    constexpr float kPopupMargin = 28.f;
    constexpr float kPopupNameArea = 32.f;
    constexpr float kPopupMinWidth = 520.f;
}

void handleWeaponSelectionEvent(Game& game, const sf::Event& event) {
    if (event.is<sf::Event::MouseMoved>()) {
        updateHoveredWeapon(game);
    }
    else if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button != sf::Mouse::Button::Left)
            return;

        sf::Vector2f clickPos = game.window.mapPixelToCoords(button->position);
        int clickedIndex = popupIndexAt(game, clickPos);
        if (clickedIndex >= 0)
            triggerWeaponSelection(game, static_cast<std::size_t>(clickedIndex));
    }
    else if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        if (auto hotkey = keyToWeaponHotkey(key->scancode)) {
            if (auto desiredIndex = findWeaponIndexForHotkey(game, *hotkey); desiredIndex >= 0) {
                triggerWeaponSelection(game, static_cast<std::size_t>(desiredIndex));
            }
        }
    }
}

void drawWeaponSelectionUI(Game& game, sf::RenderTarget& target) {
    UiVisibility visibility = computeUiVisibility(game, UiElement::WeaponSelectionUI | UiElement::NameBox | UiElement::TextBox | UiElement::LocationBox);
    if (visibility.hidden)
        return;

    auto candidateIndexes = gatherPopupIndexes(game);
    if (candidateIndexes.empty())
        return;

    float uiAlphaFactor = visibility.alphaFactor;
    float glowElapsedSeconds = game.uiGlowClock.getElapsedTime().asSeconds();

    sf::Vector2f viewSize = target.getView().getSize();
    float popupWidth = std::clamp(viewSize.x * 0.78f, kPopupMinWidth, static_cast<float>(viewSize.x - 80.f));
    float popupHeight = std::clamp(viewSize.y * 0.4f, 240.f, viewSize.y * 0.6f);
    popupWidth *= ui::kWeaponPopupScale;
    popupHeight *= ui::kWeaponPopupScale;
    float bottomY = game.textBox.getPosition().y - 12.f;
    float popupX = viewSize.x * 0.5f - (popupWidth * 0.5f);
    float popupY = bottomY - popupHeight;
    if (popupY < 20.f)
        popupY = 20.f;

    sf::FloatRect popupBounds{ { popupX, popupY }, { popupWidth, popupHeight } };
    ui::popup::drawPopupFrame(target, popupBounds, uiAlphaFactor);

    float scaledMargin = kPopupMargin * ui::kWeaponPopupScale;
    float scaledNameArea = kPopupNameArea * ui::kWeaponPopupScale;

    float contentWidth = popupWidth - (scaledMargin * 2.f);
    float contentHeight = popupHeight - (scaledMargin * 2.f);
    if (contentWidth <= 0.f || contentHeight <= 0.f)
        return;

    float spriteAreaHeight = std::max(0.f, contentHeight - scaledNameArea);
    float slotWidth = contentWidth / static_cast<float>(candidateIndexes.size());
    float spriteMaxWidth = slotWidth * 0.9f;
    float spriteCenterY = popupY + scaledMargin + (spriteAreaHeight * 0.5f);
    float nameY = popupY + scaledMargin + spriteAreaHeight + (scaledNameArea * 0.55f);

    game.weaponSelectionPopupEntries.clear();
    game.weaponSelectionPopupEntries.reserve(candidateIndexes.size());
    sf::Vector2f mousePos = game.window.mapPixelToCoords(sf::Mouse::getPosition(game.window));
    game.hoveredWeaponIndex = -1;

    for (std::size_t index = 0; index < candidateIndexes.size(); ++index) {
        std::size_t optionIndex = candidateIndexes[index];
        auto& option = game.weaponOptions[optionIndex];
        const auto& texture = option.texture;
        if (texture.getSize().x == 0 || texture.getSize().y == 0)
            continue;

        sf::Sprite sprite{ texture };
        float scaleX = spriteMaxWidth / static_cast<float>(texture.getSize().x);
        float scaleY = spriteAreaHeight / static_cast<float>(texture.getSize().y);
        float scale = std::min(scaleX, scaleY);
        sprite.setScale({ scale, scale });

        auto localBounds = sprite.getLocalBounds();
        sprite.setOrigin({
            localBounds.position.x + (localBounds.size.x * 0.5f),
            localBounds.position.y + (localBounds.size.y * 0.5f)
        });

        float slotCenterX = popupX + scaledMargin + (slotWidth * (static_cast<float>(index) + 0.5f));
        sprite.setPosition({ slotCenterX, spriteCenterY });

        bool isHovered = false;
        auto spriteBounds = sprite.getGlobalBounds();
        if (spriteBounds.contains(mousePos)) {
            game.hoveredWeaponIndex = static_cast<int>(optionIndex);
            isHovered = true;
        }

        sf::Color spriteColor = sprite.getColor();
        if (isHovered)
            spriteColor = ColorHelper::darken(spriteColor, 0.25f);
        spriteColor.a = static_cast<std::uint8_t>(255.f * uiAlphaFactor);
        sprite.setColor(spriteColor);
        target.draw(sprite);

        game.weaponSelectionPopupEntries.push_back({
            optionIndex,
            spriteBounds,
            { slotCenterX, nameY }
        });

        sf::Text nameText{ game.resources.uiFont, option.displayName, 28 };
        nameText.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, uiAlphaFactor));
        auto nameBounds = nameText.getLocalBounds();
        nameText.setOrigin({
            nameBounds.size.x * 0.5f,
            nameBounds.size.y * 0.5f
        });
        nameText.setPosition({ slotCenterX, nameY });
        target.draw(nameText);
    }
}
