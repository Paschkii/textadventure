#include "ui/treasureChestUI.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include "core/game.hpp"
#include "core/itemActivation.hpp"
#include "helper/colorHelper.hpp"
#include "items/itemRegistry.hpp"
#include "ui/popupStyle.hpp"
#include "ui/quizUI.hpp"

namespace ui {
namespace treasureChest {

namespace {
constexpr float kFadeDuration = 0.25f;
constexpr float kChestWidthRatio = 0.32f;
constexpr float kChestHeightRatio = 0.26f;
constexpr float kPopupWidthRatio = 0.36f;
constexpr float kPopupHeightRatio = 0.42f;
constexpr float kButtonHeight = 42.f;
constexpr float kButtonWidthMultiplier = 0.6f;
constexpr float kPopupPadding = 18.f;

const std::array<const char*, 8> kSillyWords{{
    "Wow", "uuhh", "Can I sell this?", "Booooring", "Cool", "I knew it!", "Yay!", "What a catch"
}};

const std::array<const char*, 4> kUmbraPieces{{
    "umbra_ussea_one",
    "umbra_ussea_two",
    "umbra_ussea_three",
    "umbra_ussea_four"
}};

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

void incrementArtifactCounter(Game& game, std::string_view key) {
    if (auto type = artifactTypeIndex(key))
        ++game.artifactCounts[*type];
}

void maybeCombineUmbraPieces(Game& game) {
    if (game.umbraMapComplete)
        return;
    if (game.umbraFragmentsCollectedCount < static_cast<int>(kUmbraPieces.size()))
        return;
    for (const char* key : kUmbraPieces)
        game.itemController.removeItem(key);
    game.itemController.addItem(game.resources.umbraUsseaComplete, "umbra_ussea_complete");
    game.umbraMapComplete = true;
}

bool isArtifactKey(std::string_view key) {
    return key.rfind("dragonscale_", 0) == 0
        || key.rfind("dragonclaw_", 0) == 0
        || key.rfind("luckycharm_", 0) == 0;
}

std::string elementSuffix(LocationId location) {
    switch (location) {
        case LocationId::Blyathyroid: return "fire";
        case LocationId::Aerobronchi: return "air";
        case LocationId::Lacrimere: return "water";
        case LocationId::Cladrenal: return "earth";
        default: return "fire";
    }
}

const sf::Texture* chestTextureFor(LocationId location, const Game& game) {
    switch (location) {
        case LocationId::Blyathyroid: return &game.resources.treasureChestFire;
        case LocationId::Aerobronchi: return &game.resources.treasureChestAir;
        case LocationId::Lacrimere: return &game.resources.treasureChestWater;
        case LocationId::Cladrenal: return &game.resources.treasureChestEarth;
        default: return &game.resources.treasureChestFire;
    }
}

std::optional<std::size_t> umbraPieceIndex(const std::string& key) {
    if (key == "umbra_ussea_one")
        return 0;
    if (key == "umbra_ussea_two")
        return 1;
    if (key == "umbra_ussea_three")
        return 2;
    if (key == "umbra_ussea_four")
        return 3;
    return std::nullopt;
}

const sf::Texture* mapPieceTexture(const Game& game, const std::string& key) {
    if (auto index = umbraPieceIndex(key)) {
        switch (*index) {
            case 0: return &game.resources.umbraUsseaOne;
            case 1: return &game.resources.umbraUsseaTwo;
            case 2: return &game.resources.umbraUsseaThree;
            case 3: return &game.resources.umbraUsseaFour;
            default: break;
        }
    }
    return nullptr;
}

std::string randomSillyWord(Game::TreasureChestState& state) {
    if (kSillyWords.empty())
        return "Okay";
    std::uniform_int_distribution<std::size_t> picker(0, kSillyWords.size() - 1);
    return kSillyWords[picker(state.rng)];
}

std::string chooseMapPiece(Game& game, Game::TreasureChestState& state) {
    std::vector<std::size_t> available;
    available.reserve(kUmbraPieces.size());
    for (std::size_t idx = 0; idx < kUmbraPieces.size(); ++idx) {
        if (!game.umbraPiecesCollected[idx])
            available.push_back(idx);
    }
    std::size_t chosen = 0;
    if (available.empty()) {
        std::uniform_int_distribution<std::size_t> picker(0, kUmbraPieces.size() - 1);
        chosen = picker(state.rng);
    }
    else {
        std::uniform_int_distribution<std::size_t> picker(0, available.size() - 1);
        chosen = available[picker(state.rng)];
    }
    return kUmbraPieces[chosen];
}

std::vector<std::string> buildRewardKeys(Game& game, LocationId location, Game::TreasureChestState& state) {
    std::vector<std::string> keys;
    std::string suffix = elementSuffix(location);
    keys.push_back("dragonclaw_" + suffix);
    keys.push_back("dragonscale_" + suffix);
    keys.push_back("dragonstone_" + suffix);
    keys.push_back("luckycharm_" + suffix);
    keys.push_back("ring_" + suffix);
    keys.push_back(chooseMapPiece(game, state));
    return keys;
}

void awardReward(Game& game, const std::string& key) {
    if (key.rfind("dragonstone_", 0) == 0) {
        incrementArtifactCounter(game, key);
        game.itemController.collectDragonstone(game.treasureChest.targetLocation);
        core::itemActivation::activateItem(game, key);
        return;
    }
    const sf::Texture* texture = nullptr;
    if (key.rfind("umbra_ussea", 0) == 0)
        texture = mapPieceTexture(game, key);
    else
        texture = core::itemActivation::textureForItemKey(game, key);
    if (!texture)
        return;
    if (key.rfind("ring_", 0) == 0) {
        core::itemActivation::activateItem(game, key);
        return;
    }
    game.itemController.addItem(*texture, key);
    if (auto pieceIndex = umbraPieceIndex(key)) {
        if (!game.umbraPiecesCollected[*pieceIndex]) {
            game.umbraPiecesCollected[*pieceIndex] = true;
            ++game.umbraFragmentsCollectedCount;
        }
    }
    incrementArtifactCounter(game, key);
    maybeCombineUmbraPieces(game);
    if (isArtifactKey(key))
        core::itemActivation::activateItem(game, key);
}

void advanceReward(Game& game) {
    auto& state = game.treasureChest;
    if (state.rewardIndex >= state.rewardKeys.size())
        return;
    awardReward(game, state.rewardKeys[state.rewardIndex]);
    ++state.rewardIndex;
    if (state.rewardIndex >= state.rewardKeys.size()) {
        state.sequenceComplete = true;
        presentDragonstoneReward(game);
        return;
    }
    state.confirmationLabel = randomSillyWord(state);
    state.confirmationHovered = false;
}
} // namespace

void prepare(Game& game, LocationId location) {
    game.state = GameState::TreasureChest;
    auto& state = game.treasureChest;
    state.targetLocation = location;
    state.rewardKeys = buildRewardKeys(game, location, state);
    state.rewardIndex = 0;
    state.chestFade = 0.f;
    state.chestVisible = false;
    state.rewardPopupReady = false;
    state.confirmationLabel = randomSillyWord(state);
    state.confirmationBounds = {};
    state.sequenceComplete = false;
    state.confirmationHovered = false;
}

bool handleEvent(Game& game, const sf::Event& event) {
    if (game.state != GameState::TreasureChest)
        return false;
    auto& state = game.treasureChest;
    if (!state.rewardPopupReady || state.sequenceComplete)
        return false;
    if (auto move = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f point = game.window.mapPixelToCoords(move->position);
        state.confirmationHovered = state.confirmationBounds.contains(point);
        return false;
    }
    if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button != sf::Mouse::Button::Left)
            return false;
        sf::Vector2f point = game.window.mapPixelToCoords(button->position);
        if (state.confirmationBounds.contains(point)) {
            advanceReward(game);
            return true;
        }
    }
    return false;
}

void update(Game& game, float deltaSeconds) {
    if (game.state != GameState::TreasureChest)
        return;
    auto& state = game.treasureChest;
    if (state.sequenceComplete)
        return;
    if (state.chestFade < 1.f) {
        state.chestFade = std::min(1.f, state.chestFade + deltaSeconds / kFadeDuration);
        if (state.chestFade >= 1.f) {
            state.chestVisible = true;
            state.rewardPopupReady = true;
        }
    }
}

void draw(Game& game, sf::RenderTarget& target) {
    if (game.state != GameState::TreasureChest)
        return;
    const auto& state = game.treasureChest;
    if (state.rewardKeys.empty() || state.sequenceComplete)
        return;
    sf::Vector2f viewSize = target.getView().getSize();
    float overlayAlpha = 180.f * state.chestFade;
    sf::RectangleShape overlay({ viewSize.x, viewSize.y });
    overlay.setFillColor(sf::Color(8, 6, 4, static_cast<std::uint8_t>(overlayAlpha)));
    target.draw(overlay);

    const sf::Texture* chestTexture = chestTextureFor(state.targetLocation, game);
    if (chestTexture && chestTexture->getSize().x > 0 && chestTexture->getSize().y > 0) {
        sf::Sprite chest(*chestTexture);
        float maxChestWidth = viewSize.x * kChestWidthRatio;
        float maxChestHeight = viewSize.y * kChestHeightRatio;
        float texWidth = static_cast<float>(chestTexture->getSize().x);
        float texHeight = static_cast<float>(chestTexture->getSize().y);
        float scale = std::min(maxChestWidth / texWidth, maxChestHeight / texHeight);
        chest.setScale({ scale, scale });
        chest.setOrigin({
            texWidth * 0.5f,
            texHeight * 0.5f
        });
        chest.setPosition({ viewSize.x * 0.5f, viewSize.y * 0.5f });
        sf::Color color = chest.getColor();
        color.a = static_cast<std::uint8_t>(255.f * state.chestFade);
        chest.setColor(color);
        target.draw(chest);
    }

    if (!state.rewardPopupReady)
        return;

    float popupWidth = viewSize.x * kPopupWidthRatio;
    float popupHeight = viewSize.y * kPopupHeightRatio;
    sf::Vector2f popupPos{
        (viewSize.x - popupWidth) * 0.5f,
        viewSize.y * 0.12f
    };
    RoundedRectangleShape popup({ popupWidth, popupHeight }, 20.f, 18);
    popup.setPosition(popupPos);
    popup.setFillColor(ColorHelper::Palette::EquipmentPanelFill);
    popup.setOutlineThickness(3.f);
    popup.setOutlineColor(ColorHelper::Palette::EquipmentPanelOutline);
    target.draw(popup);

    const auto& key = state.rewardKeys[state.rewardIndex];
    const auto* definition = items::definitionFor(key);
    std::string title = definition ? definition->title : "Unknown Reward";
    std::string description = definition ? definition->description : "";
    sf::Text titleText{ game.resources.titleFont, title, 30 };
    titleText.setFillColor(ColorHelper::Palette::SoftYellow);
    titleText.setPosition({
        popupPos.x + kPopupPadding,
        popupPos.y + kPopupPadding
    });
    target.draw(titleText);

    sf::Text descText{ game.resources.uiFont, description, 18 };
    descText.setFillColor(ColorHelper::Palette::DarkBrown);
    descText.setLineSpacing(1.1f);
    descText.setPosition({
        popupPos.x + kPopupPadding,
        titleText.getPosition().y + titleText.getLocalBounds().size.y + 10.f
    });
    target.draw(descText);

    const sf::Texture* iconTexture = nullptr;
    if (key.rfind("umbra_ussea", 0) == 0)
        iconTexture = mapPieceTexture(game, key);
    else
        iconTexture = core::itemActivation::textureForItemKey(game, key);
    if (iconTexture && iconTexture->getSize().x > 0 && iconTexture->getSize().y > 0) {
        sf::Sprite icon(*iconTexture);
        float iconSize = std::min(popupWidth, popupHeight) * 0.35f;
        float texWidth = static_cast<float>(iconTexture->getSize().x);
        float texHeight = static_cast<float>(iconTexture->getSize().y);
        float scale = iconSize / std::max(texWidth, texHeight);
        icon.setScale({ scale, scale });
        icon.setOrigin({ texWidth * 0.5f, texHeight * 0.5f });
        icon.setPosition({
            popupPos.x + popupWidth * 0.5f,
            popupPos.y + popupHeight * 0.45f
        });
        icon.setColor(ColorHelper::Palette::MenuSectionBeige);
        target.draw(icon);
    }

    float buttonWidth = std::max(popupWidth * kButtonWidthMultiplier, 180.f);
    if (buttonWidth > popupWidth - kPopupPadding * 2.f)
        buttonWidth = popupWidth - kPopupPadding * 2.f;
    sf::Vector2f buttonPos{
        popupPos.x + (popupWidth - buttonWidth) * 0.5f,
        popupPos.y + popupHeight - kButtonHeight - kPopupPadding
    };
    RoundedRectangleShape button({ buttonWidth, kButtonHeight }, kButtonHeight * 0.5f, 18);
    button.setPosition(buttonPos);
    sf::Color buttonColor = state.confirmationHovered
        ? ColorHelper::lighten(ColorHelper::Palette::MenuSectionBeige, 0.12f)
        : ColorHelper::Palette::MenuSectionBeige;
    button.setFillColor(buttonColor);
    button.setOutlineThickness(2.f);
    button.setOutlineColor(ColorHelper::Palette::DarkBrown);
    target.draw(button);

    sf::Text buttonText{ game.resources.uiFont, state.confirmationLabel, 20 };
    buttonText.setFillColor(ColorHelper::Palette::DarkBrown);
    auto textBounds = buttonText.getLocalBounds();
    buttonText.setOrigin({
        textBounds.position.x + textBounds.size.x * 0.5f,
        textBounds.position.y + textBounds.size.y * 0.5f
    });
    buttonText.setPosition({
        buttonPos.x + buttonWidth * 0.5f,
        buttonPos.y + kButtonHeight * 0.5f
    });
    target.draw(buttonText);

    game.treasureChest.confirmationBounds = button.getGlobalBounds();
}

} // namespace treasureChest
} // namespace ui
