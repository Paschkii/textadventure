#include "battleUI.hpp"

#include <algorithm>
#include <cctype>
#include <array>
#include <cmath>
#include <optional>
#include <string>
#include <unordered_map>

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

#include "core/game.hpp"
#include "ui/popupStyle.hpp"

namespace ui::battle {
namespace {
    constexpr std::array<const char*, 3> kActionLabels = {
        "Dragon Claw",
        "Dragon Pulse",
        "Roost"
    };

    constexpr std::array<const char*, 2> kEnemyMoves = {
        "Thunder Punch",
        "Earthquake"
    };

    constexpr std::array<float, 2> kEnemyDamage = { 28.f, 32.f };

    sf::Sprite makeLayer(const sf::Texture& texture, const sf::Vector2f& targetSize) {
        sf::Sprite sprite(texture);
        auto bounds = sprite.getLocalBounds();
        if (bounds.size.x <= 0.f || bounds.size.y <= 0.f)
            return sprite;
        float scaleX = targetSize.x / bounds.size.x;
        float scaleY = targetSize.y / bounds.size.y;
        sprite.setScale(sf::Vector2f{ scaleX, scaleY });
        return sprite;
    }

    sf::FloatRect normalizedOpaqueBounds(const sf::Texture& texture) {
        static std::unordered_map<const sf::Texture*, sf::FloatRect> cache;
        auto itr = cache.find(&texture);
        if (itr != cache.end())
            return itr->second;

        sf::Image image = texture.copyToImage();
        auto size = image.getSize();
        sf::FloatRect normalized(sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ 1.f, 1.f });
        if (size.x == 0 || size.y == 0) {
            cache.emplace(&texture, normalized);
            return normalized;
        }

        unsigned minX = size.x;
        unsigned minY = size.y;
        unsigned maxX = 0;
        unsigned maxY = 0;
        bool found = false;
        for (unsigned y = 0; y < size.y; ++y) {
            for (unsigned x = 0; x < size.x; ++x) {
                if (image.getPixel({ x, y }).a == 0)
                    continue;
                if (!found) {
                    minX = maxX = x;
                    minY = maxY = y;
                    found = true;
                } else {
                    minX = std::min(minX, x);
                    minY = std::min(minY, y);
                    maxX = std::max(maxX, x);
                    maxY = std::max(maxY, y);
                }
            }
        }

        if (found) {
            float width = static_cast<float>(maxX - minX + 1);
            float height = static_cast<float>(maxY - minY + 1);
            normalized.position.x = static_cast<float>(minX) / static_cast<float>(size.x);
            normalized.position.y = static_cast<float>(minY) / static_cast<float>(size.y);
            normalized.size.x = width / static_cast<float>(size.x);
            normalized.size.y = height / static_cast<float>(size.y);
        }

        cache.emplace(&texture, normalized);
        return normalized;
    }

    sf::Vector2f normalizedOpaqueCenter(const sf::Texture& texture) {
        sf::FloatRect bounds = normalizedOpaqueBounds(texture);
        if (bounds.size.x <= 0.f || bounds.size.y <= 0.f)
            return { 0.5f, 0.5f };
        return {
            bounds.position.x + (bounds.size.x * 0.5f),
            bounds.position.y + (bounds.size.y * 0.5f)
        };
    }

    sf::Vector2f normalizedOpaqueBottom(const sf::Texture& texture) {
        sf::FloatRect bounds = normalizedOpaqueBounds(texture);
        if (bounds.size.x <= 0.f || bounds.size.y <= 0.f)
            return { 0.5f, 1.f };
        return {
            bounds.position.x + (bounds.size.x * 0.5f),
            bounds.position.y + bounds.size.y
        };
    }

    void pushLog(Game& game, std::string entry) {
        auto& log = game.battleDemo.logHistory;
        log.push_back(std::move(entry));
        if (log.size() > 3)
            log.erase(log.begin());
    }

    void queueEnemyAttack(Game& game) {
        auto& battle = game.battleDemo;
        int moveIndex = battle.enemyMoveIndex % static_cast<int>(kEnemyMoves.size());
        float damage = kEnemyDamage[moveIndex];
        battle.enemyMoveIndex = (moveIndex + 1) % static_cast<int>(kEnemyMoves.size());
        battle.player.hp = std::max(1.f, battle.player.hp - damage);
        pushLog(game, "Master Bates used " + std::string(kEnemyMoves[moveIndex]) + "!");
    }

    void beginPlayerAction(Game& game) {
        auto& battle = game.battleDemo;
        if (battle.phase != BattleDemoState::Phase::PlayerChoice)
            return;

        battle.phase = BattleDemoState::Phase::PlayerAction;
        battle.actionTimer = 0.f;

        constexpr std::array<const char*, 3> kMoveNames = {
            "Dragon Claw",
            "Dragon Pulse",
            "Roost"
        };
        constexpr std::array<float, 3> kMovePower = { 32.f, 44.f, 0.f };

        pushLog(game, "Dragonborn used " + std::string(kMoveNames[battle.selectedAction]) + "!");

        if (battle.selectedAction == 2) {
            float heal = 36.f;
            battle.player.hp = std::min(battle.player.maxHp, battle.player.hp + heal);
        } else {
            battle.enemy.hp = std::max(0.f, battle.enemy.hp - kMovePower[battle.selectedAction]);
            if (battle.enemy.hp <= 0.f) {
                battle.phase = BattleDemoState::Phase::Victory;
                battle.victoryTransitioned = false;
                battle.completionClock.restart();
            }
        }
    }
}

void update(Game& game, sf::Time dt) {
    if (game.state != GameState::BattleDemo)
        return;

    auto& battle = game.battleDemo;
    if (battle.phase == BattleDemoState::Phase::PlayerAction) {
        battle.actionTimer += dt.asSeconds();
        if (battle.actionTimer >= battle.playerActionDelay) {
            battle.phase = BattleDemoState::Phase::EnemyAction;
            battle.actionTimer = 0.f;
        }
    }
    else if (battle.phase == BattleDemoState::Phase::EnemyAction) {
        battle.actionTimer += dt.asSeconds();
        if (battle.actionTimer >= battle.enemyActionDelay) {
            queueEnemyAttack(game);
            battle.phase = BattleDemoState::Phase::PlayerChoice;
            battle.actionTimer = 0.f;
        }
    }
    else if (battle.phase == BattleDemoState::Phase::Victory && !battle.victoryTransitioned) {
        if (battle.completionClock.getElapsedTime().asSeconds() >= battle.victoryHoldTime) {
            battle.victoryTransitioned = true;
            battle.phase = BattleDemoState::Phase::Complete;
            game.state = GameState::IntroScreen;
            game.introClock.restart();
            game.introFadeOutActive = false;
            game.introTitleHidden = false;
            game.introTitleRevealStarted = false;
            game.introTitleRevealComplete = false;
            game.introTitleOptionsFadeTriggered = false;
            game.introTitleOptionsFadeActive = false;
            game.introTitleOptionsFadeProgress = 0.f;
            game.introTitleHoveredOption = -1;
            game.pendingIntroDialogue = false;
        }
    }
}

bool handleEvent(Game& game, const sf::Event& event) {
    if (game.state != GameState::BattleDemo)
        return false;

    auto& battle = game.battleDemo;
    if (battle.phase != BattleDemoState::Phase::PlayerChoice)
        return false;

    if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        switch (key->scancode) {
            case sf::Keyboard::Scan::Left:
            case sf::Keyboard::Scan::Up:
                battle.selectedAction = (battle.selectedAction + static_cast<int>(kActionLabels.size()) - 1)
                    % static_cast<int>(kActionLabels.size());
                return true;
            case sf::Keyboard::Scan::Right:
            case sf::Keyboard::Scan::Down:
                battle.selectedAction = (battle.selectedAction + 1) % static_cast<int>(kActionLabels.size());
                return true;
            case sf::Keyboard::Scan::Enter:
            case sf::Keyboard::Scan::Space:
            case sf::Keyboard::Scan::Z:
                beginPlayerAction(game);
                return true;
            default:
                return false;
        }
    }

    if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button == sf::Mouse::Button::Left) {
            beginPlayerAction(game);
            return true;
        }
    }

    return false;
}

void draw(Game& game, sf::RenderTarget& target) {
    const auto size = target.getSize();
    float width = static_cast<float>(size.x);
    float height = static_cast<float>(size.y);

    auto& battle = game.battleDemo;
    auto formatHpValue = [](float current, float maximum) {
        int cur = static_cast<int>(std::lround(std::max(0.f, current)));
        int maxv = static_cast<int>(std::lround(std::max(0.f, maximum)));
        return std::to_string(cur) + "/" + std::to_string(maxv);
    };

    sf::Sprite background = makeLayer(game.resources.battleBackgroundLayer, { width, height });
    auto backgroundBounds = background.getGlobalBounds();
    auto backgroundPosition = sf::Vector2f{ (width - backgroundBounds.size.x) * 0.5f, (height - backgroundBounds.size.y) * 0.5f };
    background.setPosition(backgroundPosition);
    target.draw(background);

    auto drawLayer = [&](const sf::Texture& texture, const sf::Vector2f& targetSize) -> sf::FloatRect {
        sf::Sprite sprite = makeLayer(texture, targetSize);
        sprite.setPosition(backgroundPosition);
        target.draw(sprite);
        return sprite.getGlobalBounds();
    };

    auto computeBoxBounds = [&](const sf::Texture& texture) -> sf::FloatRect {
        sf::FloatRect normalized = normalizedOpaqueBounds(texture);
        if (normalized.size.x <= 0.f || normalized.size.y <= 0.f)
            return {};
        const sf::Vector2f scale = backgroundBounds.size;
        return sf::FloatRect(
            sf::Vector2f{
                backgroundPosition.x + (normalized.position.x * scale.x),
                backgroundPosition.y + (normalized.position.y * scale.y)
            },
            sf::Vector2f{
                normalized.size.x * scale.x,
                normalized.size.y * scale.y
            }
        );
    };

    constexpr float kGenderIconScale = 0.02f;
    constexpr float kGenderIconSpacing = 5.f;

    auto iconForGender = [&](Game::DragonbornGender gender) -> const sf::Texture* {
        return (gender == Game::DragonbornGender::Female)
            ? &game.resources.battleIconFemale
            : &game.resources.battleIconMale;
    };

    auto genderFromName = [&](const std::string& name) -> std::optional<Game::DragonbornGender> {
        std::string normalized = name;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        if (normalized == "wanda rinn")
            return Game::DragonbornGender::Female;
        if (normalized == "noah lott")
            return Game::DragonbornGender::Male;
        if (normalized == "will figsid")
            return Game::DragonbornGender::Male;
        if (normalized == "master bates")
            return Game::DragonbornGender::Male;
        return std::nullopt;
    };

    auto drawNameField = [&](const sf::FloatRect& boxBounds, const std::string& label,
                             float offsetX, float offsetY, const sf::Texture* iconTexture) {
        constexpr float kPadding = 12.f;
        std::string upperLabel = label;
        std::transform(upperLabel.begin(), upperLabel.end(), upperLabel.begin(), [](unsigned char ch) {
            return static_cast<char>(std::toupper(ch));
        });
        sf::Text nameLabel{ game.resources.battleFont, upperLabel, 32 };
        nameLabel.setFillColor(sf::Color::White);
        nameLabel.setOutlineColor(sf::Color::Transparent);
        nameLabel.setOutlineThickness(0.f);
        nameLabel.setStyle(sf::Text::Bold);
        nameLabel.setLetterSpacing(nameLabel.getLetterSpacing() + 1.f);
        nameLabel.setPosition({
            boxBounds.position.x + kPadding + offsetX,
            boxBounds.position.y + kPadding + offsetY
        });
        auto nameBounds = nameLabel.getGlobalBounds();
        target.draw(nameLabel);

        if (iconTexture) {
            sf::Sprite icon{ *iconTexture };
            auto iconBounds = icon.getLocalBounds();
            icon.setOrigin(sf::Vector2f{
                iconBounds.position.x,
                iconBounds.position.y + (iconBounds.size.y * 0.5f)
            });
            icon.setScale(sf::Vector2f{ kGenderIconScale, kGenderIconScale });
            float textRight = nameBounds.position.x + nameBounds.size.x;
            float iconY = nameBounds.position.y + (nameBounds.size.y * 0.5f);
            icon.setPosition(sf::Vector2f{ textRight + kGenderIconSpacing, iconY });
            target.draw(icon);
        }
    };

    constexpr float kNameBadgePadding = 12.f;
    constexpr float kLevelOffsetFromAnchor = 30.f;
    auto computeRightAnchor = [&](const sf::FloatRect& boxBounds, float extraLeftOffset) {
        float boxHeight = boxBounds.size.y;
        float padding = std::clamp(boxHeight * 0.04f, 8.f, 16.f);
        float rightPaddingOffset = 5.f;
        return boxBounds.position.x + boxBounds.size.x - padding - rightPaddingOffset - extraLeftOffset;
    };

    auto drawLevelBadge = [&](const sf::FloatRect& boxBounds, const BattleDemoState::Combatant& combatant,
                              float extraLeftOffset, float offsetY) {
        float rowRight = computeRightAnchor(boxBounds, extraLeftOffset);

        std::string levelLabel = "Lv." + std::to_string(combatant.level);
        std::transform(levelLabel.begin(), levelLabel.end(), levelLabel.begin(), [](unsigned char ch) {
            return static_cast<char>(std::toupper(ch));
        });

        sf::Text levelText{ game.resources.battleFont, levelLabel, 32 };
        levelText.setFillColor(sf::Color::White);
        levelText.setStyle(sf::Text::Bold);
        levelText.setLetterSpacing(levelText.getLetterSpacing() + 1.f);

        auto levelBounds = levelText.getLocalBounds();
        float levelX = rowRight - kLevelOffsetFromAnchor - levelBounds.size.x;
        float boxHeight = boxBounds.size.y;
        float padding = std::clamp(boxHeight * 0.04f, 8.f, 16.f);
        float minX = boxBounds.position.x + padding;
        levelX = std::max(levelX, minX);
        float levelY = boxBounds.position.y + kNameBadgePadding + offsetY;
        levelText.setPosition(sf::Vector2f{ levelX, levelY });
        target.draw(levelText);
    };

    auto drawHealthWidget = [&](const sf::FloatRect& boxBounds, const BattleDemoState::Combatant& combatant,
                                float extraLeftOffset, float extraBottomOffset) -> std::optional<sf::FloatRect> {
        float boxWidth = boxBounds.size.x;
        float boxHeight = boxBounds.size.y;
        float padding = std::clamp(boxHeight * 0.04f, 8.f, 16.f);
        float barHeight = std::clamp(boxHeight * 0.05f, 10.f, 18.f);

        sf::Text badgeLabel{ game.resources.battleFontBold, "HP", 20 };
        badgeLabel.setFillColor(ColorHelper::Palette::SoftRed);
        badgeLabel.setOutlineColor(sf::Color::Transparent);
        badgeLabel.setOutlineThickness(0.f);
        badgeLabel.setStyle(sf::Text::Bold);
        auto badgeLabelBounds = badgeLabel.getLocalBounds();
        float labelWidth = badgeLabelBounds.size.x;
        float labelHeight = badgeLabelBounds.size.y;
        float labelBlockWidth = labelWidth + 8.f;

        float rightPaddingOffset = 5.f;
        float badgeX = boxBounds.position.x + padding;
        float labelSpacing = 5.f;
        float horizontalShift = extraLeftOffset;
        float extraVerticalShift = extraBottomOffset;

        float rowRight = boxBounds.position.x + boxWidth - padding - rightPaddingOffset - horizontalShift;
        float rowLeft = badgeX;
        float maxAvailableWidth = std::max(0.f, rowRight - rowLeft);
        float barWidth = std::clamp(boxWidth * 0.4f, 30.f, std::max(0.f, maxAvailableWidth - (labelWidth + labelSpacing)));
        if (barWidth <= 0.f || rowRight <= rowLeft)
            return std::nullopt;

        float barX = rowRight - barWidth;
        float labelX = barX - labelSpacing - labelWidth;
        if (labelX < rowLeft)
            labelX = rowLeft;
        float minBarTop = boxBounds.position.y + padding;
        float desiredBarBottom = boxBounds.position.y + boxHeight - padding - 70.f - extraVerticalShift;
        float barY = desiredBarBottom - barHeight;
        float maxBarY = boxBounds.position.y + boxHeight - padding - barHeight;
        barY = std::clamp(barY, minBarTop, maxBarY);

        if (barX + barWidth > boxBounds.position.x + boxWidth - padding)
            barWidth = std::max(0.f, (boxBounds.position.x + boxWidth - padding) - barX);
        if (barWidth <= 0.f)
            return std::nullopt;

        float labelCenterY = barY + (barHeight * 0.5f);
        float textTop = labelCenterY - (labelHeight * 0.5f);
        float textBottom = labelCenterY + (labelHeight * 0.5f);
        float backgroundLeft = labelX - 2.f;
        float backgroundTop = std::min(barY, textTop) - 2.f;
        float backgroundRight = std::max(barX + barWidth, labelX + labelWidth) + 2.f;
        float backgroundBottom = std::max(barY + barHeight, textBottom) + 2.f;
        float backgroundWidth = std::max(0.f, backgroundRight - backgroundLeft);
        float backgroundHeight = std::max(0.f, backgroundBottom - backgroundTop);
        if (backgroundWidth > 0.f && backgroundHeight > 0.f) {
            RoundedRectangleShape hpBackground({ backgroundWidth, backgroundHeight }, backgroundHeight * 0.5f, 20);
            hpBackground.setPosition({ backgroundLeft, backgroundTop });
            hpBackground.setFillColor(sf::Color::Black);
            hpBackground.setOutlineThickness(2.f);
            hpBackground.setOutlineColor(sf::Color::Black);
            target.draw(hpBackground);
        }

        badgeLabel.setOrigin({
            badgeLabelBounds.position.x,
            badgeLabelBounds.position.y + (badgeLabelBounds.size.y * 0.5f)
        });
        badgeLabel.setPosition({
            labelX,
            labelCenterY
        });
        auto& shader = game.resources.hpBadgeShader;
        if (shader.isAvailable()) {
            badgeLabel.setFillColor(sf::Color::White);
            target.draw(badgeLabel, sf::RenderStates{ &shader });
        } else {
            badgeLabel.setFillColor(ColorHelper::Palette::SoftRed);
            target.draw(badgeLabel);
        }

        float hpRatio = (combatant.maxHp > 0.f)
            ? std::clamp(combatant.hp / combatant.maxHp, 0.f, 1.f)
            : 0.f;
        if (hpRatio > 0.f) {
            float hpFillWidth = std::max(barWidth * hpRatio, barHeight);
            hpFillWidth = std::min(barWidth, hpFillWidth);
            RoundedRectangleShape hpFill({ hpFillWidth, barHeight }, barHeight * 0.5f, 20);
            hpFill.setPosition({ barX, barY });
            hpFill.setFillColor(ColorHelper::Palette::Health);
            target.draw(hpFill);
        }

        RoundedRectangleShape hpBorder({ barWidth, barHeight }, barHeight * 0.5f, 20);
        hpBorder.setPosition({ barX, barY });
        hpBorder.setFillColor(sf::Color::Transparent);
        hpBorder.setOutlineThickness(1.f);
        hpBorder.setOutlineColor(ColorHelper::Palette::Dim);
        target.draw(hpBorder);
        return sf::FloatRect(sf::Vector2f{ barX, barY }, sf::Vector2f{ barWidth, barHeight });
    };

    auto drawPlayerHpValue = [&](const sf::FloatRect& barBounds, const sf::FloatRect& boxBounds,
                                 const BattleDemoState::Combatant& combatant, float extraLeftOffset) {
        std::string hpTextValue = formatHpValue(combatant.hp, combatant.maxHp);
        sf::Text hpText{ game.resources.battleFont, hpTextValue, 28 };
        hpText.setFillColor(sf::Color::White);
        hpText.setStyle(sf::Text::Bold);
        hpText.setLetterSpacing(hpText.getLetterSpacing() + 1.f);
        constexpr float kHpValueSpacing = 6.f;
        auto hpBounds = hpText.getLocalBounds();
        float rowRight = computeRightAnchor(boxBounds, extraLeftOffset);
        float textX = rowRight - kLevelOffsetFromAnchor - hpBounds.size.x;
        float boxHeight = boxBounds.size.y;
        float padding = std::clamp(boxHeight * 0.04f, 8.f, 16.f);
        float minX = boxBounds.position.x + padding;
        textX = std::max(textX, minX);
        float textY = barBounds.position.y + barBounds.size.y + kHpValueSpacing;
        float bottomLimit = boxBounds.position.y + boxBounds.size.y - kHpValueSpacing;
        textY = std::min(textY, bottomLimit);
        hpText.setPosition(sf::Vector2f{ textX, textY });
        target.draw(hpText);
    };

    sf::FloatRect playerPlatformBounds = drawLayer(game.resources.battlePlayerPlatform, backgroundBounds.size);
    sf::FloatRect enemyPlatformBounds = drawLayer(game.resources.battleEnemyPlatform, backgroundBounds.size);

    sf::Sprite dragonborn(game.resources.spriteDragonbornBack);
    auto dragonbornBounds = dragonborn.getLocalBounds();
    sf::Vector2f dragonbornCenter = normalizedOpaqueCenter(game.resources.spriteDragonbornBack);
    dragonborn.setOrigin(sf::Vector2f{ dragonbornCenter.x * dragonbornBounds.size.x, dragonbornCenter.y * dragonbornBounds.size.y });
    dragonborn.setScale(sf::Vector2f{ 0.30f, 0.30f });
    sf::Vector2f playerCenterNormalized = normalizedOpaqueCenter(game.resources.battlePlayerPlatform);
    sf::Vector2f playerCenter{
        playerPlatformBounds.position.x + playerPlatformBounds.size.x * playerCenterNormalized.x,
        playerPlatformBounds.position.y + playerPlatformBounds.size.y * playerCenterNormalized.y
    };
    dragonborn.setPosition(playerCenter);
    target.draw(dragonborn);

    drawLayer(game.resources.battleTextBox, backgroundBounds.size);
    drawLayer(game.resources.battleActionBox, backgroundBounds.size);
    drawLayer(game.resources.battlePlayerBox, backgroundBounds.size);
    drawLayer(game.resources.battleEnemyBox, backgroundBounds.size);

    sf::FloatRect playerBoxBounds = computeBoxBounds(game.resources.battlePlayerBox);
    sf::FloatRect enemyBoxBounds = computeBoxBounds(game.resources.battleEnemyBox);

    auto playerHealthBar = drawHealthWidget(playerBoxBounds, battle.player, 40.f, 0.f);
    drawHealthWidget(enemyBoxBounds, battle.enemy, 40.f, -40.f);
    drawLevelBadge(playerBoxBounds, battle.player, 10.f, 30.f);
    drawLevelBadge(enemyBoxBounds, battle.enemy, 10.f, 15.f);
    if (playerHealthBar)
        drawPlayerHpValue(*playerHealthBar, playerBoxBounds, battle.player, 10.f);
    const sf::Texture* playerIcon = iconForGender(game.playerGender);
    auto enemyGender = genderFromName(battle.enemy.name);
    const sf::Texture* enemyIcon = enemyGender ? iconForGender(*enemyGender) : nullptr;
    drawNameField(playerBoxBounds, "Player", 50.f, 30.f, playerIcon);
    drawNameField(enemyBoxBounds, "Master Bates", 50.f, 15.f, enemyIcon);

    sf::Sprite masterBates(game.resources.spriteMasterBates);
    auto masterBounds = masterBates.getLocalBounds();
    sf::Vector2f masterBottom = normalizedOpaqueBottom(game.resources.spriteMasterBates);
    masterBates.setOrigin(sf::Vector2f{ masterBottom.x * masterBounds.size.x, masterBottom.y * masterBounds.size.y });
    masterBates.setScale(sf::Vector2f{ 0.20f, 0.20f });
    sf::Vector2f enemyCenterNormalized = normalizedOpaqueCenter(game.resources.battleEnemyPlatform);
    sf::Vector2f enemyCenter{
        enemyPlatformBounds.position.x + enemyPlatformBounds.size.x * enemyCenterNormalized.x,
        enemyPlatformBounds.position.y + enemyPlatformBounds.size.y * enemyCenterNormalized.y
    };
    sf::Vector2f anchorOffset{
        -enemyPlatformBounds.size.x * 0.03f,
        enemyPlatformBounds.size.y * 0.05f
    };
    masterBates.setPosition(enemyCenter + anchorOffset);
    target.draw(masterBates);
}

} // namespace ui::battle
