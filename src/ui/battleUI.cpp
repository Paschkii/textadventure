#include "battleUI.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <array>
#include <cmath>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <random>

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

#include "core/game.hpp"
#include "core/itemActivation.hpp"
#include "helper/colorHelper.hpp"
#include "story/storyIntro.hpp"
#include "ui/popupStyle.hpp"

namespace ui::battle {
void startBattle(Game& game) {
    game.battleDemo = BattleDemoState{};
    game.battleDemo.player.name = game.playerName.empty() ? "Dragonborn" : game.playerName;
    game.state = GameState::BattleDemo;
    game.setMenuActive(false);
    game.menuHoveredTab = -1;
    game.menuMapPopup.reset();
    game.audioManager.fadeOutLocationMusic(0.f);
}

namespace {
    constexpr std::array<const char*, BattleDemoState::kActionOptionCount> kActionLabels = {
        "FIGHT",
        "DRAG",
        "GLANDUMON",
        "RUN"
    };
    static_assert(kActionLabels.size() == BattleDemoState::kActionOptionCount, "Action label count mismatch");

    constexpr int kDragonbornBaseLevel = 50;
    constexpr float kDragonbornBaseMaxHp = 980.f;
    constexpr float kDragonbornDamageMultiplier = 4.f;
    constexpr std::array<float, BattleDemoState::kSkillSlotCount> kDragonbornSkillDamageLevel50 = {
        50.f, 64.f, 56.f, 52.f
    };

    constexpr std::array<float, BattleDemoState::kSkillSlotCount> kEnemySkillDamage = {
        120.f, 150.f, 0.f, 0.f
    };
    constexpr std::array<const char*, BattleDemoState::kSkillSlotCount> kPlayerSkillNames = {
        "Air Slash",
        "Fire Slash",
        "Earth Slash",
        "Water Slash"
    };
    constexpr float kBattleHpPulseDuration = 0.6f;
    constexpr int kIntroBlinkCycles = 5;
    constexpr float kIntroBlinkVisibleDuration = 0.75f;
    constexpr float kIntroBlinkHiddenDuration = 0.45f;
    constexpr float kIntroShadeDuration = 1.4f;
    constexpr char kBattleMusicPath[] = "assets/audio/battleMusic.wav";
    constexpr float kBattleMusicSampleRate = 44100.f;
    constexpr float kBattleMusicLoopStartSample = 1089536.f;
    constexpr float kBattleMusicLoopEndSample = 3045894.f;
    constexpr float kSkillSlashBlinkInterval = 0.06f;
    constexpr int kSkillSlashBlinkCycles = 3;
    constexpr float kSkillWeaponFadeDuration = 0.25f;
    constexpr float kMasterBatesSkillSpriteScale = 0.23f;
    constexpr float kMasterBatesDragonMultiplier = 2.6f;
    constexpr float kSkillEffectScaleFactor = 0.65f;
    constexpr float kFriendshipSpriteTargetFactor = 0.5f;
    constexpr float kSwapPhaseDuration = 1.f;
    constexpr float kSwapOverlayMaxAlpha = 220.f;
    constexpr float kSwapPromptMargin = 32.f;
    constexpr float kSwapPromptWidth = 360.f;
    constexpr float kSwapPromptHeight = 180.f;
    // TEMP: speedrun helper; remove this flag + skipBattleAnimations for cleanup.
    constexpr bool kEnableBattleSkip = true;

    struct SwapPromptLayout {
        sf::FloatRect panel;
        sf::FloatRect yesBounds;
        sf::FloatRect noBounds;
    };

    void openCreatureMenu(BattleDemoState& battle, BattleDemoState::CreatureMenuType type, bool allowCancel = true);
    float dragonbornMaxHpForLevel(int level);
    float dragonbornSkillDamageForLevel(int level, int skillIndex);
    bool allTrackedCreaturesDefeated(const BattleDemoState& battle);

    SwapPromptLayout computeSwapPromptLayout(const sf::Vector2f& viewportSize) {
        float width = std::clamp(kSwapPromptWidth, 0.f, std::max(0.f, viewportSize.x - (kSwapPromptMargin * 2.f)));
        float height = std::clamp(kSwapPromptHeight, 0.f, std::max(0.f, viewportSize.y - (kSwapPromptMargin * 2.f)));
        float x = std::max(0.f, viewportSize.x - kSwapPromptMargin - width);
        float y = kSwapPromptMargin;
        sf::FloatRect panel({ x, y }, { width, height });
        float buttonWidth = std::min(140.f, width * 0.35f);
        float buttonHeight = 48.f;
        float buttonSpacing = 18.f;
        float buttonY = panel.position.y + panel.size.y - buttonHeight - 20.f;
        float centerX = panel.position.x + (panel.size.x * 0.5f);
        sf::FloatRect yesBounds(
            { centerX - buttonWidth - (buttonSpacing * 0.5f), buttonY },
            { buttonWidth, buttonHeight }
        );
        sf::FloatRect noBounds(
            { centerX + (buttonSpacing * 0.5f), buttonY },
            { buttonWidth, buttonHeight }
        );
        return { panel, yesBounds, noBounds };
    }

    std::string toLowerCopy(std::string_view value) {
        std::string lowered(value);
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return lowered;
    }

    bool containsToken(std::string_view full, std::string_view token) {
        if (token.empty())
            return false;
        std::string loweredFull = toLowerCopy(full);
        std::string loweredToken = toLowerCopy(token);
        return loweredFull.find(loweredToken) != std::string::npos;
    }

    std::string displayPlayerName(const Game& game, const BattleDemoState& battle) {
        if (!battle.player.name.empty())
            return battle.player.name;
        if (!game.playerName.empty())
            return game.playerName;
        return "Dragonborn";
    }

    std::string heroDisplayName(const Game& game) {
        if (!game.playerName.empty())
            return game.playerName;
        return "Dragonborn";
    }

    bool isMasterBatesName(std::string_view candidate) {
        return containsToken(candidate, "master bates");
    }

    const sf::Texture* dragonbornBackTexture(const Game& game) {
        return (game.playerGender == Game::DragonbornGender::Female)
            ? &game.resources.spriteDragonbornFemaleBack
            : &game.resources.spriteDragonbornMaleBack;
    }

    std::array<const sf::Texture*, 3> finalizeFriendshipTextures(
        const std::vector<const sf::Texture*>& sources,
        const sf::Texture* fallback
    ) {
        std::array<const sf::Texture*, 3> result{};
        size_t written = 0;
        for (const sf::Texture* texture : sources) {
            if (!texture)
                continue;
            result[written++] = texture;
            if (written >= result.size())
                break;
        }
        while (written < result.size()) {
            result[written++] = fallback;
        }
        return result;
    }

    std::array<const sf::Texture*, 3> friendshipNpcTextures(const Game& game, std::string_view activeName) {
        std::vector<const sf::Texture*> candidates;
        auto addIfNotActive = [&](std::string_view token, const sf::Texture* texture) {
            if (!containsToken(activeName, token)) {
                candidates.push_back(texture);
            }
        };
        addIfNotActive("wanda", &game.resources.spriteWandaRinnBack);
        addIfNotActive("will", &game.resources.spriteWillFigsidBack);
        addIfNotActive("noah", &game.resources.spriteNoahLottBack);
        const sf::Texture* dragonbornBack = dragonbornBackTexture(game);
        if (!containsToken(activeName, "dragonborn")) {
            candidates.push_back(dragonbornBack);
        }
        return finalizeFriendshipTextures(candidates, dragonbornBack);
    }

    std::array<const sf::Texture*, 3> friendshipDragonTextures(const Game& game, std::string_view activeName) {
        std::vector<const sf::Texture*> candidates;
        auto addIfNotActive = [&](std::string_view token, const sf::Texture* texture) {
            if (!containsToken(activeName, token))
                candidates.push_back(texture);
        };
        addIfNotActive("rowsted", &game.resources.spriteRowstedSheacaneBack);
        addIfNotActive("flawtin", &game.resources.spriteFlawtinSeamenBack);
        addIfNotActive("gustavo", &game.resources.spriteGustavoWindimaessBack);
        addIfNotActive("grounded", &game.resources.spriteGroundedClaymoreBack);
        const sf::Texture* fallback = &game.resources.spriteRowstedSheacaneBack;
        return finalizeFriendshipTextures(candidates, fallback);
    }

    bool isDragonName(std::string_view name) {
        return containsToken(name, "rowsted")
            || containsToken(name, "flawtin")
            || containsToken(name, "gustavo")
            || containsToken(name, "grounded");
    }

    bool isMasterBatesSkill(std::string_view skillName) {
        return skillName == "Midnight Release"
            || skillName == "Bad Habit"
            || skillName == "Shadow Routine"
            || skillName == "Guilty Ember";
    }

    constexpr std::array<std::string_view, 7> kTrackedCreatureNames = {
        "Wanda Rinn",
        "Will Figsid",
        "Noah Lott",
        "Rowsted",
        "Flawtin",
        "Gustavo",
        "Grounded"
    };

    bool haveTrackingForCreature(const BattleDemoState& battle, std::string_view name) {
        return battle.creatureHp.find(std::string(name)) != battle.creatureHp.end();
    }

    float fallbackTrackedMaxHp(std::string_view) {
        return 1000.f;
    }

    float menuCreatureMaxHp(const BattleDemoState& battle, std::string_view name, float fallback) {
        auto it = battle.creatureMaxHp.find(std::string(name));
        if (it != battle.creatureMaxHp.end())
            return it->second;
        return fallback;
    }

    float menuCreatureHp(const BattleDemoState& battle, std::string_view name, float maxHp) {
        auto it = battle.creatureHp.find(std::string(name));
        if (it != battle.creatureHp.end())
            return it->second;
        return maxHp;
    }

    void cacheActiveCreatureStats(BattleDemoState& battle) {
        if (battle.player.name.empty())
            return;
        std::string name = battle.player.name;
        battle.creatureMaxHp[name] = battle.player.maxHp;
        battle.creatureHp[name] = std::clamp(battle.player.hp, 0.f, battle.player.maxHp);
    }

    void applyDragonbornLevel(Game& game, BattleDemoState& battle, int level, bool healToFull) {
        int desiredLevel = std::max(level, kDragonbornBaseLevel);
        if (desiredLevel < battle.dragonbornLevel)
            desiredLevel = battle.dragonbornLevel;
        battle.dragonbornLevel = desiredLevel;
        std::string heroName = heroDisplayName(game);
        float newMaxHp = dragonbornMaxHpForLevel(desiredLevel);
        if (battle.currentDragonbornActive) {
            battle.player.level = desiredLevel;
            battle.player.maxHp = newMaxHp;
            if (healToFull) {
                battle.player.hp = newMaxHp;
            } else {
                battle.player.hp = std::min(battle.player.hp, newMaxHp);
            }
            battle.playerDisplayedHp = battle.player.hp;
            cacheActiveCreatureStats(battle);
        }
        battle.creatureMaxHp[heroName] = newMaxHp;
        if (healToFull) {
            battle.creatureHp[heroName] = newMaxHp;
        } else {
            auto it = battle.creatureHp.find(heroName);
            if (it == battle.creatureHp.end()) {
                battle.creatureHp[heroName] = newMaxHp;
            } else {
                it->second = std::min(it->second, newMaxHp);
            }
        }
    }

    void maybeRecallDragonborn(Game& game, BattleDemoState& battle) {
        if (!allTrackedCreaturesDefeated(battle))
            return;
        std::string heroName = heroDisplayName(game);
        if (battle.defeatedCreatures.erase(heroName) == 0)
            return;
        applyDragonbornLevel(game, battle, 100, true);
    }

    bool allTrackedCreaturesDefeated(const BattleDemoState& battle) {
        for (const auto& tracked : kTrackedCreatureNames) {
            if (battle.defeatedCreatures.find(std::string(tracked)) == battle.defeatedCreatures.end())
                return false;
        }
        return true;
    }

    sf::Color skillHighlightColor(std::string_view skillName) {
        if (skillName == "Air Slash")
            return sf::Color(120, 220, 180);
        if (skillName == "Earth Slash")
            return sf::Color(153, 102, 51);
        if (skillName == "Fire Slash")
            return sf::Color(255, 85, 85);
        if (skillName == "Water Slash")
            return sf::Color(120, 220, 255);
        if (skillName == "Friendship")
            return ColorHelper::Palette::PurpleBlue;
        if (isMasterBatesSkill(skillName))
            return ColorHelper::Palette::DarkPurple;
        return sf::Color::White;
    }

    const sf::SoundBuffer* masterBatesSkillSound(const Game& game, std::string_view skillName) {
        if (skillName == "Midnight Release")
            return &game.resources.skillMidnightRelease;
        if (skillName == "Bad Habit")
            return &game.resources.skillBadHabit;
        if (skillName == "Shadow Routine")
            return &game.resources.skillShadowRoutine;
        if (skillName == "Guilty Ember")
            return &game.resources.skillGuiltyEmber;
        return nullptr;
    }

    const sf::Texture* masterBatesSkillTexture(const Game& game, std::string_view skillName) {
        if (skillName == "Midnight Release")
            return &game.resources.skillMasterBatesMidnightRelease;
        if (skillName == "Bad Habit")
            return &game.resources.skillMasterBatesBadHabit;
        if (skillName == "Shadow Routine")
            return &game.resources.skillMasterBatesShadowRoutine;
        if (skillName == "Guilty Ember")
            return &game.resources.skillMasterBatesGuiltyEmber;
        return nullptr;
    }

    std::vector<std::pair<std::string, sf::Color>> buildLogHighlightTokens(
        const Game& game,
        const BattleDemoState& battle,
        std::string_view message
    ) {
        std::vector<std::pair<std::string, sf::Color>> tokens;
        auto addToken = [&](std::string token, sf::Color color) {
            if (token.empty() || message.find(token) == std::string_view::npos)
                return;
            auto alreadyAdded = std::any_of(tokens.begin(), tokens.end(), [&](const auto& pair) {
                return pair.first == token;
            });
            if (alreadyAdded)
                return;
            tokens.emplace_back(std::move(token), color);
        };
        addToken(displayPlayerName(game, battle), ColorHelper::Palette::SoftRed);
        if (!battle.enemy.name.empty()) {
            sf::Color enemyColor = isMasterBatesName(battle.enemy.name)
                ? ColorHelper::Palette::DarkPurple
                : ColorHelper::Palette::PurpleBlue;
            addToken(battle.enemy.name, enemyColor);
        }
        auto addSkillToken = [&](const std::optional<std::string>& skill) {
            if (skill)
                addToken(*skill, skillHighlightColor(*skill));
        };
        for (const auto& skill : battle.player.skills)
            addSkillToken(skill);
        for (const auto& skill : battle.enemy.skills)
            addSkillToken(skill);
        return tokens;
    }

    void pushLog(Game& game, std::string entry,
                 std::vector<std::pair<std::string, sf::Color>> extraTokens = {}) {
        auto& battle = game.battleDemo;
        auto& log = battle.logHistory;
        log.clear();
        BattleDemoState::LogEntry logEntry;
        logEntry.message = std::move(entry);
        logEntry.highlightTokens = buildLogHighlightTokens(game, battle, logEntry.message);
        for (auto& extra : extraTokens) {
            bool alreadyAdded = std::any_of(logEntry.highlightTokens.begin(), logEntry.highlightTokens.end(),
                [&](const auto& pair) {
                    return pair.first == extra.first;
                });
            if (!alreadyAdded) {
                logEntry.highlightTokens.emplace_back(std::move(extra));
            }
        }
        log.push_back(std::move(logEntry));
    }

    BattleDemoState::SkillElement skillElementFromName(const std::string& skillName) {
        std::string normalized = skillName;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        if (normalized.find("air") != std::string::npos)
            return BattleDemoState::SkillElement::Air;
        if (normalized.find("earth") != std::string::npos)
            return BattleDemoState::SkillElement::Earth;
        if (normalized.find("fire") != std::string::npos)
            return BattleDemoState::SkillElement::Fire;
        if (normalized.find("water") != std::string::npos)
            return BattleDemoState::SkillElement::Water;
        return BattleDemoState::SkillElement::None;
    }

    const sf::Texture* skillSlashTextureFor(const Game& game, BattleDemoState::SkillElement element) {
        switch (element) {
            case BattleDemoState::SkillElement::Air:
                return &game.resources.skillSlashAir;
            case BattleDemoState::SkillElement::Earth:
                return &game.resources.skillSlashEarth;
            case BattleDemoState::SkillElement::Fire:
                return &game.resources.skillSlashFire;
            case BattleDemoState::SkillElement::Water:
                return &game.resources.skillSlashWater;
            default:
                return nullptr;
        }
    }

    const sf::Texture* skillWeaponTextureFor(const Game& game, BattleDemoState::SkillElement element) {
        switch (element) {
            case BattleDemoState::SkillElement::Air:
                return &game.resources.skillWeaponAir;
            case BattleDemoState::SkillElement::Earth:
                return &game.resources.skillWeaponEarth;
            case BattleDemoState::SkillElement::Fire:
                return &game.resources.skillWeaponFire;
            case BattleDemoState::SkillElement::Water:
                return &game.resources.skillWeaponWater;
            default:
                return nullptr;
        }
    }

    const sf::SoundBuffer* skillElementSoundFor(const Game& game, BattleDemoState::SkillElement element) {
        switch (element) {
            case BattleDemoState::SkillElement::Air:
                return &game.resources.skillAir;
            case BattleDemoState::SkillElement::Earth:
                return &game.resources.skillEarth;
            case BattleDemoState::SkillElement::Fire:
                return &game.resources.skillFire;
            case BattleDemoState::SkillElement::Water:
                return &game.resources.skillWater;
            default:
                return nullptr;
        }
    }
    float dragonbornLevelRatio(int level) {
        if (level <= 0)
            level = kDragonbornBaseLevel;
        return static_cast<float>(level) / static_cast<float>(kDragonbornBaseLevel);
    }

    float dragonbornMaxHpForLevel(int level) {
        return kDragonbornBaseMaxHp * dragonbornLevelRatio(level);
    }

    float dragonbornSkillDamageForLevel(int level, int skillIndex) {
        if (skillIndex < 0 || skillIndex >= static_cast<int>(BattleDemoState::kSkillSlotCount))
            return 0.f;
        return kDragonbornSkillDamageLevel50[skillIndex] * dragonbornLevelRatio(level);
    }

    void startHpPulse(BattleDemoState::HpPulse& pulse, float from, float to) {
        pulse.active = true;
        pulse.startHp = from;
        pulse.endHp = to;
        pulse.clock.restart();
    }

    bool isDragonbornActive(const BattleDemoState& battle) {
        return battle.currentDragonbornActive;
    }

    void finalizeForcedRetreat(Game& game) {
        auto& battle = game.battleDemo;
        if (!battle.forcedRetreat.awaitingSwap)
            return;
        cacheActiveCreatureStats(battle);
        battle.forcedRetreat.awaitingSwap = false;
        battle.forcedRetreat.active = true;
        if (!battle.forcedRetreat.pendingName.empty()) {
            battle.forcedRetreat.blockedName = battle.forcedRetreat.pendingName;
            battle.forcedRetreat.pendingName.clear();
        } else {
            battle.forcedRetreat.blockedName = battle.player.name;
        }
        battle.playerHpPulse.active = false;
        battle.reopenMenuAfterPlayerPulse = false;
        battle.actionMenuVisible = false;
        battle.fightMenuVisible = false;
        battle.fightCancelHighlight = false;
        if (battle.currentDragonbornActive) {
            std::string heroName = heroDisplayName(game);
            battle.defeatedCreatures.insert(heroName);
            battle.creatureHp[heroName] = 0.f;
            std::string heroLabel = displayPlayerName(game, battle);
            pushLog(
                game,
                heroLabel + " has miraculously survived!\n"
                    + "But " + heroLabel + " has to retreat for a while..."
            );
        } else {
            pushLog(game, battle.player.name + " has been defeated! Choose your next Glandumon.");
        }
        maybeRecallDragonborn(game, battle);
        battle.creatureMenuAllowCancel = false;
        openCreatureMenu(battle, BattleDemoState::CreatureMenuType::Glandumon, false);
    }

    void maybeFinalizeForcedRetreat(Game& game) {
        auto& battle = game.battleDemo;
        if (!battle.forcedRetreat.awaitingSwap)
            return;
        if (battle.playerHpPulse.active)
            return;
        if (battle.skillEffect.active || battle.friendshipEffect.active || battle.masterBatesSkillEffect.active)
            return;
        finalizeForcedRetreat(game);
    }

    void markActiveCreatureDefeated(BattleDemoState& battle) {
        if (!battle.currentDragonbornActive && battle.player.hp <= 0.f) {
            battle.defeatedCreatures.insert(battle.player.name);
            battle.creatureHp[battle.player.name] = 0.f;
        }
    }

    struct CreatureMenuEntry {
        BattleDemoState::Combatant combatant;
        const sf::Texture* sprite = nullptr;
        const sf::Texture* backSprite = nullptr;
        Game::DragonbornGender gender = Game::DragonbornGender::Male;
        bool showBothGenders = false;
        bool isDragonborn = false;
        bool disabled = false;
        bool defeated = false;
    };

    int normalizeCreatureSelection(BattleDemoState& battle, const std::vector<CreatureMenuEntry>& entries) {
        if (entries.empty()) {
            battle.creatureMenuSelection = 0;
            return -1;
        }
        int clamped = std::clamp(battle.creatureMenuSelection, 0, static_cast<int>(entries.size()) - 1);
        if (!entries[clamped].disabled)
            return clamped;
        for (int idx = 0; idx < static_cast<int>(entries.size()); ++idx) {
            if (!entries[idx].disabled) {
                battle.creatureMenuSelection = idx;
                return idx;
            }
        }
        battle.creatureMenuSelection = clamped;
        return clamped;
    }

    bool actionMenuReady(const BattleDemoState& battle) {
        return !battle.skillEffect.active
            && !battle.friendshipEffect.active
            && !battle.playerHpPulse.active
            && !battle.enemyHpPulse.active
            && !battle.masterBatesEvolution.active
            && !battle.masterBatesSkillEffect.active;
    }

    void completeMasterBatesEvolution(Game& game, BattleDemoState& battle) {
        auto& evolution = battle.masterBatesEvolution;
        evolution.stage = BattleDemoState::MasterBatesEvolution::Stage::ChickSound;
        evolution.timer = 0.f;
        evolution.tint = sf::Color::White;
        evolution.shakeOffset = { 0.f, 0.f };
        evolution.shakeTimer = 0.f;
        evolution.active = true;
        evolution.sound.reset();
        evolution.sound.emplace(game.resources.chickSound);
        evolution.sound->play();
        battle.masterBatesDragonActive = true;
        battle.enemy.name = "Master Bates (Dragon)";
        battle.enemy.level = std::max(1, battle.enemy.level * 2);
        float newMaxHp = std::max(1.f, battle.enemy.maxHp * kMasterBatesDragonMultiplier);
        newMaxHp += 2000.f;
        battle.enemy.maxHp = newMaxHp;
        battle.enemy.hp = newMaxHp;
        battle.enemyDisplayedHp = newMaxHp;
        battle.enemy.skills = {{
            std::optional<std::string>{"Shadow Routine"},
            std::optional<std::string>{"Guilty Ember"},
            std::nullopt,
            std::nullopt
        }};
        battle.enemyMoveIndex = 0;
        battle.enemyHpPulse.active = false;
        battle.phase = BattleDemoState::Phase::EnemyAction;
        battle.actionTimer = 0.f;
        battle.actionMenuVisible = false;
        battle.reopenMenuAfterPlayerPulse = false;
        pushLog(game, "Master Bates turned himself into a dragon!");
    }

    void startMasterBatesEvolution(Game& game, BattleDemoState& battle) {
        auto& evolution = battle.masterBatesEvolution;
        if (evolution.active || battle.masterBatesDragonActive)
            return;
        evolution.active = true;
        evolution.stage = BattleDemoState::MasterBatesEvolution::Stage::FirstSound;
        pushLog(game, "Is he dead already...?");
        evolution.timer = 0.f;
        evolution.tint = sf::Color::White;
        evolution.shakeOffset = { 0.f, 0.f };
        evolution.shakeTimer = 0.f;
        evolution.sound.emplace(game.resources.masterBatesEvolution);
        evolution.sound->play();
        battle.actionTimer = 0.f;
        battle.actionMenuVisible = false;
        battle.fightMenuVisible = false;
        battle.reopenMenuAfterPlayerPulse = false;
    }

    bool resolveEnemyDefeat(Game& game, BattleDemoState& battle) {
        if (battle.enemy.hp > 0.f)
            return false;
        if (!battle.masterBatesDragonActive && isMasterBatesName(battle.enemy.name)) {
            startMasterBatesEvolution(game, battle);
            return true;
        }
        battle.phase = BattleDemoState::Phase::Victory;
        battle.victoryTransitioned = false;
        battle.completionClock.restart();
        return true;
    }

    void startSkillEffect(Game& game, BattleDemoState& battle,
                          BattleDemoState::SkillElement element,
                          BattleDemoState::SkillEffect::Target target) {
        if (element == BattleDemoState::SkillElement::None)
            return;
        auto& effect = battle.skillEffect;
        effect.active = true;
        effect.phase = BattleDemoState::SkillEffect::Phase::Slash;
        effect.target = target;
        effect.element = element;
        effect.blinkTimer = 0.f;
        effect.blinkCycles = 0;
        effect.slashVisible = true;
        effect.fadeTimer = 0.f;
        effect.weaponFade = 1.f;
        effect.pendingDamage = false;
        effect.pendingHp = 0.f;
        effect.slashSound.reset();
        effect.elementSound.reset();
        effect.slashSound.emplace(game.resources.skillSlash);
        effect.slashSound->play();
    }

    void updateSkillEffect(Game& game, BattleDemoState& battle, float deltaSeconds) {
        auto& effect = battle.skillEffect;
        if (!effect.active)
            return;
        if (effect.phase == BattleDemoState::SkillEffect::Phase::Slash) {
            effect.blinkTimer += deltaSeconds;
            if (effect.blinkTimer >= kSkillSlashBlinkInterval) {
                effect.blinkTimer -= kSkillSlashBlinkInterval;
                effect.slashVisible = !effect.slashVisible;
                if (effect.slashVisible)
                    ++effect.blinkCycles;
            }
            if (effect.blinkCycles >= kSkillSlashBlinkCycles && effect.slashVisible) {
                effect.phase = BattleDemoState::SkillEffect::Phase::Weapon;
                effect.blinkTimer = 0.f;
                effect.blinkCycles = 0;
                effect.slashVisible = false;
                if (const sf::SoundBuffer* buffer = skillElementSoundFor(game, effect.element)) {
                    effect.elementSound.emplace(*buffer);
                    effect.elementSound->play();
                } else {
                    effect.phase = BattleDemoState::SkillEffect::Phase::FadeOut;
                    effect.fadeTimer = 0.f;
                    effect.weaponFade = 1.f;
                }
            }
        } else if (effect.phase == BattleDemoState::SkillEffect::Phase::Weapon) {
            if (effect.elementSound && effect.elementSound->getStatus() == sf::Sound::Status::Stopped) {
                effect.phase = BattleDemoState::SkillEffect::Phase::FadeOut;
                effect.fadeTimer = 0.f;
                effect.weaponFade = 1.f;
            }
        }
        if (effect.phase == BattleDemoState::SkillEffect::Phase::FadeOut) {
            effect.fadeTimer += deltaSeconds;
            float ratio = std::clamp(effect.fadeTimer / kSkillWeaponFadeDuration, 0.f, 1.f);
            effect.weaponFade = 1.f - ratio;
            if (ratio >= 1.f) {
                if (effect.pendingDamage) {
                    effect.pendingDamage = false;
                    if (effect.target == BattleDemoState::SkillEffect::Target::Enemy) {
                        battle.enemy.hp = effect.pendingHp;
                        startHpPulse(battle.enemyHpPulse, battle.enemyDisplayedHp, effect.pendingHp);
                        resolveEnemyDefeat(game, battle);
                        } else if (effect.target == BattleDemoState::SkillEffect::Target::Player) {
                        battle.player.hp = effect.pendingHp;
                        startHpPulse(battle.playerHpPulse, battle.playerDisplayedHp, effect.pendingHp);
                        cacheActiveCreatureStats(battle);
                        markActiveCreatureDefeated(battle);
                        maybeRecallDragonborn(game, battle);
                        maybeFinalizeForcedRetreat(game);
                    }
                }
                effect = BattleDemoState::SkillEffect();
            }
        }
    }

    std::optional<sf::Color> computeSwapOverlayColor(const BattleDemoState& battle) {
        if (!battle.swapAnimation.active)
            return std::nullopt;
        using Stage = BattleDemoState::SwapAnimation::Stage;
        float progress = std::clamp(battle.swapAnimation.timer / kSwapPhaseDuration, 0.f, 1.f);
        sf::Color color{ sf::Color::White };
        float alpha = 0.f;
        switch (battle.swapAnimation.stage) {
            case Stage::OutgoingWhite:
                alpha = kSwapOverlayMaxAlpha * progress;
                break;
            case Stage::OutgoingSoftRed:
                color = ColorHelper::Palette::SoftRed;
                alpha = kSwapOverlayMaxAlpha * progress;
                break;
            case Stage::IncomingSoftRedToWhite:
                color = ColorHelper::tint(ColorHelper::Palette::SoftRed, sf::Color::White, progress);
                alpha = kSwapOverlayMaxAlpha;
                break;
            case Stage::IncomingWhiteToNormal:
                alpha = kSwapOverlayMaxAlpha * (1.f - progress);
                break;
            default:
                return std::nullopt;
        }
        if (alpha <= 0.f)
            return std::nullopt;
        color.a = static_cast<std::uint8_t>(std::clamp(alpha, 0.f, 255.f));
        return color;
    }

    void drawSwapPrompt(Game& game, sf::RenderTarget& target,
                        const sf::Vector2f& viewportSize, const BattleDemoState& battle) {
        if (!battle.swapPrompt.active)
            return;
        SwapPromptLayout layout = computeSwapPromptLayout(viewportSize);
        RoundedRectangleShape frame({ layout.panel.size.x, layout.panel.size.y }, 24.f, 20);
        frame.setPosition(layout.panel.position);
        frame.setFillColor(sf::Color(18, 20, 28, 220));
        frame.setOutlineThickness(2.f);
        frame.setOutlineColor(ColorHelper::Palette::SoftRed);
        target.draw(frame);

        constexpr float kTextPadding = 20.f;
        sf::Text promptText{ game.resources.battleFont, "Swap with this Glandumon?", 26 };
        promptText.setStyle(sf::Text::Bold);
        promptText.setLetterSpacing(promptText.getLetterSpacing() + 0.5f);
        promptText.setFillColor(sf::Color::White);
        promptText.setPosition({
            layout.panel.position.x + kTextPadding,
            layout.panel.position.y + kTextPadding
        });
        target.draw(promptText);

        sf::Text candidateText{ game.resources.battleFont, battle.swapPrompt.candidate.name, 20 };
        candidateText.setLetterSpacing(candidateText.getLetterSpacing() + 0.25f);
        candidateText.setFillColor(ColorHelper::Palette::SoftRed);
        candidateText.setPosition({
            layout.panel.position.x + kTextPadding,
            promptText.getPosition().y + promptText.getGlobalBounds().size.y + 8.f
        });
        target.draw(candidateText);

        auto drawButton = [&](const sf::FloatRect& bounds, const std::string& label,
                              bool active, const sf::Color& highlight) {
            RoundedRectangleShape button({ bounds.size.x, bounds.size.y }, bounds.size.y * 0.35f, 16);
            button.setPosition(bounds.position);
            button.setFillColor(active ? highlight : sf::Color(32, 34, 42));
            button.setOutlineThickness(1.5f);
            button.setOutlineColor(active ? sf::Color::White : sf::Color(160, 160, 170));
            target.draw(button);

            sf::Text labelText{ game.resources.battleFont, label, 24 };
            labelText.setLetterSpacing(labelText.getLetterSpacing() + 0.25f);
            labelText.setFillColor(sf::Color::White);
            auto textBounds = labelText.getLocalBounds();
            labelText.setOrigin({
                textBounds.position.x + (textBounds.size.x * 0.5f),
                textBounds.position.y + (textBounds.size.y * 0.5f)
            });
            labelText.setPosition({
                bounds.position.x + (bounds.size.x * 0.5f),
                bounds.position.y + (bounds.size.y * 0.5f)
            });
            target.draw(labelText);
        };

        drawButton(layout.yesBounds, "YES", battle.swapPrompt.selectedButton == 0, ColorHelper::Palette::SoftRed);
        drawButton(layout.noBounds, "NO", battle.swapPrompt.selectedButton == 1, ColorHelper::Palette::Purple);
    }

    void startSwapPrompt(BattleDemoState& battle,
                         BattleDemoState::Combatant candidate,
                         const sf::Texture* backSprite,
                         bool isDragonborn) {
        battle.swapPrompt.active = true;
        battle.swapPrompt.selectedButton = 0;
        battle.swapPrompt.candidate = std::move(candidate);
        battle.swapPrompt.backSprite = backSprite;
        battle.swapPrompt.candidateIsDragonborn = isDragonborn;
    }

    void beginPlayerSwap(Game& game) {
        auto& battle = game.battleDemo;
        if (!battle.swapPrompt.active)
            return;
        cacheActiveCreatureStats(battle);
        battle.creatureMenuVisible = false;
        battle.creatureMenuType = BattleDemoState::CreatureMenuType::None;
        battle.creatureMenuSelection = 0;
        battle.creatureMenuEntryBounds.clear();
        battle.creatureMenuCancelBounds = {};
        battle.phase = BattleDemoState::Phase::PlayerAction;
        battle.actionTimer = 0.f;
        battle.actionMenuVisible = false;
        battle.fightMenuVisible = false;
        battle.fightCancelHighlight = false;
        battle.reopenMenuAfterPlayerPulse = false;
        battle.swapAnimation.active = true;
        battle.swapAnimation.stage = BattleDemoState::SwapAnimation::Stage::OutgoingWhite;
        battle.swapAnimation.timer = 0.f;
        battle.swapAnimation.pendingCombatant = battle.swapPrompt.candidate;
        battle.swapAnimation.pendingBackSprite = battle.swapPrompt.backSprite;
        battle.swapAnimation.pendingIsDragonborn = battle.swapPrompt.candidateIsDragonborn;
        battle.swapPrompt.active = false;
    }

    void updateSwapAnimation(Game& game, BattleDemoState& battle, float deltaSeconds) {
        auto& animation = battle.swapAnimation;
        if (!animation.active)
            return;
        animation.timer += deltaSeconds;
        using Stage = BattleDemoState::SwapAnimation::Stage;
        while (animation.active && animation.timer >= kSwapPhaseDuration) {
            animation.timer -= kSwapPhaseDuration;
            switch (animation.stage) {
                case Stage::OutgoingWhite:
                    animation.stage = Stage::OutgoingSoftRed;
                    break;
                case Stage::OutgoingSoftRed: {
                    std::string currentName = battle.player.name;
                    std::string swappedName = animation.pendingCombatant.name;
                    battle.player = animation.pendingCombatant;
                    if (animation.pendingIsDragonborn) {
                        for (int idx = 0; idx < static_cast<int>(BattleDemoState::kSkillSlotCount); ++idx) {
                            battle.player.skills[idx] = std::optional<std::string>{ kPlayerSkillNames[idx] };
                        }
                    }
                    const sf::Texture* fallbackBack = (game.playerGender == Game::DragonbornGender::Female)
                        ? &game.resources.spriteDragonbornFemaleBack
                        : &game.resources.spriteDragonbornMaleBack;
                    const sf::Texture* newBack = animation.pendingBackSprite
                        ? animation.pendingBackSprite
                        : fallbackBack;
                    battle.playerBackSprite = newBack;
                    battle.playerDisplayedHp = battle.player.hp;
                    battle.playerHpPulse.active = false;
                    cacheActiveCreatureStats(battle);
                    battle.currentDragonbornActive = animation.pendingIsDragonborn;
                    if (animation.pendingIsDragonborn) {
                        bool levelUp = allTrackedCreaturesDefeated(battle) && battle.dragonbornLevel < 100;
                        int targetLevel = levelUp ? 100 : battle.dragonbornLevel;
                        applyDragonbornLevel(game, battle, targetLevel, levelUp);
                    }
                    std::string swapLog = "Well done, " + currentName + "! Come back!\n"
                        + swappedName + " took the field!";
                    pushLog(
                        game,
                        swapLog,
                        { { currentName, ColorHelper::Palette::SoftRed } }
                    );
                    animation.stage = Stage::IncomingSoftRedToWhite;
                } break;
                case Stage::IncomingSoftRedToWhite:
                    animation.stage = Stage::IncomingWhiteToNormal;
                    break;
                case Stage::IncomingWhiteToNormal:
                    animation.active = false;
                    animation.stage = Stage::Inactive;
                    animation.pendingBackSprite = nullptr;
                    animation.pendingCombatant = BattleDemoState::Combatant();
                    animation.pendingIsDragonborn = false;
                    battle.phase = BattleDemoState::Phase::EnemyAction;
                    battle.actionTimer = 0.f;
                    battle.actionMenuVisible = false;
                    battle.reopenMenuAfterPlayerPulse = false;
                    animation.timer = 0.f;
                    break;
                default:
                    animation.active = false;
                    animation.pendingIsDragonborn = false;
                    break;
            }
        }
    }

    void startMasterBatesSkillEffect(Game& game, BattleDemoState& battle,
                                     BattleDemoState::MasterBatesSkillEffect::Target target,
                                     std::string_view skillName,
                                     float pendingHp) {
        auto& effect = battle.masterBatesSkillEffect;
        effect.active = true;
        effect.stage = BattleDemoState::MasterBatesSkillEffect::Stage::Powerup;
        effect.target = target;
        effect.powerupTexture = &game.resources.skillMasterBatesPowerup;
        effect.skillTexture = masterBatesSkillTexture(game, skillName);
        effect.skillSoundBuffer = masterBatesSkillSound(game, skillName);
        effect.pendingDamage = true;
        effect.pendingHp = pendingHp;
        effect.sound.reset();
        effect.sound.emplace(game.resources.skillPowerUp);
        effect.sound->play();
    }

    void updateMasterBatesSkillEffect(Game& game, BattleDemoState& battle, float) {
        auto& effect = battle.masterBatesSkillEffect;
        if (!effect.active)
            return;
        bool finished = true;
        if (effect.sound) {
            finished = (effect.sound->getStatus() == sf::Sound::Status::Stopped);
        }
        if (!finished)
            return;
        if (effect.stage == BattleDemoState::MasterBatesSkillEffect::Stage::Powerup) {
            effect.stage = BattleDemoState::MasterBatesSkillEffect::Stage::Skill;
            effect.sound.reset();
            if (effect.skillSoundBuffer) {
                effect.sound.emplace(*effect.skillSoundBuffer);
                effect.sound->play();
                return;
            }
        }
        if (effect.stage == BattleDemoState::MasterBatesSkillEffect::Stage::Skill) {
                if (effect.pendingDamage) {
                    effect.pendingDamage = false;
                    if (effect.target == BattleDemoState::MasterBatesSkillEffect::Target::Enemy) {
                        battle.enemy.hp = effect.pendingHp;
                        startHpPulse(battle.enemyHpPulse, battle.enemyDisplayedHp, effect.pendingHp);
                        if (battle.enemy.hp <= 0.f) {
                            resolveEnemyDefeat(game, battle);
                        }
                    } else {
                        battle.player.hp = effect.pendingHp;
                        startHpPulse(battle.playerHpPulse, battle.playerDisplayedHp, effect.pendingHp);
                        cacheActiveCreatureStats(battle);
                        markActiveCreatureDefeated(battle);
                        maybeRecallDragonborn(game, battle);
                        maybeFinalizeForcedRetreat(game);
                    }
                }
            effect.active = false;
            effect.stage = BattleDemoState::MasterBatesSkillEffect::Stage::Powerup;
            effect.powerupTexture = nullptr;
            effect.skillTexture = nullptr;
            effect.skillSoundBuffer = nullptr;
            effect.sound.reset();
        }
    }

    void updateMasterBatesEvolution(Game& game, BattleDemoState& battle, float deltaSeconds) {
        auto& evolution = battle.masterBatesEvolution;
        if (!evolution.active)
            return;
        if (evolution.stage == BattleDemoState::MasterBatesEvolution::Stage::ChickSound) {
            if (evolution.sound && evolution.sound->getStatus() == sf::Sound::Status::Stopped) {
                evolution.stage = BattleDemoState::MasterBatesEvolution::Stage::AwwSound;
                evolution.sound.reset();
                evolution.sound.emplace(game.resources.awwSound);
                evolution.sound->play();
            }
            return;
        }
        if (evolution.stage == BattleDemoState::MasterBatesEvolution::Stage::AwwSound) {
            if (evolution.sound && evolution.sound->getStatus() == sf::Sound::Status::Stopped) {
                evolution.active = false;
                evolution.stage = BattleDemoState::MasterBatesEvolution::Stage::Idle;
                evolution.sound.reset();
                battle.actionTimer = 0.f;
            }
            return;
        }
        if (evolution.stage == BattleDemoState::MasterBatesEvolution::Stage::FirstSound) {
            if (evolution.sound && evolution.sound->getStatus() == sf::Sound::Status::Stopped) {
                evolution.stage = BattleDemoState::MasterBatesEvolution::Stage::FadeWhite;
                evolution.timer = 0.f;
                evolution.tint = ColorHelper::Palette::SoftRed;
                pushLog(game, "Master Bates is evolving!");
                if (evolution.sound)
                    evolution.sound->play();
            }
            return;
        }

        evolution.timer += deltaSeconds;
        auto applyShake = [&]() {
            evolution.shakeTimer += deltaSeconds;
            if (evolution.shakeTimer >= evolution.shakeInterval) {
                evolution.shakeTimer -= evolution.shakeInterval;
                std::uniform_real_distribution<float> dist(-evolution.shakeStrength, evolution.shakeStrength);
                evolution.shakeOffset.x = dist(battle.rng);
                evolution.shakeOffset.y = dist(battle.rng);
            }
        };

        if (evolution.stage == BattleDemoState::MasterBatesEvolution::Stage::FadeWhite) {
            float ratio = std::clamp(evolution.timer / evolution.whiteDuration, 0.f, 1.f);
            evolution.tint = ColorHelper::tint(ColorHelper::Palette::SoftRed, sf::Color::White, ratio);
            applyShake();
            if (ratio >= 1.f) {
                evolution.stage = BattleDemoState::MasterBatesEvolution::Stage::FadePurple;
                evolution.timer = 0.f;
            }
        } else if (evolution.stage == BattleDemoState::MasterBatesEvolution::Stage::FadePurple) {
            float ratio = std::clamp(evolution.timer / evolution.purpleDuration, 0.f, 1.f);
            evolution.tint = ColorHelper::tint(sf::Color::White, ColorHelper::Palette::DarkPurple, ratio);
            applyShake();
            if (ratio >= 1.f) {
                completeMasterBatesEvolution(game, battle);
            }
        }
    }

    void startFriendshipEffect(Game& game, BattleDemoState& battle,
                               BattleDemoState::FriendshipEffect::Target target,
                               const sf::Vector2f& startCenter,
                               const sf::Vector2f& targetCenter) {
        auto& effect = battle.friendshipEffect;
        effect.active = true;
        effect.target = target;
        effect.phase = BattleDemoState::FriendshipEffect::Phase::Standing;
        effect.timer = 0.f;
        effect.fade = 1.f;
        effect.hpPulseTriggered = false;
        effect.pendingDamage = false;
        effect.pendingHp = 0.f;
        constexpr std::array<sf::Vector2f, 3> layout = {
            sf::Vector2f{ -96.f, 0.f },
            sf::Vector2f{ 0.f, 0.f },
            sf::Vector2f{ 96.f, 0.f }
        };
        auto textureSet = isDragonName(battle.player.name)
            ? friendshipDragonTextures(game, battle.player.name)
            : friendshipNpcTextures(game, battle.player.name);
        for (int i = 0; i < static_cast<int>(effect.textures.size()); ++i) {
            effect.textures[i] = textureSet[i];
            effect.offsets[i] = layout[i];
        }
        effect.startCenter = startCenter;
        effect.targetCenter = targetCenter;
        effect.sound.emplace(game.resources.skillFriendship);
        effect.sound->play();
    }

    void updateFriendshipEffect(Game& game, BattleDemoState& battle, float delta) {
        auto& effect = battle.friendshipEffect;
        if (!effect.active)
            return;
        effect.timer += delta;
        if (effect.phase == BattleDemoState::FriendshipEffect::Phase::Standing) {
            if (effect.timer >= effect.standDuration) {
                effect.timer -= effect.standDuration;
                effect.phase = BattleDemoState::FriendshipEffect::Phase::Throwing;
            }
            effect.fade = 1.f;
            return;
        }
        if (effect.phase == BattleDemoState::FriendshipEffect::Phase::Throwing) {
            float ratio = std::clamp(effect.timer / effect.throwDuration, 0.f, 1.f);
            effect.fade = 1.f - ratio;
            if (effect.timer >= effect.throwDuration) {
                if (!effect.hpPulseTriggered && effect.pendingDamage) {
                effect.hpPulseTriggered = true;
                effect.pendingDamage = false;
                if (effect.target == BattleDemoState::FriendshipEffect::Target::Enemy) {
                    battle.enemy.hp = effect.pendingHp;
                    startHpPulse(battle.enemyHpPulse, battle.enemyDisplayedHp, effect.pendingHp);
                    resolveEnemyDefeat(game, battle);
                } else if (effect.target == BattleDemoState::FriendshipEffect::Target::Player) {
                    battle.player.hp = effect.pendingHp;
                    startHpPulse(battle.playerHpPulse, battle.playerDisplayedHp, effect.pendingHp);
                    cacheActiveCreatureStats(battle);
                    markActiveCreatureDefeated(battle);
                    maybeRecallDragonborn(game, battle);
                    maybeFinalizeForcedRetreat(game);
                }
            }
                effect.active = false;
                effect.timer = 0.f;
                effect.target = BattleDemoState::FriendshipEffect::Target::None;
                effect.sound.reset();
            }
        }
    }

    void drawFriendshipEffect(Game& game, const BattleDemoState& battle, sf::RenderTarget& target) {
        const auto& effect = battle.friendshipEffect;
        if (!effect.active)
            return;
        float progress = 0.f;
        if (effect.phase == BattleDemoState::FriendshipEffect::Phase::Throwing) {
            progress = std::clamp(effect.timer / effect.throwDuration, 0.f, 1.f);
        }
        float fadeAlpha = (effect.phase == BattleDemoState::FriendshipEffect::Phase::Throwing)
            ? std::clamp(effect.fade * 255.f, 0.f, 255.f)
            : 255.f;
        constexpr float kFriendshipSpriteBaseScale = 0.5f;
        auto baseSize = game.resources.spriteWandaRinn.getSize();
        sf::Vector2f targetSize{
            static_cast<float>(baseSize.x) * kFriendshipSpriteTargetFactor,
            static_cast<float>(baseSize.y) * kFriendshipSpriteTargetFactor
        };
        for (int i = 0; i < static_cast<int>(effect.textures.size()); ++i) {
            const sf::Texture* texture = effect.textures[i];
            if (!texture)
                continue;
            sf::Sprite sprite(*texture);
            auto bounds = sprite.getLocalBounds();
            sprite.setOrigin(sf::Vector2f{
                bounds.position.x + (bounds.size.x * 0.5f),
                bounds.position.y + (bounds.size.y * 0.5f)
            });
            sf::Vector2f startPos = effect.startCenter + effect.offsets[i];
            sf::Vector2f targetPos = effect.targetCenter + (effect.offsets[i] * 0.35f);
            sf::Vector2f position = startPos + (targetPos - startPos) * progress;
            sprite.setPosition(position);
        float scaleX = (bounds.size.x > 0.f) ? (targetSize.x / bounds.size.x) : kFriendshipSpriteBaseScale;
        float scaleY = (bounds.size.y > 0.f) ? (targetSize.y / bounds.size.y) : kFriendshipSpriteBaseScale;
        float scale = std::max(0.01f, std::min(scaleX, scaleY));
        scale *= 0.5f;
            sprite.setScale(sf::Vector2f{ scale, scale });
            sprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(fadeAlpha)));
            target.draw(sprite);
        }
    }

    void drawMasterBatesSkillEffect(Game& game, const BattleDemoState& battle, sf::RenderTarget& target) {
        const auto& effect = battle.masterBatesSkillEffect;
        if (!effect.active)
            return;
        using Stage = BattleDemoState::MasterBatesSkillEffect::Stage;
        Stage stage = effect.stage;
        if (stage == Stage::Powerup && effect.powerupTexture) {
            sf::Sprite powerup(*effect.powerupTexture);
            auto bounds = powerup.getLocalBounds();
            powerup.setOrigin({
                bounds.position.x + (bounds.size.x * 0.5f),
                bounds.position.y + (bounds.size.y * 0.5f)
            });
            powerup.setScale(sf::Vector2f{ kMasterBatesSkillSpriteScale, kMasterBatesSkillSpriteScale });
            powerup.setPosition(battle.cachedEnemyCenter);
            powerup.setColor(sf::Color(255, 255, 255, 230));
            target.draw(powerup);
            return;
        }
        if (stage == Stage::Skill && effect.skillTexture) {
            sf::Sprite skillSprite(*effect.skillTexture);
            auto bounds = skillSprite.getLocalBounds();
            skillSprite.setOrigin({
                bounds.position.x + (bounds.size.x * 0.5f),
                bounds.position.y + (bounds.size.y * 0.5f)
            });
            skillSprite.setScale(sf::Vector2f{ kMasterBatesSkillSpriteScale, kMasterBatesSkillSpriteScale });
            sf::Vector2f targetCenter = (effect.target == BattleDemoState::MasterBatesSkillEffect::Target::Player)
                ? battle.cachedPlayerCenter
                : battle.cachedEnemyCenter;
            skillSprite.setPosition(targetCenter);
            skillSprite.setColor(sf::Color(255, 255, 255, 200));
            target.draw(skillSprite);
        }
    }

    void drawSkillEffect(Game& game, const BattleDemoState& battle, sf::RenderTarget& target,
                         const sf::Vector2f& playerCenter, const sf::Vector2f& enemyCenter,
                         const sf::FloatRect& playerPlatformBounds, const sf::FloatRect& enemyPlatformBounds) {
        const auto& effect = battle.skillEffect;
        if (!effect.active)
            return;
        const sf::Texture* texture = nullptr;
        bool shouldDrawSlash = (effect.phase == BattleDemoState::SkillEffect::Phase::Slash && effect.slashVisible);
        if (shouldDrawSlash) {
            texture = skillSlashTextureFor(game, effect.element);
        } else if (effect.phase == BattleDemoState::SkillEffect::Phase::Weapon
            || effect.phase == BattleDemoState::SkillEffect::Phase::FadeOut) {
            texture = skillWeaponTextureFor(game, effect.element);
        }
        if (!texture)
            return;
        sf::Sprite sprite(*texture);
        auto bounds = sprite.getLocalBounds();
        sprite.setOrigin(sf::Vector2f{
            bounds.position.x + (bounds.size.x * 0.5f),
            bounds.position.y + (bounds.size.y * 0.5f)
        });
        sf::Vector2f anchor = (effect.target == BattleDemoState::SkillEffect::Target::Player)
            ? playerCenter
            : enemyCenter;
        sprite.setPosition(anchor);
        const sf::FloatRect& platformBounds = (effect.target == BattleDemoState::SkillEffect::Target::Player)
            ? playerPlatformBounds
            : enemyPlatformBounds;
        if (platformBounds.size.x > 0.f && platformBounds.size.y > 0.f
            && bounds.size.x > 0.f && bounds.size.y > 0.f) {
            float targetWidth = platformBounds.size.x * 0.9f;
            float targetHeight = platformBounds.size.y * 0.9f;
            float scaleX = targetWidth / bounds.size.x;
            float scaleY = targetHeight / bounds.size.y;
            float scale = std::min(scaleX, scaleY);
            if (scale > 0.f) {
                sprite.setScale(sf::Vector2f{ scale * kSkillEffectScaleFactor, scale * kSkillEffectScaleFactor });
            }
        } else {
            float fallbackScale = 0.6f * kSkillEffectScaleFactor;
            sprite.setScale(sf::Vector2f{ fallbackScale, fallbackScale });
        }
        if (effect.phase == BattleDemoState::SkillEffect::Phase::FadeOut) {
            std::uint8_t fadeAlpha = static_cast<std::uint8_t>(std::clamp(effect.weaponFade * 255.f, 0.f, 255.f));
            sprite.setColor(sf::Color(255, 255, 255, fadeAlpha));
        }
        target.draw(sprite);
    }

    std::vector<CreatureMenuEntry> buildCreatureMenuEntries(const Game& game, BattleDemoState::CreatureMenuType type) {
        std::vector<CreatureMenuEntry> entries;
        const auto& battle = game.battleDemo;
        auto makeCombatant = [&](std::string name) {
            BattleDemoState::Combatant combatant;
            combatant.name = std::move(name);
            combatant.level = 100;
            float maxHp = menuCreatureMaxHp(battle, combatant.name, fallbackTrackedMaxHp(combatant.name));
            combatant.maxHp = maxHp;
            combatant.hp = menuCreatureHp(battle, combatant.name, maxHp);
            return combatant;
        };
        auto isDefeated = [&](const std::string& name) {
            return battle.defeatedCreatures.find(name) != battle.defeatedCreatures.end();
        };
        auto isBlocked = [&](const std::string& name) {
            return battle.forcedRetreat.active
                && !battle.forcedRetreat.blockedName.empty()
                && battle.forcedRetreat.blockedName == name;
        };

        auto friendshipOnly = []() {
            return std::array<std::optional<std::string>, BattleDemoState::kSkillSlotCount>{{
                std::optional<std::string>{"Friendship"},
                std::nullopt,
                std::nullopt,
                std::nullopt
            }};
        };

        auto dragonSkills = [](std::string element) {
            std::array<std::optional<std::string>, BattleDemoState::kSkillSlotCount> skills;
            skills.fill(std::nullopt);
            skills[0] = std::optional<std::string>{"Friendship"};
            skills[1] = std::optional<std::string>{ element + " Slash" };
            return skills;
        };

        struct DragonDefinition {
            const char* name;
            const sf::Texture* sprite;
            const sf::Texture* back;
            const char* element;
        };

        std::array<DragonDefinition, 4> dragonDefinitions = {{
            { "Rowsted", &game.resources.spriteRowstedSheacane, &game.resources.spriteRowstedSheacaneBack, "Fire" },
            { "Flawtin", &game.resources.spriteFlawtinSeamen, &game.resources.spriteFlawtinSeamenBack, "Water" },
            { "Gustavo", &game.resources.spriteGustavoWindimaess, &game.resources.spriteGustavoWindimaessBack, "Air" },
            { "Grounded", &game.resources.spriteGroundedClaymore, &game.resources.spriteGroundedClaymoreBack, "Earth" }
        }};

        auto pushDragon = [&](const DragonDefinition& definition) {
            BattleDemoState::Combatant entry = makeCombatant(definition.name);
            entry.skills = dragonSkills(definition.element);
            CreatureMenuEntry card{
                entry,
                definition.sprite,
                definition.back,
                Game::DragonbornGender::Male,
                false,
                false,
                false,
                isDefeated(definition.name)
            };
            card.disabled = card.defeated || isBlocked(definition.name) || (battle.player.name == definition.name);
            entries.push_back(card);
        };

        auto addDragonsForSlots = [&](size_t maxSlots) {
            for (const auto& definition : dragonDefinitions) {
                if (entries.size() >= maxSlots)
                    break;
                if (isDefeated(definition.name))
                    continue;
                pushDragon(definition);
            }
        };

        auto addAllDragons = [&]() {
            for (const auto& definition : dragonDefinitions) {
                if (isDefeated(definition.name))
                    continue;
                pushDragon(definition);
            }
        };

        std::string heroName = heroDisplayName(game);
        bool heroDefeated = isDefeated(heroName);
        if (type == BattleDemoState::CreatureMenuType::Glandumon) {
            constexpr size_t kMenuSlots = 4;
            if (!heroDefeated) {
                BattleDemoState::Combatant playerEntry = makeCombatant(heroName);
                playerEntry.level = battle.dragonbornLevel;
                for (int i = 0; i < static_cast<int>(BattleDemoState::kSkillSlotCount); ++i) {
                    playerEntry.skills[i] = std::optional<std::string>{ kPlayerSkillNames[i] };
                }
                const sf::Texture* playerSprite = (game.playerGender == Game::DragonbornGender::Female)
                    ? &game.resources.spriteDragonbornFemale
                    : &game.resources.spriteDragonbornMale;
                const sf::Texture* playerBackSprite = (game.playerGender == Game::DragonbornGender::Female)
                    ? &game.resources.spriteDragonbornFemaleBack
                    : &game.resources.spriteDragonbornMaleBack;
                CreatureMenuEntry heroEntry{
                    playerEntry,
                    playerSprite,
                    playerBackSprite,
                    game.playerGender,
                    false,
                    true,
                    false,
                    heroDefeated
                };
                heroEntry.disabled = heroEntry.defeated || isBlocked(heroName) || (battle.player.name == heroName);
                entries.push_back(heroEntry);
            }
            auto pushFriend = [&](std::string name, const sf::Texture* sprite, const sf::Texture* back, Game::DragonbornGender gender) {
                if (entries.size() >= kMenuSlots)
                    return;
                if (isDefeated(name))
                    return;
                BattleDemoState::Combatant entry = makeCombatant(name);
                entry.skills = friendshipOnly();
                CreatureMenuEntry card{
                    entry,
                    sprite,
                    back,
                    gender,
                    false,
                    false,
                    false,
                    false
                };
                card.disabled = card.defeated || isBlocked(name) || (battle.player.name == name);
                entries.push_back(card);
            };
            pushFriend("Wanda Rinn", &game.resources.spriteWandaRinn, &game.resources.spriteWandaRinnBack, Game::DragonbornGender::Female);
            pushFriend("Will Figsid", &game.resources.spriteWillFigsid, &game.resources.spriteWillFigsidBack, Game::DragonbornGender::Male);
            pushFriend("Noah Lott", &game.resources.spriteNoahLott, &game.resources.spriteNoahLottBack, Game::DragonbornGender::Male);
            addDragonsForSlots(kMenuSlots);
        } else if (type == BattleDemoState::CreatureMenuType::Dragons) {
            addAllDragons();
        }

        return entries;
    }

    std::string formatHpValue(float current, float maximum) {
        int cur = static_cast<int>(std::lround(std::max(0.f, current)));
        int maxv = static_cast<int>(std::lround(std::max(0.f, maximum)));
        return std::to_string(cur) + "/" + std::to_string(maxv);
    }

    bool skillSlotAvailable(const BattleDemoState::Combatant& combatant, int index) {
        if (index < 0 || index >= static_cast<int>(BattleDemoState::kSkillSlotCount))
            return false;
        return combatant.skills[index].has_value();
    }

    int firstAvailableSkillIndex(const BattleDemoState::Combatant& combatant) {
        for (int i = 0; i < static_cast<int>(BattleDemoState::kSkillSlotCount); ++i) {
            if (skillSlotAvailable(combatant, i))
                return i;
        }
        return -1;
    }

    const sf::Texture* genderIconFor(const Game& game, Game::DragonbornGender gender) {
        return (gender == Game::DragonbornGender::Female)
            ? &game.resources.battleIconFemale
            : &game.resources.battleIconMale;
    }

    constexpr float kGenderIconScale = 0.015f;
    constexpr float kGenderIconSpacing = 5.f;

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

    void drawIntroBlink(Game& game, sf::RenderTarget& target, float width, float height, const BattleDemoState& battle) {
        sf::RectangleShape blackout({ width, height });
        blackout.setPosition({ 0.f, 0.f });
        blackout.setFillColor(sf::Color::Black);
        target.draw(blackout);

        if (!battle.introBlinkVisible)
            return;

        sf::Sprite badge{ game.resources.battleBeginsGlandular };
        auto badgeBounds = badge.getLocalBounds();
        float badgeWidth = badgeBounds.size.x;
        float badgeHeight = badgeBounds.size.y;
        if (badgeWidth > 0.f && badgeHeight > 0.f) {
            float targetWidth = width * 0.75f;
            float targetHeight = height * 0.35f;
            float scaleX = targetWidth / badgeWidth;
            float scaleY = targetHeight / badgeHeight;
            float scale = std::min(scaleX, scaleY);
            float maxScaleX = width / badgeWidth;
            float maxScaleY = height / badgeHeight;
            float maxScale = std::min(maxScaleX, maxScaleY);
            scale = std::min(scale * 2.f, maxScale);
            badge.setScale(sf::Vector2f{ scale, scale });
            badge.setOrigin({
                badgeBounds.position.x + (badgeBounds.size.x * 0.5f),
                badgeBounds.position.y + (badgeBounds.size.y * 0.5f)
            });
        }
        badge.setPosition({ width * 0.5f, height * 0.5f });
        target.draw(badge);
    }

    void drawShadeOverlay(sf::RenderTarget& target, float width, float height, float progress) {
        sf::VertexArray shade(sf::PrimitiveType::Triangles, 6);
        auto alphaFromOffset = [&](float offset) {
            float ratio = 1.f - progress - offset;
            ratio = std::clamp(ratio, 0.f, 1.f);
            return static_cast<std::uint8_t>(ratio * 255.f);
        };
        shade[0] = { { 0.f, 0.f }, sf::Color(30, 30, 30, alphaFromOffset(0.0f)) };
        shade[1] = { { width, 0.f }, sf::Color(30, 30, 30, alphaFromOffset(0.25f)) };
        shade[2] = { { width, height }, sf::Color(30, 30, 30, alphaFromOffset(0.6f)) };
        shade[3] = { { width, height }, sf::Color(30, 30, 30, alphaFromOffset(0.6f)) };
        shade[4] = { { 0.f, height }, sf::Color(30, 30, 30, alphaFromOffset(0.4f)) };
        shade[5] = { { 0.f, 0.f }, sf::Color(30, 30, 30, alphaFromOffset(0.0f)) };
        target.draw(shade);
    }

    void startBattleMusicIfNeeded(Game& game) {
        auto& battle = game.battleDemo;
        if (battle.battleMusicPlaying)
            return;
        if (!battle.battleMusic)
            battle.battleMusic = std::make_unique<sf::Music>();
        sf::Music& music = *battle.battleMusic;
        if (!music.openFromFile(kBattleMusicPath)) {
            battle.battleMusic.reset();
            return;
        }
        float startSec = kBattleMusicLoopStartSample / kBattleMusicSampleRate;
        float loopDuration = (kBattleMusicLoopEndSample - kBattleMusicLoopStartSample) / kBattleMusicSampleRate;
        sf::Music::TimeSpan loopPoints{ sf::seconds(startSec), sf::seconds(loopDuration) };
        music.setLoopPoints(loopPoints);
        music.setLooping(true);
        music.play();
        battle.battleMusicPlaying = true;
    }

    void stopBattleMusic(Game& game) {
        auto& battle = game.battleDemo;
        if (!battle.battleMusicPlaying)
            return;
        if (battle.battleMusic)
            battle.battleMusic->stop();
        battle.battleMusicPlaying = false;
        battle.battleMusic.reset();
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

    void openCreatureMenu(BattleDemoState& battle, BattleDemoState::CreatureMenuType type, bool allowCancel) {
        battle.creatureMenuVisible = true;
        battle.creatureMenuType = type;
        battle.creatureMenuSelection = 0;
        battle.creatureMenuEntryBounds.clear();
        battle.creatureMenuCancelBounds = {};
        battle.creatureMenuAllowCancel = allowCancel;
        battle.actionMenuVisible = false;
    }

    bool handleSwapPromptEvent(Game& game, const sf::Event& event) {
        auto& battle = game.battleDemo;
        if (!battle.swapPrompt.active)
            return false;
        sf::Vector2u viewSize = game.window.getSize();
        sf::Vector2f viewportSize{
            static_cast<float>(viewSize.x),
            static_cast<float>(viewSize.y)
        };
        SwapPromptLayout layout = computeSwapPromptLayout(viewportSize);

        auto confirmSelection = [&](bool confirmed) -> bool {
            if (confirmed) {
                beginPlayerSwap(game);
            } else {
                battle.swapPrompt.active = false;
            }
            return true;
        };

        if (auto key = event.getIf<sf::Event::KeyReleased>()) {
            switch (key->scancode) {
                case sf::Keyboard::Scan::Left:
                case sf::Keyboard::Scan::Up:
                    battle.swapPrompt.selectedButton = 0;
                    return true;
                case sf::Keyboard::Scan::Right:
                case sf::Keyboard::Scan::Down:
                    battle.swapPrompt.selectedButton = 1;
                    return true;
                case sf::Keyboard::Scan::Enter:
                case sf::Keyboard::Scan::Space:
                case sf::Keyboard::Scan::Z:
                    return confirmSelection(battle.swapPrompt.selectedButton == 0);
                case sf::Keyboard::Scan::Escape:
                case sf::Keyboard::Scan::Backspace:
                    return confirmSelection(false);
                default:
                    return true;
            }
        }

        if (auto mouse = event.getIf<sf::Event::MouseMoved>()) {
            auto position = game.window.mapPixelToCoords(mouse->position);
            if (layout.yesBounds.contains(position)) {
                battle.swapPrompt.selectedButton = 0;
            } else if (layout.noBounds.contains(position)) {
                battle.swapPrompt.selectedButton = 1;
            }
            return true;
        }

        if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (button->button == sf::Mouse::Button::Left) {
                auto position = game.window.mapPixelToCoords(button->position);
                if (layout.yesBounds.contains(position)) {
                    return confirmSelection(true);
                }
                if (layout.noBounds.contains(position)) {
                    return confirmSelection(false);
                }
                return true;
            }
        }

        return true;
    }

    bool handleCreatureMenuEvent(Game& game, const sf::Event& event) {
        auto& battle = game.battleDemo;
        if (!battle.creatureMenuVisible)
            return false;

        auto entries = buildCreatureMenuEntries(game, battle.creatureMenuType);
        int entryCount = static_cast<int>(entries.size());
        if (entryCount <= 0) {
            battle.creatureMenuVisible = false;
            battle.creatureMenuType = BattleDemoState::CreatureMenuType::None;
            battle.actionMenuVisible = true;
            return false;
        }
        normalizeCreatureSelection(battle, entries);
        int totalOptions = entryCount + (battle.creatureMenuAllowCancel ? 1 : 0);
        auto wrapIndex = [&](int value) {
            if (value < 0)
                value += totalOptions;
            else if (value >= totalOptions)
                value -= totalOptions;
            return value;
        };
        auto moveSelection = [&](int delta) {
            if (entryCount <= 0)
                return;
            int candidate = battle.creatureMenuSelection;
            int tries = entryCount;
            do {
                candidate = wrapIndex(candidate + delta);
                if (candidate >= entryCount) {
                    if (battle.creatureMenuAllowCancel) {
                        battle.creatureMenuSelection = candidate;
                        return;
                    }
                }
                if (!entries[candidate].disabled) {
                    battle.creatureMenuSelection = candidate;
                    return;
                }
            } while (--tries > 0);
        };
        auto finalizeSelection = [&](int index, bool cancelled) {
            if (!battle.creatureMenuAllowCancel && (cancelled || index >= entryCount))
                return;
            if (!cancelled && index >= 0 && index < entryCount) {
                if (entries[index].disabled)
                    return;
                if (battle.player.name == entries[index].combatant.name)
                    return;
                startSwapPrompt(
                    battle,
                    entries[index].combatant,
                    entries[index].backSprite,
                    entries[index].isDragonborn
                );
                return;
            }
                if (battle.creatureMenuType == BattleDemoState::CreatureMenuType::Dragons) {
                    pushLog(game, "Exited Drag menu.");
                }
            battle.creatureMenuVisible = false;
            battle.creatureMenuType = BattleDemoState::CreatureMenuType::None;
            battle.creatureMenuSelection = 0;
            battle.creatureMenuEntryBounds.clear();
            battle.creatureMenuCancelBounds = {};
            battle.actionMenuVisible = true;
        };

        if (auto key = event.getIf<sf::Event::KeyReleased>()) {
            switch (key->scancode) {
                case sf::Keyboard::Scan::Up:
                    moveSelection(-1);
                    return true;
                case sf::Keyboard::Scan::Down:
                    moveSelection(1);
                    return true;
                case sf::Keyboard::Scan::Enter:
                case sf::Keyboard::Scan::Space:
                case sf::Keyboard::Scan::Z:
                    if (battle.creatureMenuAllowCancel && battle.creatureMenuSelection >= entryCount) {
                        finalizeSelection(battle.creatureMenuSelection, true);
                    } else {
                        finalizeSelection(battle.creatureMenuSelection, false);
                    }
                    return true;
                case sf::Keyboard::Scan::Escape:
                case sf::Keyboard::Scan::Backspace:
                    if (battle.creatureMenuAllowCancel)
                        finalizeSelection(entryCount, true);
                    return true;
                default:
                    return false;
            }
        }

        auto handleMouseMove = [&](const sf::Vector2f& mousePos) -> bool {
            for (int i = 0; i < static_cast<int>(battle.creatureMenuEntryBounds.size()); ++i) {
                if (battle.creatureMenuEntryBounds[i].contains(mousePos) && !entries[i].disabled) {
                    battle.creatureMenuSelection = i;
                    return true;
                }
            }
            if (battle.creatureMenuAllowCancel && battle.creatureMenuCancelBounds.contains(mousePos)) {
                battle.creatureMenuSelection = entryCount;
                return true;
            }
            return false;
        };

        if (auto mouse = event.getIf<sf::Event::MouseMoved>()) {
            auto mousePos = game.window.mapPixelToCoords(mouse->position);
            if (handleMouseMove(mousePos))
                return true;
        }

        if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (button->button == sf::Mouse::Button::Left) {
                auto mousePos = game.window.mapPixelToCoords(button->position);
                    if (handleMouseMove(mousePos)) {
                        if (battle.creatureMenuAllowCancel && battle.creatureMenuSelection >= entryCount) {
                            finalizeSelection(entryCount, true);
                        } else if (!entries[battle.creatureMenuSelection].disabled) {
                            finalizeSelection(battle.creatureMenuSelection, false);
                        }
                        return true;
                    }
            }
        }

        return false;
    }

    void drawCreatureCard(Game& game, sf::RenderTarget& target,
                          const CreatureMenuEntry& entry, const sf::FloatRect& bounds,
                          bool highlight, bool large) {
        if (bounds.size.x <= 0.f || bounds.size.y <= 0.f)
            return;

        bool cardDisabled = entry.disabled;
        bool useHighlight = highlight && !cardDisabled;
        sf::Color baseFill = cardDisabled
            ? sf::Color(18, 20, 26)
            : (useHighlight ? sf::Color(80, 80, 90) : sf::Color(20, 22, 28));
        sf::Color outlineColor = cardDisabled
            ? sf::Color(60, 60, 70)
            : (useHighlight ? sf::Color(220, 60, 74) : sf::Color(110, 20, 32));
        RoundedRectangleShape frame({ bounds.size.x, bounds.size.y }, bounds.size.y * 0.15f, 20);
        frame.setPosition(bounds.position);
        frame.setFillColor(baseFill);
        frame.setOutlineThickness(2.f);
        frame.setOutlineColor(outlineColor);
        target.draw(frame);
        sf::Color textColor = cardDisabled ? sf::Color(150, 150, 170) : sf::Color::White;

        auto gatherGenderIcons = [&](bool showBoth, Game::DragonbornGender baseGender) {
            std::vector<const sf::Texture*> icons;
            if (showBoth) {
                icons.push_back(genderIconFor(game, Game::DragonbornGender::Female));
                icons.push_back(genderIconFor(game, Game::DragonbornGender::Male));
            } else {
                if (const sf::Texture* icon = genderIconFor(game, baseGender))
                    icons.push_back(icon);
            }
            return icons;
        };

        auto drawGenderRow = [&](float startX, float centerY, const std::vector<const sf::Texture*>& icons) {
            float cursor = startX;
            for (const sf::Texture* iconTexture : icons) {
                if (!iconTexture)
                    continue;
                sf::Sprite iconSprite{ *iconTexture };
                auto iconBounds = iconSprite.getLocalBounds();
                iconSprite.setOrigin({
                    iconBounds.position.x,
                    iconBounds.position.y + (iconBounds.size.y * 0.5f)
                });
                iconSprite.setScale(sf::Vector2f{ kGenderIconScale, kGenderIconScale });
                iconSprite.setPosition({ cursor, centerY });
                target.draw(iconSprite);
                cursor += (iconBounds.size.x * kGenderIconScale) + kGenderIconSpacing;
            }
            return cursor;
        };

        float padding = large ? 28.f : 16.f;
        const sf::Texture* spriteTexture = entry.sprite ? entry.sprite : &game.resources.spriteDragonbornMale;
        sf::Sprite sprite{ *spriteTexture };
        auto spriteBounds = sprite.getLocalBounds();

        float spriteTarget = 0.f;
        float hpRowHeight = 0.f;
        float nameAreaHeight = 0.f;
        float spriteAreaHeight = 0.f;
        if (large) {
            float innerHeight = bounds.size.y - (padding * 2.f);
            hpRowHeight = std::clamp(innerHeight * 0.24f, 64.f, 110.f);
            nameAreaHeight = std::clamp(innerHeight * 0.18f, 60.f, 140.f);
            spriteAreaHeight = std::max(0.f, innerHeight - hpRowHeight - nameAreaHeight);
            float availableWidth = std::max(0.f, bounds.size.x - (padding * 2.f));
            float spriteSide = std::max(0.f, std::min(spriteAreaHeight, availableWidth));
            spriteTarget = std::max(spriteSide, 1.f);
        } else {
            float availableSpriteArea = bounds.size.y - (padding * 2.f);
            spriteTarget = std::max(40.f, std::max(0.f, availableSpriteArea));
        }

        float spriteScale = 1.f;
        if (spriteBounds.size.x > 0.f && spriteBounds.size.y > 0.f)
            spriteScale = spriteTarget / std::max(spriteBounds.size.x, spriteBounds.size.y);
        sprite.setScale(sf::Vector2f{ spriteScale, spriteScale });
        float spriteWidth = spriteBounds.size.x * spriteScale;
        float spriteHeight = spriteBounds.size.y * spriteScale;

        if (large) {
            sf::FloatRect hpArea(
                sf::Vector2f{ bounds.position.x + padding, bounds.position.y + padding },
                sf::Vector2f{ bounds.size.x - (padding * 2.f), hpRowHeight }
            );
            auto drawLargeHpRow = [&](const sf::FloatRect& area) {
                if (area.size.x <= 0.f || area.size.y <= 0.f)
                    return;

                sf::Text hpLabel{ game.resources.battleFont, "HP", 16 };
                hpLabel.setFillColor(cardDisabled ? textColor : ColorHelper::Palette::SoftRed);
                hpLabel.setLetterSpacing(hpLabel.getLetterSpacing() + 0.5f);
                auto hpLabelBounds = hpLabel.getLocalBounds();
                float labelCenterY = area.position.y + (area.size.y * 0.5f);
                float labelX = area.position.x + 18.f;
                hpLabel.setOrigin({
                    hpLabelBounds.position.x,
                    hpLabelBounds.position.y + (hpLabelBounds.size.y * 0.5f)
                });
                hpLabel.setPosition({ labelX, labelCenterY });
                target.draw(hpLabel);

                sf::Text hpValue{ game.resources.battleFont, formatHpValue(entry.combatant.hp, entry.combatant.maxHp), 24 };
                hpValue.setFillColor(textColor);
                hpValue.setLetterSpacing(hpValue.getLetterSpacing() + 0.5f);
                auto hpValueBounds = hpValue.getLocalBounds();
                float valueRight = area.position.x + area.size.x - 18.f;
                float valueX = valueRight - hpValueBounds.size.x;
                hpValue.setOrigin({
                    hpValueBounds.position.x + hpValueBounds.size.x,
                    hpValueBounds.position.y + (hpValueBounds.size.y * 0.5f)
                });
                hpValue.setPosition({ valueRight, labelCenterY });
                target.draw(hpValue);

                float barHeight = std::min(area.size.y * 0.4f, 18.f);
                barHeight = std::max(barHeight, 8.f);
                float barLeft = labelX + hpLabelBounds.size.x + 24.f;
                float barRight = valueX - 18.f;
                float barWidth = std::max(0.f, barRight - barLeft);
                if (barWidth <= 0.f)
                    return;

                float hpRatio = (entry.combatant.maxHp > 0.f)
                    ? std::clamp(entry.combatant.hp / entry.combatant.maxHp, 0.f, 1.f)
                    : 0.f;
                if (hpRatio > 0.f) {
                    float hpFillWidth = std::max(barWidth * hpRatio, barHeight);
                    hpFillWidth = std::min(barWidth, hpFillWidth);
                    RoundedRectangleShape hpFill({ hpFillWidth, barHeight }, barHeight * 0.5f, 20);
                    hpFill.setPosition({ barLeft, labelCenterY - (barHeight * 0.5f) });
                    hpFill.setFillColor(ColorHelper::Palette::Health);
                    target.draw(hpFill);
                }

                RoundedRectangleShape hpBorder({ barWidth, barHeight }, barHeight * 0.5f, 20);
                hpBorder.setPosition({ barLeft, labelCenterY - (barHeight * 0.5f) });
                hpBorder.setFillColor(sf::Color::Transparent);
                hpBorder.setOutlineThickness(1.f);
                hpBorder.setOutlineColor(sf::Color::Black);
                target.draw(hpBorder);
            };
            drawLargeHpRow(hpArea);

            float spriteX = bounds.position.x + (bounds.size.x - spriteWidth) * 0.5f;
            float spriteYBase = bounds.position.y + padding + hpRowHeight;
            float spriteSpace = std::max(0.f, spriteAreaHeight - spriteHeight);
            float spriteY = spriteYBase + (spriteSpace * 0.5f);
            sprite.setPosition({ spriteX, spriteY });
            sprite.setColor(cardDisabled ? sf::Color(160, 160, 170) : sf::Color::White);
            target.draw(sprite);

            float nameFontSize = 40.f;
            sf::Text nameText{ game.resources.battleFont, entry.combatant.name, static_cast<unsigned int>(nameFontSize) };
            nameText.setFillColor(textColor);
            nameText.setLetterSpacing(nameText.getLetterSpacing() + 0.5f);
            auto nameBounds = nameText.getLocalBounds();
            float nameTopLimit = bounds.position.y + bounds.size.y - padding - nameAreaHeight;
            float nameY = std::max(spriteY + spriteHeight + 12.f, nameTopLimit + 6.f);
            float centeredX = bounds.position.x + (bounds.size.x * 0.5f);
            nameText.setOrigin({
                nameBounds.position.x + (nameBounds.size.x * 0.5f),
                nameBounds.position.y
            });
            nameText.setPosition({ centeredX, nameY });
            target.draw(nameText);
            auto largeIcons = gatherGenderIcons(entry.showBothGenders, entry.gender);
            if (!largeIcons.empty()) {
                float largeNameCenterY = nameY + (nameFontSize * 0.5f);
                float largeStartX = centeredX + (nameBounds.size.x * 0.5f) + 12.f;
                drawGenderRow(largeStartX, largeNameCenterY, largeIcons);
            }

            sf::Text levelText{ game.resources.battleFont, "Lv." + std::to_string(entry.combatant.level), 32 };
            levelText.setFillColor(textColor);
            levelText.setLetterSpacing(levelText.getLetterSpacing() + 0.5f);
            auto levelBounds = levelText.getLocalBounds();
            levelText.setOrigin({
                levelBounds.position.x + (levelBounds.size.x * 0.5f),
                levelBounds.position.y
            });
            float levelY = nameY + nameFontSize + 6.f;
            levelText.setPosition({ centeredX, levelY });
            target.draw(levelText);
        } else {
            float spriteX = bounds.position.x + padding;
            float spriteYBase = bounds.position.y + padding;
            float availableSpriteArea = bounds.size.y - (padding * 2.f);
            float spriteSpace = std::max(0.f, availableSpriteArea - spriteHeight);
            float spriteY = spriteYBase + (spriteSpace * 0.5f);
            sprite.setPosition({ spriteX, spriteY });
            sprite.setColor(cardDisabled ? sf::Color(160, 160, 170) : sf::Color::White);
            target.draw(sprite);

            float textX = spriteX + spriteWidth + 14.f;
            sf::Text nameText{ game.resources.battleFont, entry.combatant.name, 32 };
            nameText.setFillColor(textColor);
            nameText.setLetterSpacing(nameText.getLetterSpacing() + 0.5f);
            float nameY = bounds.position.y + padding - 4.f;
            nameText.setPosition({ textX, nameY });
            target.draw(nameText);

            float nameFontSizeSmall = static_cast<float>(nameText.getCharacterSize());
            float smallNameCenterY = nameY + (nameFontSizeSmall * 0.5f);
            auto smallIcons = gatherGenderIcons(entry.showBothGenders, entry.gender);
            if (!smallIcons.empty()) {
                float smallNameRight = textX + nameText.getGlobalBounds().size.x;
                drawGenderRow(smallNameRight + 12.f, smallNameCenterY, smallIcons);
            }

            sf::Text levelText{ game.resources.battleFont, "Lv." + std::to_string(entry.combatant.level), 24 };
            levelText.setFillColor(textColor);
            levelText.setLetterSpacing(levelText.getLetterSpacing() + 0.5f);
            float levelY = nameY + static_cast<float>(nameText.getCharacterSize()) + 8.f;
            levelText.setPosition({ textX, levelY });
            target.draw(levelText);

            float hpBarWidth = std::clamp(bounds.size.x * 0.35f, 120.f, bounds.size.x * 0.45f);
            float hpBarHeight = std::clamp(bounds.size.y * 0.07f, 10.f, 18.f);
            float hpBarX = bounds.position.x + bounds.size.x - padding - hpBarWidth;
            float hpBarY = smallNameCenterY - (hpBarHeight * 0.5f);
            RoundedRectangleShape hpBackground({ hpBarWidth, hpBarHeight }, hpBarHeight * 0.5f, 20);
            hpBackground.setPosition({ hpBarX, hpBarY });
            hpBackground.setFillColor(sf::Color::Black);
            hpBackground.setOutlineThickness(1.f);
            hpBackground.setOutlineColor(sf::Color::Black);
            target.draw(hpBackground);

            float hpRatio = (entry.combatant.maxHp > 0.f)
                ? std::clamp(entry.combatant.hp / entry.combatant.maxHp, 0.f, 1.f)
                : 0.f;
            if (hpRatio > 0.f) {
                RoundedRectangleShape hpFill({ hpBarWidth * hpRatio, hpBarHeight }, hpBarHeight * 0.5f, 20);
                hpFill.setPosition({ hpBarX, hpBarY });
                hpFill.setFillColor(ColorHelper::Palette::Health);
                target.draw(hpFill);
            }

            sf::Text hpLabel{ game.resources.battleFont, "HP", 16 };
            hpLabel.setFillColor(cardDisabled ? textColor : ColorHelper::Palette::SoftRed);
            auto hpLabelBounds = hpLabel.getLocalBounds();
            hpLabel.setOrigin({ hpLabelBounds.position.x, hpLabelBounds.position.y + (hpLabelBounds.size.y * 0.5f) });
            float hpLabelX = hpBarX - 24.f - hpLabelBounds.size.x;
            hpLabel.setPosition({ hpLabelX, smallNameCenterY });
            target.draw(hpLabel);

            sf::Text hpValue{ game.resources.battleFont, formatHpValue(entry.combatant.hp, entry.combatant.maxHp), 24 };
            hpValue.setFillColor(sf::Color::White);
            auto hpValueBounds = hpValue.getLocalBounds();
            float hpValueRight = hpBarX + hpBarWidth;
            hpValue.setOrigin({
                hpValueBounds.position.x + hpValueBounds.size.x,
                hpValueBounds.position.y
            });
            hpValue.setPosition({ hpValueRight, levelY });
            target.draw(hpValue);
        }
    }

    void drawCreatureMenuOverlay(Game& game, sf::RenderTarget& target,
                                 const sf::Vector2f& backgroundPosition, const sf::FloatRect& backgroundBounds) {
        auto& battle = game.battleDemo;
        auto entries = buildCreatureMenuEntries(game, battle.creatureMenuType);
        if (entries.empty())
            return;

        sf::VertexArray gradient(sf::PrimitiveType::Triangles, 6);
        gradient[0].position = backgroundPosition;
        gradient[0].color = sf::Color(12, 14, 20);
        gradient[1].position = { backgroundPosition.x + backgroundBounds.size.x, backgroundPosition.y };
        gradient[1].color = sf::Color(196, 52, 58);
        gradient[2].position = { backgroundPosition.x + backgroundBounds.size.x, backgroundPosition.y + backgroundBounds.size.y };
        gradient[2].color = sf::Color(240, 241, 245);
        gradient[3].position = backgroundPosition;
        gradient[3].color = sf::Color(12, 14, 20);
        gradient[4].position = { backgroundPosition.x + backgroundBounds.size.x, backgroundPosition.y + backgroundBounds.size.y };
        gradient[4].color = sf::Color(240, 241, 245);
        gradient[5].position = { backgroundPosition.x, backgroundPosition.y + backgroundBounds.size.y };
        gradient[5].color = sf::Color(28, 28, 34);
        target.draw(gradient);

        const float padding = 48.f;
        const float footerHeight = 90.f;
        const float footerSpacing = 16.f;
        float menuHeight = backgroundBounds.size.y - (padding * 2.f) - footerHeight - footerSpacing;
        sf::FloatRect menuArea({
            backgroundPosition.x + padding,
            backgroundPosition.y + padding
        }, {
            backgroundBounds.size.x - (padding * 2.f),
            menuHeight
        });
        if (menuArea.size.y <= 0.f || menuArea.size.x <= 0.f)
            return;

        const float columnGap = 32.f;
        float leftWidth = menuArea.size.x * 0.45f;
        float rightWidth = menuArea.size.x - leftWidth - columnGap;
        sf::FloatRect leftBounds({ menuArea.position.x, menuArea.position.y }, { leftWidth, menuArea.size.y });
        sf::FloatRect rightBounds({ menuArea.position.x + leftWidth + columnGap, menuArea.position.y }, { rightWidth, menuArea.size.y });

        int activeIndex = normalizeCreatureSelection(battle, entries);
        if (activeIndex < 0)
            return;
        drawCreatureCard(game, target, entries[activeIndex], leftBounds, true, true);

        battle.creatureMenuEntryBounds.assign(entries.size(), {});
        constexpr float kMenuSlotCount = 4.f;
        float entrySpacing = 12.f;
        float totalSpacing = entrySpacing * (kMenuSlotCount - 1.f);
        float availableHeightForSlots = std::max(0.f, rightBounds.size.y - totalSpacing);
        float entryHeight = std::max(64.f, availableHeightForSlots / kMenuSlotCount);
        float currentY = rightBounds.position.y;
        for (std::size_t index = 0; index < entries.size(); ++index) {
            sf::FloatRect smallBounds(
                { rightBounds.position.x, currentY },
                { rightBounds.size.x, entryHeight }
            );
            bool highlight = (static_cast<int>(index) == battle.creatureMenuSelection);
            drawCreatureCard(game, target, entries[index], smallBounds, highlight, false);
            battle.creatureMenuEntryBounds[index] = smallBounds;
            currentY += entryHeight + entrySpacing;
        }

        float maxFooterWidth = backgroundBounds.size.x - (padding * 2.f);
        float footerWidth = std::max(0.f, maxFooterWidth * 0.5f);
        float footerX = backgroundPosition.x + (backgroundBounds.size.x - footerWidth) * 0.5f;
        float footerY = backgroundPosition.y + padding + menuArea.size.y + footerSpacing;
        sf::FloatRect footerBounds(
            { footerX, footerY },
            { footerWidth, footerHeight }
        );
        if (footerBounds.size.x <= 0.f)
            return;
        float footerCenterY = footerBounds.position.y + (footerBounds.size.y * 0.5f);
        RoundedRectangleShape footerShape({ footerBounds.size.x, footerBounds.size.y }, 24.f, 20);
        footerShape.setPosition(footerBounds.position);
        footerShape.setFillColor(sf::Color(18, 20, 28));
        footerShape.setOutlineColor(sf::Color(120, 20, 32));
        footerShape.setOutlineThickness(2.f);
        target.draw(footerShape);

        std::string promptText = "CHOOSE YOUR GLANDUMON";
        sf::Text prompt{ game.resources.battleFont, promptText, 32 };
        prompt.setFillColor(sf::Color::White);
        prompt.setLetterSpacing(prompt.getLetterSpacing() + 0.5f);
        auto promptBounds = prompt.getLocalBounds();
        prompt.setOrigin({0.f, promptBounds.position.y + (promptBounds.size.y * 0.5f)});
        prompt.setPosition({
            footerBounds.position.x + 24.f,
            footerCenterY
        });
        target.draw(prompt);

        if (battle.creatureMenuAllowCancel) {
            float cancelWidth = 140.f;
            float cancelHeight = 56.f;
            float cancelY = footerCenterY - (cancelHeight * 0.5f);
            sf::FloatRect cancelBounds(
                { footerBounds.position.x + footerBounds.size.x - cancelWidth - 24.f,
                  cancelY },
                { cancelWidth, cancelHeight }
            );
            battle.creatureMenuCancelBounds = cancelBounds;
            bool cancelHighlight = (battle.creatureMenuSelection >= static_cast<int>(entries.size()));
            sf::Color cancelFill = cancelHighlight ? sf::Color(99, 93, 201) : sf::Color(150, 85, 210);
            sf::Color cancelOutline = sf::Color(62, 25, 128);
            RoundedRectangleShape cancelShape({ cancelBounds.size.x, cancelBounds.size.y }, cancelBounds.size.y * 0.35f, 16);
            cancelShape.setPosition(cancelBounds.position);
            cancelShape.setFillColor(cancelFill);
            cancelShape.setOutlineColor(cancelOutline);
            cancelShape.setOutlineThickness(2.f);
            target.draw(cancelShape);

            sf::Text cancelText{ game.resources.battleFont, "CANCEL", 26 };
            cancelText.setFillColor(sf::Color::White);
            auto cancelTextBounds = cancelText.getLocalBounds();
            cancelText.setOrigin({
                cancelTextBounds.position.x + (cancelTextBounds.size.x * 0.5f),
                cancelTextBounds.position.y + (cancelTextBounds.size.y * 0.5f)
            });
            cancelText.setPosition({
                cancelBounds.position.x + (cancelBounds.size.x * 0.5f),
                cancelBounds.position.y + (cancelBounds.size.y * 0.5f)
            });
            target.draw(cancelText);
        } else {
            battle.creatureMenuCancelBounds = {};
        }
    }

    bool queueEnemyAttack(Game& game) {
        auto& battle = game.battleDemo;

        std::vector<int> available;
        available.reserve(BattleDemoState::kSkillSlotCount);
        for (int index = 0; index < static_cast<int>(BattleDemoState::kSkillSlotCount); ++index) {
            if (skillSlotAvailable(battle.enemy, index))
                available.push_back(index);
        }

        if (available.empty())
            return false;

        int moveIndex = battle.enemyMoveIndex % static_cast<int>(available.size());
        int skillIndex = available[moveIndex];
        float damage = kEnemySkillDamage[skillIndex];
        if (battle.masterBatesDragonActive)
            damage *= kMasterBatesDragonMultiplier;
        battle.enemyMoveIndex = (moveIndex + 1) % static_cast<int>(available.size());
        const std::string& enemySkill = *battle.enemy.skills[skillIndex];
        float projectedHp = battle.player.hp - damage;
        bool heroActive = isDragonbornActive(battle);
        bool forcedRetreatNeeded = projectedHp <= 0.f;
        float newPlayerHp = std::max(0.f, projectedHp);
        if (heroActive && forcedRetreatNeeded)
            newPlayerHp = 1.f;
        BattleDemoState::SkillElement element = skillElementFromName(enemySkill);
        bool hasVisualEffect = (enemySkill == "Friendship")
            || isMasterBatesSkill(enemySkill)
            || element != BattleDemoState::SkillElement::None;
        if (forcedRetreatNeeded) {
            battle.forcedRetreat.awaitingSwap = true;
            battle.forcedRetreat.pendingName = battle.player.name;
        }
        battle.actionMenuVisible = false;
        battle.reopenMenuAfterPlayerPulse = !forcedRetreatNeeded;
        if (enemySkill == "Friendship") {
            sf::Vector2f startAnchor = (battle.cachedPlayerCenter + battle.cachedEnemyCenter) * 0.5f;
            startFriendshipEffect(
                game,
                battle,
                BattleDemoState::FriendshipEffect::Target::Player,
                startAnchor,
                battle.cachedPlayerCenter
            );
            battle.friendshipEffect.pendingDamage = true;
            battle.friendshipEffect.pendingHp = newPlayerHp;
        } else if (isMasterBatesSkill(enemySkill)) {
            startMasterBatesSkillEffect(
                game,
                battle,
                BattleDemoState::MasterBatesSkillEffect::Target::Player,
                enemySkill,
                newPlayerHp
            );
        } else if (element != BattleDemoState::SkillElement::None) {
            startSkillEffect(game, battle, element, BattleDemoState::SkillEffect::Target::Player);
            battle.skillEffect.pendingDamage = true;
            battle.skillEffect.pendingHp = newPlayerHp;
        } else {
            battle.player.hp = newPlayerHp;
            startHpPulse(battle.playerHpPulse, battle.playerDisplayedHp, newPlayerHp);
            cacheActiveCreatureStats(battle);
            markActiveCreatureDefeated(battle);
            maybeRecallDragonborn(game, battle);
        }
        std::string playerLabel = displayPlayerName(game, battle);
        pushLog(game, battle.enemy.name + " uses " + enemySkill + " on " + playerLabel + "!");
        return forcedRetreatNeeded;
    }

    void advanceForcedRetreatCooldown(BattleDemoState& battle) {
        auto& retreat = battle.forcedRetreat;
        if (!retreat.active)
            return;
        if (!allTrackedCreaturesDefeated(battle))
            return;
        if (!retreat.blockedName.empty()) {
            auto heroName = retreat.blockedName;
            float heroMax = fallbackTrackedMaxHp(heroName);
            auto maxIt = battle.creatureMaxHp.find(heroName);
            if (maxIt != battle.creatureMaxHp.end())
                heroMax = maxIt->second;
            battle.creatureHp[heroName] = heroMax;
        }
        retreat.active = false;
        retreat.blockedName.clear();
    }

    void beginPlayerAction(Game& game, int skillIndex) {
        auto& battle = game.battleDemo;
        if (battle.phase != BattleDemoState::Phase::PlayerChoice)
            return;

        if (!skillSlotAvailable(battle.player, skillIndex))
            return;

        battle.phase = BattleDemoState::Phase::PlayerAction;
        battle.actionTimer = 0.f;
        battle.actionMenuVisible = false;
        battle.fightMenuVisible = false;
        battle.fightCancelHighlight = false;

        std::string skillName = *battle.player.skills[skillIndex];
        float damage = dragonbornSkillDamageForLevel(battle.player.level, skillIndex);
        if (battle.currentDragonbornActive && battle.player.level >= 100) {
            damage *= kDragonbornDamageMultiplier;
        }
        std::string playerLabel = displayPlayerName(game, battle);
        pushLog(game, playerLabel + " uses " + skillName + " on " + battle.enemy.name + "!");

        float newEnemyHp = std::max(0.f, battle.enemy.hp - damage);
        if (skillName == "Friendship") {
            sf::Vector2f startAnchor = (battle.cachedPlayerCenter + battle.cachedEnemyCenter) * 0.5f;
            startFriendshipEffect(
                game,
                battle,
                BattleDemoState::FriendshipEffect::Target::Enemy,
                startAnchor,
                battle.cachedEnemyCenter
            );
            battle.friendshipEffect.pendingDamage = true;
            battle.friendshipEffect.pendingHp = newEnemyHp;
        } else {
            auto element = skillElementFromName(skillName);
            if (element != BattleDemoState::SkillElement::None) {
                startSkillEffect(game, battle, element, BattleDemoState::SkillEffect::Target::Enemy);
                battle.skillEffect.pendingDamage = true;
                battle.skillEffect.pendingHp = newEnemyHp;
            } else {
                battle.enemy.hp = newEnemyHp;
                startHpPulse(battle.enemyHpPulse, battle.enemyDisplayedHp, newEnemyHp);
                if (battle.enemy.hp <= 0.f) {
                    battle.phase = BattleDemoState::Phase::Victory;
                    battle.victoryTransitioned = false;
                    battle.completionClock.restart();
                }
            }
        }
    }
}

void update(Game& game, sf::Time dt) {
    if (game.state != GameState::BattleDemo)
        return;

    auto& battle = game.battleDemo;
    cacheActiveCreatureStats(battle);
    startBattleMusicIfNeeded(game);
    auto advanceIntroPhase = [&](sf::Time delta) -> bool {
        if (battle.introPhase == BattleDemoState::IntroPhase::Complete)
            return false;

        if (battle.introPhase == BattleDemoState::IntroPhase::Blinking) {
            float duration = battle.introBlinkVisible ? kIntroBlinkVisibleDuration : kIntroBlinkHiddenDuration;
            battle.introBlinkTimer += delta.asSeconds();
            if (battle.introBlinkTimer >= duration) {
                battle.introBlinkTimer -= duration;
                battle.introBlinkVisible = !battle.introBlinkVisible;
                if (battle.introBlinkVisible) {
                    ++battle.introBlinkCount;
                    if (battle.introBlinkCount >= kIntroBlinkCycles) {
                        battle.introPhase = BattleDemoState::IntroPhase::Shading;
                        battle.introShadeProgress = 0.f;
                        battle.introBlinkTimer = 0.f;
                        battle.introBlinkVisible = false;
                    }
                }
            }
        } else {
            battle.introShadeProgress = std::min(1.f, battle.introShadeProgress + delta.asSeconds() / kIntroShadeDuration);
            if (battle.introShadeProgress >= 1.f) {
                battle.introPhase = BattleDemoState::IntroPhase::Complete;
            }
        }

        return true;
    };
    if (advanceIntroPhase(dt))
        return;
    updateSkillEffect(game, battle, dt.asSeconds());
    updateFriendshipEffect(game, battle, dt.asSeconds());
    updateSwapAnimation(game, battle, dt.asSeconds());
    updateMasterBatesEvolution(game, battle, dt.asSeconds());
    updateMasterBatesSkillEffect(game, battle, dt.asSeconds());
    auto advanceHpPulseDisplay = [&](BattleDemoState::HpPulse& pulse, float& displayHp, float actualHp) {
        if (pulse.active) {
            float progress = std::clamp(
                pulse.clock.getElapsedTime().asSeconds() / kBattleHpPulseDuration,
                0.f,
                1.f
            );
            displayHp = pulse.startHp + (pulse.endHp - pulse.startHp) * progress;
            if (progress >= 1.f) {
                pulse.active = false;
                displayHp = pulse.endHp;
            }
        } else {
            displayHp = actualHp;
        }
    };
    advanceHpPulseDisplay(battle.playerHpPulse, battle.playerDisplayedHp, battle.player.hp);
    advanceHpPulseDisplay(battle.enemyHpPulse, battle.enemyDisplayedHp, battle.enemy.hp);
    maybeFinalizeForcedRetreat(game);
    advanceForcedRetreatCooldown(battle);
    if (battle.reopenMenuAfterPlayerPulse
        && !battle.playerHpPulse.active
        && battle.phase == BattleDemoState::Phase::PlayerChoice
        && !battle.skillEffect.active
        && !battle.friendshipEffect.active
        && !battle.masterBatesSkillEffect.active) {
        battle.actionMenuVisible = true;
        battle.reopenMenuAfterPlayerPulse = false;
    }
    if (battle.platformEntranceTimer < battle.platformEntranceDuration) {
        battle.platformEntranceTimer = std::min(battle.platformEntranceDuration, battle.platformEntranceTimer + dt.asSeconds());
    }
    if (battle.phase == BattleDemoState::Phase::PlayerAction) {
        battle.actionTimer += dt.asSeconds();
        if (battle.actionTimer >= battle.playerActionDelay
            && !battle.skillEffect.active
            && !battle.swapAnimation.active
            && !battle.masterBatesEvolution.active) {
            battle.phase = BattleDemoState::Phase::EnemyAction;
            battle.actionTimer = 0.f;
            battle.actionMenuVisible = false;
        }
    }
    else if (battle.phase == BattleDemoState::Phase::EnemyAction) {
        battle.actionTimer += dt.asSeconds();
        if (battle.actionTimer >= battle.enemyActionDelay
            && !battle.friendshipEffect.active
            && !battle.masterBatesEvolution.active) {
            queueEnemyAttack(game);
            battle.phase = BattleDemoState::Phase::PlayerChoice;
            battle.actionTimer = 0.f;
            battle.actionMenuVisible = false;
        }
    }
    else if (battle.phase == BattleDemoState::Phase::Victory && !battle.victoryTransitioned) {
        if (battle.completionClock.getElapsedTime().asSeconds() >= battle.victoryHoldTime) {
            battle.victoryTransitioned = true;
            battle.phase = BattleDemoState::Phase::Complete;
            stopBattleMusic(game);
            if (game.battleReturnToSeminiferous) {
                game.battleReturnToSeminiferous = false;
                core::itemActivation::activateItem(game, "dragoncup_umbra");
                core::itemActivation::activateItem(game, "emblem_ascension");
                game.boostToLevel(100);
                if (auto umbraLocation = Locations::findById(game.locations, LocationId::UmbraOssea))
                    game.setCurrentLocation(umbraLocation);
                game.finalEncounterActive = false;
                game.finalEndingPending = true;
                game.transientDialogue.clear();
                game.transientDialogue.insert(game.transientDialogue.end(), seminiferous_part_two.begin(), seminiferous_part_two.end());
                game.currentDialogue = &game.transientDialogue;
                game.dialogueIndex = 0;
                game.visibleText.clear();
                game.currentProcessedLine.clear();
                game.charIndex = 0;
                game.typewriterClock.restart();
                game.state = GameState::Dialogue;
                game.uiFadeInActive = true;
                game.uiFadeOutActive = false;
                game.uiFadeClock.restart();
            }
            else {
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
}

bool skipBattleAnimations(Game& game) {
    auto& battle = game.battleDemo;
    if (battle.swapPrompt.active || battle.creatureMenuVisible || battle.fightMenuVisible)
        return false;

    auto finalizePulse = [&](BattleDemoState::HpPulse& pulse, float& displayHp, float actualHp) {
        if (!pulse.active)
            return;
        pulse.active = false;
        displayHp = actualHp;
    };

    auto skipOnce = [&]() -> bool {
        if (battle.introPhase != BattleDemoState::IntroPhase::Complete) {
            battle.introPhase = BattleDemoState::IntroPhase::Complete;
            battle.introBlinkTimer = 0.f;
            battle.introBlinkCount = 0;
            battle.introBlinkVisible = false;
            battle.introShadeProgress = 1.f;
            return true;
        }

        if (battle.masterBatesEvolution.active) {
            auto& evolution = battle.masterBatesEvolution;
            if (evolution.stage == BattleDemoState::MasterBatesEvolution::Stage::FirstSound) {
                pushLog(game, "Master Bates is evolving!");
            }
            if (evolution.stage != BattleDemoState::MasterBatesEvolution::Stage::ChickSound
                && evolution.stage != BattleDemoState::MasterBatesEvolution::Stage::AwwSound) {
                if (evolution.sound)
                    evolution.sound->stop();
                evolution.sound.reset();
                completeMasterBatesEvolution(game, battle);
            }
            if (evolution.sound)
                evolution.sound->stop();
            evolution.sound.reset();
            evolution.active = false;
            evolution.stage = BattleDemoState::MasterBatesEvolution::Stage::Idle;
            battle.actionTimer = 0.f;
            return true;
        }

        if (battle.masterBatesSkillEffect.active) {
            auto& effect = battle.masterBatesSkillEffect;
            if (effect.pendingDamage) {
                effect.pendingDamage = false;
                if (effect.target == BattleDemoState::MasterBatesSkillEffect::Target::Enemy) {
                    battle.enemy.hp = effect.pendingHp;
                    battle.enemyDisplayedHp = battle.enemy.hp;
                    battle.enemyHpPulse.active = false;
                    if (battle.enemy.hp <= 0.f)
                        resolveEnemyDefeat(game, battle);
                } else {
                    battle.player.hp = effect.pendingHp;
                    battle.playerDisplayedHp = battle.player.hp;
                    battle.playerHpPulse.active = false;
                    cacheActiveCreatureStats(battle);
                    markActiveCreatureDefeated(battle);
                    maybeFinalizeForcedRetreat(game);
                }
            }
            if (effect.sound)
                effect.sound->stop();
            effect = BattleDemoState::MasterBatesSkillEffect();
            return true;
        }

        if (battle.friendshipEffect.active) {
            auto& effect = battle.friendshipEffect;
            if (effect.pendingDamage && !effect.hpPulseTriggered) {
                effect.pendingDamage = false;
                effect.hpPulseTriggered = true;
                if (effect.target == BattleDemoState::FriendshipEffect::Target::Enemy) {
                    battle.enemy.hp = effect.pendingHp;
                    battle.enemyDisplayedHp = battle.enemy.hp;
                    battle.enemyHpPulse.active = false;
                    resolveEnemyDefeat(game, battle);
                } else if (effect.target == BattleDemoState::FriendshipEffect::Target::Player) {
                    battle.player.hp = effect.pendingHp;
                    battle.playerDisplayedHp = battle.player.hp;
                    battle.playerHpPulse.active = false;
                    cacheActiveCreatureStats(battle);
                    markActiveCreatureDefeated(battle);
                    maybeRecallDragonborn(game, battle);
                    maybeFinalizeForcedRetreat(game);
                }
            }
            if (effect.sound)
                effect.sound->stop();
            effect = BattleDemoState::FriendshipEffect();
            return true;
        }

        if (battle.skillEffect.active) {
            auto& effect = battle.skillEffect;
            if (effect.pendingDamage) {
                effect.pendingDamage = false;
                if (effect.target == BattleDemoState::SkillEffect::Target::Enemy) {
                    battle.enemy.hp = effect.pendingHp;
                    battle.enemyDisplayedHp = battle.enemy.hp;
                    battle.enemyHpPulse.active = false;
                    resolveEnemyDefeat(game, battle);
                } else if (effect.target == BattleDemoState::SkillEffect::Target::Player) {
                    battle.player.hp = effect.pendingHp;
                    battle.playerDisplayedHp = battle.player.hp;
                    battle.playerHpPulse.active = false;
                    cacheActiveCreatureStats(battle);
                    markActiveCreatureDefeated(battle);
                    maybeRecallDragonborn(game, battle);
                    maybeFinalizeForcedRetreat(game);
                }
            }
            if (effect.slashSound)
                effect.slashSound->stop();
            if (effect.elementSound)
                effect.elementSound->stop();
            effect = BattleDemoState::SkillEffect();
            return true;
        }

        if (battle.swapAnimation.active) {
            updateSwapAnimation(game, battle, kSwapPhaseDuration * 5.f);
            return true;
        }

        bool pulseSkipped = false;
        if (battle.playerHpPulse.active) {
            finalizePulse(battle.playerHpPulse, battle.playerDisplayedHp, battle.player.hp);
            pulseSkipped = true;
        }
        if (battle.enemyHpPulse.active) {
            finalizePulse(battle.enemyHpPulse, battle.enemyDisplayedHp, battle.enemy.hp);
            pulseSkipped = true;
        }
        if (pulseSkipped)
            return true;

        if (battle.forcedRetreat.awaitingSwap) {
            maybeFinalizeForcedRetreat(game);
            if (battle.creatureMenuVisible)
                return true;
        }

        if (battle.phase == BattleDemoState::Phase::PlayerAction) {
            if (!battle.skillEffect.active
                && !battle.swapAnimation.active
                && !battle.masterBatesEvolution.active) {
                battle.phase = BattleDemoState::Phase::EnemyAction;
                battle.actionTimer = 0.f;
                battle.actionMenuVisible = false;
                return true;
            }
        } else if (battle.phase == BattleDemoState::Phase::EnemyAction) {
            if (!battle.friendshipEffect.active
                && !battle.masterBatesEvolution.active) {
                queueEnemyAttack(game);
                battle.phase = BattleDemoState::Phase::PlayerChoice;
                battle.actionTimer = 0.f;
                battle.actionMenuVisible = false;
                return true;
            }
        }

        if (battle.reopenMenuAfterPlayerPulse
            && battle.phase == BattleDemoState::Phase::PlayerChoice
            && actionMenuReady(battle)) {
            battle.actionMenuVisible = true;
            battle.reopenMenuAfterPlayerPulse = false;
            return true;
        }

        return false;
    };

    bool skipped = false;
    for (int step = 0; step < 8; ++step) {
        if (!skipOnce())
            break;
        skipped = true;
        if (battle.swapPrompt.active || battle.creatureMenuVisible || battle.fightMenuVisible)
            break;
    }

    return skipped;
}

bool handleEvent(Game& game, const sf::Event& event) {
    if (game.state != GameState::BattleDemo)
        return false;

    auto& battle = game.battleDemo;
    // if (kEnableBattleSkip) {
    //     if (auto key = event.getIf<sf::Event::KeyReleased>()) {
    //         if (key->scancode == sf::Keyboard::Scan::Enter) {
    //             if (skipBattleAnimations(game))
    //                 return true;
    //         }
    //     }
    // }
    if (battle.introPhase != BattleDemoState::IntroPhase::Complete)
        return false;
    if (battle.swapPrompt.active)
        return handleSwapPromptEvent(game, event);
    if (battle.creatureMenuVisible)
        return handleCreatureMenuEvent(game, event);
    if (battle.phase != BattleDemoState::Phase::PlayerChoice)
        return false;

    auto openFightMenu = [&]() -> bool {
        int firstSkill = firstAvailableSkillIndex(battle.player);
        if (firstSkill < 0)
            return false;
        battle.fightMenuSelection = firstSkill;
        battle.fightMenuVisible = true;
        battle.fightCancelHighlight = false;
        battle.actionMenuVisible = true;
        return true;
    };

    auto executeSelectedAction = [&]() -> bool {
        if (battle.selectedAction == 0)
            return openFightMenu();
        if (battle.selectedAction == 1) {
            openCreatureMenu(battle, BattleDemoState::CreatureMenuType::Dragons);
            return true;
        }
        if (battle.selectedAction == 2) {
            openCreatureMenu(battle, BattleDemoState::CreatureMenuType::Glandumon);
            return true;
        }
        if (battle.selectedAction == 3) {
            std::string runningName = displayPlayerName(game, battle);
            pushLog(game, runningName + " tried to run away!\nYou can't run away from a Boss Fight!");
            battle.phase = BattleDemoState::Phase::EnemyAction;
            battle.actionTimer = 0.f;
            battle.actionMenuVisible = false;
            return true;
        }
        return false;
    };

    auto handleFightMenuEvent = [&]() -> bool {
        if (!battle.actionMenuVisible)
            return false;

        auto& player = battle.player;
        auto moveSelectionGrid = [&](int rowDelta, int colDelta) {
            constexpr int columns = 2;
            constexpr int rows = (static_cast<int>(BattleDemoState::kSkillSlotCount) + columns - 1) / columns;
            int row = battle.fightMenuSelection / columns;
            int column = battle.fightMenuSelection % columns;
            row = std::clamp(row + rowDelta, 0, rows - 1);
            column = std::clamp(column + colDelta, 0, columns - 1);
            int candidate = (row * columns) + column;
            if (candidate >= static_cast<int>(BattleDemoState::kSkillSlotCount))
                return;
            if (skillSlotAvailable(player, candidate))
                battle.fightMenuSelection = candidate;
        };

        if (auto key = event.getIf<sf::Event::KeyReleased>()) {
            switch (key->scancode) {
                case sf::Keyboard::Scan::Left:
                    moveSelectionGrid(0, -1);
                    return true;
                case sf::Keyboard::Scan::Right:
                    moveSelectionGrid(0, 1);
                    return true;
                case sf::Keyboard::Scan::Up:
                    moveSelectionGrid(-1, 0);
                    return true;
                case sf::Keyboard::Scan::Down:
                    moveSelectionGrid(1, 0);
                    return true;
                case sf::Keyboard::Scan::Enter:
                case sf::Keyboard::Scan::Space:
                case sf::Keyboard::Scan::Z:
                    if (skillSlotAvailable(player, battle.fightMenuSelection)) {
                        beginPlayerAction(game, battle.fightMenuSelection);
                        return true;
                    }
                    return false;
                case sf::Keyboard::Scan::Escape:
                case sf::Keyboard::Scan::Backspace:
                    battle.fightMenuVisible = false;
                    battle.fightCancelHighlight = false;
                    battle.selectedAction = 0;
                    battle.actionMenuVisible = true;
                    battle.fightMenuSelection = firstAvailableSkillIndex(player);
                    return true;
                default:
                    return false;
            }
        }

        if (auto mouse = event.getIf<sf::Event::MouseMoved>()) {
            auto mousePos = game.window.mapPixelToCoords(mouse->position);
            bool hoveredSkill = false;
            if (battle.fightOptionBoundsValid) {
                for (int index = 0; index < static_cast<int>(battle.fightOptionBounds.size()); ++index) {
                    if (battle.fightOptionBounds[index].contains(mousePos) && skillSlotAvailable(player, index)) {
                        battle.fightMenuSelection = index;
                        hoveredSkill = true;
                        break;
                    }
                }
            }
            battle.fightCancelHighlight = !hoveredSkill && battle.fightCancelBounds.contains(mousePos);
            return hoveredSkill || battle.fightCancelHighlight;
        }

        if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (button->button == sf::Mouse::Button::Left) {
                auto mousePos = game.window.mapPixelToCoords(button->position);
                if (battle.fightOptionBoundsValid) {
                    for (int index = 0; index < static_cast<int>(battle.fightOptionBounds.size()); ++index) {
                        if (battle.fightOptionBounds[index].contains(mousePos) && skillSlotAvailable(player, index)) {
                            battle.fightMenuSelection = index;
                            beginPlayerAction(game, index);
                            return true;
                        }
                    }
                }
                if (battle.fightCancelBounds.contains(mousePos)) {
                    battle.fightMenuVisible = false;
                    battle.fightCancelHighlight = false;
                    battle.actionMenuVisible = true;
                    battle.selectedAction = 0;
                    return true;
                }
            }
        }

        return false;
    };

    if (battle.fightMenuVisible)
        return handleFightMenuEvent();

    if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        auto moveSelectionGrid = [&](int rowDelta, int colDelta) {
            constexpr int columns = 2;
            constexpr int rows = (static_cast<int>(kActionLabels.size()) + columns - 1) / columns;
            int row = battle.selectedAction / columns;
            int column = battle.selectedAction % columns;
            row = std::clamp(row + rowDelta, 0, rows - 1);
            column = std::clamp(column + colDelta, 0, columns - 1);
            int nextIndex = (row * columns) + column;
            if (nextIndex >= static_cast<int>(kActionLabels.size()))
                nextIndex = static_cast<int>(kActionLabels.size()) - 1;
            battle.selectedAction = nextIndex;
        };

        switch (key->scancode) {
            case sf::Keyboard::Scan::Left:
                if (!battle.actionMenuVisible)
                    return false;
                moveSelectionGrid(0, -1);
                return true;
            case sf::Keyboard::Scan::Right:
                if (!battle.actionMenuVisible)
                    return false;
                moveSelectionGrid(0, 1);
                return true;
            case sf::Keyboard::Scan::Up:
                if (!battle.actionMenuVisible)
                    return false;
                moveSelectionGrid(-1, 0);
                return true;
            case sf::Keyboard::Scan::Down:
                if (!battle.actionMenuVisible)
                    return false;
                moveSelectionGrid(1, 0);
                return true;
                case sf::Keyboard::Scan::Enter:
                case sf::Keyboard::Scan::Space:
                case sf::Keyboard::Scan::Z:
                    if (!battle.actionMenuVisible) {
                        if (actionMenuReady(battle)) {
                            battle.actionMenuVisible = true;
                            battle.selectedAction = 0;
                        }
                        return true;
                    }
                    return executeSelectedAction();
                default:
                    return false;
            }
        }

    if (battle.actionMenuVisible && battle.actionOptionBoundsValid) {
        if (auto mouse = event.getIf<sf::Event::MouseMoved>()) {
            auto mousePos = game.window.mapPixelToCoords(mouse->position);
            for (std::size_t index = 0; index < battle.actionOptionBounds.size(); ++index) {
                if (battle.actionOptionBounds[index].contains(mousePos)) {
                    battle.selectedAction = static_cast<int>(index);
                    return true;
                }
            }
        }
    }

    if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (button->button == sf::Mouse::Button::Left) {
                auto mousePos = game.window.mapPixelToCoords(button->position);
                if (!battle.actionMenuVisible) {
                    if (actionMenuReady(battle)) {
                        battle.actionMenuVisible = true;
                        battle.selectedAction = 0;
                    }
                    return true;
                }
            if (battle.actionOptionBoundsValid) {
                for (std::size_t index = 0; index < battle.actionOptionBounds.size(); ++index) {
                    if (battle.actionOptionBounds[index].contains(mousePos)) {
                        battle.selectedAction = static_cast<int>(index);
                        return executeSelectedAction();
                    }
                }
            }
            return executeSelectedAction();
        }
    }

    return false;
}

void draw(Game& game, sf::RenderTarget& target) {
    const auto size = target.getSize();
    float width = static_cast<float>(size.x);
    float height = static_cast<float>(size.y);

    auto& battle = game.battleDemo;
    auto& evolution = battle.masterBatesEvolution;
    auto stage = evolution.stage;
    bool shouldShakeView = (battle.introPhase == BattleDemoState::IntroPhase::Complete)
        && evolution.active
        && (stage == BattleDemoState::MasterBatesEvolution::Stage::FadeWhite
            || stage == BattleDemoState::MasterBatesEvolution::Stage::FadePurple);
    sf::View originalView = target.getView();
    auto restoreView = [&]() {
        if (shouldShakeView)
            target.setView(originalView);
    };
    if (shouldShakeView) {
        sf::View shakenView = originalView;
        shakenView.move(evolution.shakeOffset);
        target.setView(shakenView);
    }
    sf::Sprite background = makeLayer(game.resources.battleBackgroundLayer, { width, height });
    auto backgroundBounds = background.getGlobalBounds();
    auto backgroundPosition = sf::Vector2f{ (width - backgroundBounds.size.x) * 0.5f, (height - backgroundBounds.size.y) * 0.5f };
    background.setPosition(backgroundPosition);
    if (battle.introPhase == BattleDemoState::IntroPhase::Blinking) {
        drawIntroBlink(game, target, width, height, battle);
        restoreView();
        return;
    }
    target.draw(background);

    if (battle.introPhase == BattleDemoState::IntroPhase::Shading) {
        drawShadeOverlay(target, width, height, battle.introShadeProgress);
        restoreView();
        return;
    }

    if (battle.creatureMenuVisible) {
        drawCreatureMenuOverlay(game, target, backgroundPosition, backgroundBounds);
        if (battle.swapPrompt.active)
            drawSwapPrompt(game, target, { width, height }, battle);
        restoreView();
        return;
    }

    auto drawLayer = [&](const sf::Texture& texture, const sf::Vector2f& targetSize) -> sf::FloatRect {
        sf::Sprite sprite = makeLayer(texture, targetSize);
        sprite.setPosition(backgroundPosition);
        target.draw(sprite);
        return sprite.getGlobalBounds();
    };

    auto createLayerSprite = [&](const sf::Texture& texture) {
        sf::Sprite sprite = makeLayer(texture, backgroundBounds.size);
        sprite.setPosition(backgroundPosition);
        return sprite;
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
                                float extraLeftOffset, float extraBottomOffset, float displayHp) -> std::optional<sf::FloatRect> {
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
            ? std::clamp(displayHp / combatant.maxHp, 0.f, 1.f)
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
                                 const BattleDemoState::Combatant& combatant, float extraLeftOffset, float displayHp) {
        std::string hpTextValue = formatHpValue(displayHp, combatant.maxHp);
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

    sf::Sprite playerPlatformSprite = createLayerSprite(game.resources.battlePlayerPlatform);
    sf::FloatRect playerPlatformBounds = playerPlatformSprite.getGlobalBounds();
    sf::Sprite enemyPlatformSprite = createLayerSprite(game.resources.battleEnemyPlatform);
    sf::FloatRect enemyPlatformBounds = enemyPlatformSprite.getGlobalBounds();

    float entranceDuration = battle.platformEntranceDuration;
    float entranceTimer = battle.platformEntranceTimer;
    float entranceRatio = (entranceDuration > 0.f)
        ? std::clamp(entranceTimer / entranceDuration, 0.f, 1.f)
        : 1.f;
    bool platformsArrived = entranceRatio >= 1.f;

    auto lerp = [](float start, float end, float ratio) {
        return start + (end - start) * ratio;
    };

    float playerTargetX = playerPlatformBounds.position.x;
    float playerStartX = playerTargetX + playerPlatformBounds.size.x;
    float enemyTargetX = enemyPlatformBounds.position.x;
    float enemyStartX = enemyTargetX - enemyPlatformBounds.size.x;

    float playerDrawX = lerp(playerStartX, playerTargetX, entranceRatio);
    float enemyDrawX = lerp(enemyStartX, enemyTargetX, entranceRatio);
    float playerOffsetX = playerDrawX - playerTargetX;
    float enemyOffsetX = enemyDrawX - enemyTargetX;

    float playerFinalY = playerPlatformSprite.getPosition().y;
    float enemyFinalY = enemyPlatformSprite.getPosition().y;

    playerPlatformSprite.setPosition({ playerDrawX, playerFinalY });
    enemyPlatformSprite.setPosition({ enemyDrawX, enemyFinalY });
    target.draw(playerPlatformSprite);
    target.draw(enemyPlatformSprite);

    const sf::Texture* defaultBack = (game.playerGender == Game::DragonbornGender::Female)
        ? &game.resources.spriteDragonbornFemaleBack
        : &game.resources.spriteDragonbornMaleBack;
    if (!battle.playerBackSprite)
        battle.playerBackSprite = defaultBack;
    const sf::Texture* playerBackTexture = battle.playerBackSprite ? battle.playerBackSprite : defaultBack;
    sf::Sprite playerSprite(*playerBackTexture);
    auto spriteBounds = playerSprite.getLocalBounds();
    sf::Vector2f spriteOrigin = normalizedOpaqueCenter(*playerBackTexture);
    playerSprite.setOrigin(sf::Vector2f{ spriteOrigin.x * spriteBounds.size.x, spriteOrigin.y * spriteBounds.size.y });
    playerSprite.setScale(sf::Vector2f{ 0.30f, 0.30f });
    sf::Vector2f playerCenterNormalized = normalizedOpaqueCenter(game.resources.battlePlayerPlatform);
    sf::Vector2f playerCenter{
        playerPlatformBounds.position.x + playerPlatformBounds.size.x * playerCenterNormalized.x,
        playerPlatformBounds.position.y + playerPlatformBounds.size.y * playerCenterNormalized.y
    };
    playerCenter.x += playerOffsetX;
    auto computeRetreatProgress = [&](const BattleDemoState::HpPulse& pulse, float hp, float threshold) {
        if (hp > threshold)
            return 0.f;
        if (pulse.active) {
            float ratio = std::clamp(pulse.clock.getElapsedTime().asSeconds() / kBattleHpPulseDuration, 0.f, 1.f);
            return ratio;
        }
        return 1.f;
    };
    float dropDistance = height * 1.2f;
    float heroThreshold = battle.currentDragonbornActive ? 1.f : 0.f;
    float playerDropProgress = computeRetreatProgress(battle.playerHpPulse, battle.player.hp, heroThreshold);
    float playerDropOffset = dropDistance * playerDropProgress;
    sf::Vector2f playerPosition = playerCenter + sf::Vector2f{ 0.f, playerDropOffset };
    playerSprite.setPosition(playerPosition);
    sf::FloatRect playerGlobalBounds = playerSprite.getGlobalBounds();
    sf::Vector2f playerSpriteCenter{
        playerGlobalBounds.position.x + (playerGlobalBounds.size.x * 0.5f),
        playerGlobalBounds.position.y + (playerGlobalBounds.size.y * 0.5f)
    };
    battle.cachedPlayerCenter = playerSpriteCenter;
    bool skipPlayerSprite = false;
    if (battle.swapAnimation.active) {
        using Stage = BattleDemoState::SwapAnimation::Stage;
        auto stage = battle.swapAnimation.stage;
        skipPlayerSprite = (stage == Stage::OutgoingWhite || stage == Stage::OutgoingSoftRed);
    }
    if (!skipPlayerSprite)
        target.draw(playerSprite);
    if (!skipPlayerSprite) {
        if (auto overlayColor = computeSwapOverlayColor(battle)) {
            sf::Sprite overlay = playerSprite;
            overlay.setColor(*overlayColor);
            target.draw(overlay);
        }
    }

    sf::FloatRect textBoxBounds = drawLayer(game.resources.battleTextBox, backgroundBounds.size);
    sf::FloatRect textBoxContent = computeBoxBounds(game.resources.battleTextBox);
    if ((textBoxContent.size.x <= 0.f || textBoxContent.size.y <= 0.f)
        && textBoxBounds.size.x > 0.f && textBoxBounds.size.y > 0.f)
        textBoxContent = textBoxBounds;
    if (textBoxContent.size.x > 0.f && textBoxContent.size.y > 0.f) {
        std::string displayText;
        std::vector<std::pair<std::string, sf::Color>> displayTokens;
        if (battle.actionMenuVisible) {
            std::string playerLabel = displayPlayerName(game, battle);
            displayText = "What will " + playerLabel + " do?";
            displayTokens = buildLogHighlightTokens(game, battle, displayText);
        } else if (!battle.logHistory.empty()) {
            const auto& entry = battle.logHistory.back();
            displayText = entry.message;
            if (entry.highlightTokens.empty()) {
                displayTokens = buildLogHighlightTokens(game, battle, displayText);
            } else {
                displayTokens = entry.highlightTokens;
            }
        }
        if (!displayText.empty()) {
            sf::Text logEntry{ game.resources.battleFont, "", 32 };
            logEntry.setStyle(sf::Text::Bold);
            logEntry.setLetterSpacing(logEntry.getLetterSpacing() + 0.5f);
            constexpr float kTextOffset = 40.f;
            float availableWidth = std::max(0.f, textBoxContent.size.x - (kTextOffset * 2.f));
            auto wrapText = [&](const std::string& input) {
                std::vector<std::string> lines;
                std::istringstream tokens(input);
                std::string word;
                std::string currentLine;
                while (tokens >> word) {
                    std::string candidate = currentLine.empty() ? word : (currentLine + " " + word);
                    logEntry.setString(candidate);
                    auto candidateBounds = logEntry.getGlobalBounds();
                    if (!currentLine.empty() && candidateBounds.size.x > availableWidth) {
                        lines.push_back(currentLine);
                        currentLine = word;
                    } else {
                        currentLine = candidate;
                    }
                }
                if (!currentLine.empty())
                    lines.push_back(currentLine);
                return lines;
            };
            struct ColoredSegment {
                std::string text;
                sf::Color color;
            };
            auto colorLineSegments = [&](std::string_view line) {
                std::vector<ColoredSegment> segments;
                std::vector<std::pair<std::string, sf::Color>> sortedTokens = displayTokens;
                std::sort(sortedTokens.begin(), sortedTokens.end(), [](const auto& a, const auto& b) {
                    return a.first.size() > b.first.size();
                });
                std::string buffer;
                auto flushBuffer = [&]() {
                    if (!buffer.empty()) {
                        segments.push_back({ std::move(buffer), ColorHelper::Palette::Normal });
                        buffer.clear();
                    }
                };
                size_t offset = 0;
                while (offset < line.size()) {
                    bool matched = false;
                    for (const auto& token : sortedTokens) {
                        if (token.first.empty())
                            continue;
                        if (offset + token.first.size() <= line.size()
                            && line.compare(offset, token.first.size(), token.first) == 0) {
                            flushBuffer();
                            segments.push_back({ token.first, token.second });
                            offset += token.first.size();
                            matched = true;
                            break;
                        }
                    }
                    if (!matched) {
                        buffer.push_back(line[offset++]);
                    }
                }
                flushBuffer();
                if (segments.empty())
                    segments.push_back({ std::string(line), ColorHelper::Palette::Normal });
                return segments;
            };
            constexpr float kLineSpacing = 30.f;
            float baseX = textBoxContent.position.x + kTextOffset;
            float baseY = textBoxContent.position.y + kTextOffset;
            float currentY = baseY;
            float contentBottom = textBoxContent.position.y + textBoxContent.size.y - kLineSpacing;
            std::vector<std::string> lines = wrapText(displayText);
            for (const auto& line : lines) {
                if (currentY > contentBottom)
                    break;
                float drawX = baseX;
                std::vector<ColoredSegment> segments = colorLineSegments(line);
                for (const auto& segment : segments) {
                    logEntry.setString(segment.text);
                    logEntry.setFillColor(segment.color);
                    logEntry.setPosition({ drawX, currentY });
                    target.draw(logEntry);
                    drawX += logEntry.getGlobalBounds().size.x;
                }
                currentY += kLineSpacing;
            }
        }
    }
    sf::FloatRect actionBoxBounds = drawLayer(game.resources.battleActionBox, backgroundBounds.size);
    sf::FloatRect actionBoxContent = computeBoxBounds(game.resources.battleActionBox);
    if ((actionBoxContent.size.x <= 0.f || actionBoxContent.size.y <= 0.f)
        && actionBoxBounds.size.x > 0.f && actionBoxBounds.size.y > 0.f)
        actionBoxContent = actionBoxBounds;
    battle.actionOptionBoundsValid = false;
    battle.fightOptionBoundsValid = false;
    battle.fightCancelBounds = {};
    if (actionBoxContent.size.x > 0.f && actionBoxContent.size.y > 0.f && battle.actionMenuVisible) {
        constexpr float kActionColumnPadding = 28.f;
        constexpr float kActionRowPadding = 20.f;
        constexpr float kActionMenuArrowWidth = 18.f;
        constexpr float kActionMenuArrowSpacing = 6.f;
        float leftColumnX = actionBoxContent.position.x + (actionBoxContent.size.x * 0.15f);
        float rightColumnX = actionBoxContent.position.x + (actionBoxContent.size.x * 0.55f);
        std::array<float, 2> columnPositions = { leftColumnX, rightColumnX };
        std::array<float, 2> rowPositions = {
            actionBoxContent.position.y + (actionBoxContent.size.y * 0.35f),
            actionBoxContent.position.y + (actionBoxContent.size.y * 0.65f)
        };

        if (battle.fightMenuVisible) {
            float returnWidth = 140.f;
            float returnHeight = 44.f;
            float returnX = actionBoxContent.position.x + 24.f;
            float returnY = actionBoxContent.position.y + 24.f;
            sf::Text returnLabel{ game.resources.battleFont, "RETURN", 24 };
            returnLabel.setLetterSpacing(returnLabel.getLetterSpacing() + 0.5f);
            auto returnTextBounds = returnLabel.getLocalBounds();
            float returnCenterX = returnX + (returnWidth * 0.5f);
            float returnWordLeft = returnCenterX - (returnTextBounds.size.x * 0.5f);
            float leftFightX = returnWordLeft;
            std::array<std::string, BattleDemoState::kSkillSlotCount> skillLabels;
            std::array<bool, BattleDemoState::kSkillSlotCount> skillAvailable{};
            for (int index = 0; index < static_cast<int>(BattleDemoState::kSkillSlotCount); ++index) {
                skillAvailable[index] = skillSlotAvailable(battle.player, index);
                skillLabels[index] = skillAvailable[index]
                    ? *battle.player.skills[index]
                    : "-------";
            }
            sf::Text measurement{ game.resources.battleFont, "", 48 };
            measurement.setLetterSpacing(measurement.getLetterSpacing() + 0.5f);
            float longestLeftWidth = 0.f;
            for (int idx : {0, 2}) {
                measurement.setString(skillLabels[idx]);
                longestLeftWidth = std::max(longestLeftWidth, measurement.getLocalBounds().size.x);
            }
            float spacing = longestLeftWidth + (kActionMenuArrowWidth * 2.f);
            float rightFightX = leftFightX + spacing;
            std::array<float, 2> fightRowPositions = {
                actionBoxContent.position.y + (actionBoxContent.size.y * 0.45f),
                actionBoxContent.position.y + (actionBoxContent.size.y * 0.72f)
            };
            for (int index = 0; index < static_cast<int>(BattleDemoState::kSkillSlotCount); ++index) {
                bool available = skillAvailable[index];
                const std::string& label = skillLabels[index];
                sf::Text option{ game.resources.battleFont, label, 48 };
                option.setStyle(available ? sf::Text::Bold : sf::Text::Regular);
                option.setLetterSpacing(option.getLetterSpacing() + 0.5f);
                int row = index / 2;
                int column = index % 2;
                float targetX = (column == 0) ? leftFightX : rightFightX;
                float targetY = fightRowPositions[row];
                bool highlighted = (battle.fightMenuSelection == index && available);
                sf::Color baseColor = highlighted ? skillHighlightColor(label) : sf::Color::White;
                option.setFillColor(available ? baseColor : sf::Color(160, 160, 170));
                auto optionBounds = option.getLocalBounds();
                option.setOrigin({
                    optionBounds.position.x,
                    optionBounds.position.y + (optionBounds.size.y * 0.5f)
                });
                option.setPosition({ targetX, targetY });
                target.draw(option);
                if (index < static_cast<int>(battle.fightOptionBounds.size()))
                    battle.fightOptionBounds[index] = option.getGlobalBounds();
            }
            battle.fightOptionBoundsValid = true;

            battle.fightCancelBounds = sf::FloatRect(
                { returnX, returnY },
                { returnWidth, returnHeight }
            );
            RoundedRectangleShape returnShape({ returnWidth, returnHeight }, returnHeight * 0.25f, 8);
            returnShape.setPosition(battle.fightCancelBounds.position);
            returnShape.setFillColor(sf::Color::Transparent);
            returnShape.setOutlineThickness(1.5f);
            returnShape.setOutlineColor(battle.fightCancelHighlight ? sf::Color::White : sf::Color(200, 200, 200));
            target.draw(returnShape);

            returnLabel.setFillColor(sf::Color::White);
            auto returnLabelBounds = returnLabel.getLocalBounds();
            returnLabel.setOrigin({
                returnLabelBounds.position.x + (returnLabelBounds.size.x * 0.5f),
                returnLabelBounds.position.y + (returnLabelBounds.size.y * 0.5f)
            });
            returnLabel.setPosition({
                battle.fightCancelBounds.position.x + (battle.fightCancelBounds.size.x * 0.5f),
                battle.fightCancelBounds.position.y + (battle.fightCancelBounds.size.y * 0.5f)
            });
            target.draw(returnLabel);
        } else {
            battle.actionOptionBoundsValid = true;
            for (std::size_t index = 0; index < kActionLabels.size(); ++index) {
                sf::Text option{ game.resources.battleFont, kActionLabels[index], 48 };
                option.setFillColor(sf::Color::White);
                option.setStyle(sf::Text::Bold);
                option.setLetterSpacing(option.getLetterSpacing() + 0.5f);
                auto optionBounds = option.getLocalBounds();

                int row = static_cast<int>(index / 2);
                int column = static_cast<int>(index % 2);
                float targetX = columnPositions[column];
                float targetY = rowPositions[row];
                option.setOrigin({
                    optionBounds.position.x,
                    optionBounds.position.y + (optionBounds.size.y * 0.5f)
                });
                option.setPosition({ targetX, targetY });
                target.draw(option);
                if (index < battle.actionOptionBounds.size())
                    battle.actionOptionBounds[index] = option.getGlobalBounds();

                if (battle.selectedAction == static_cast<int>(index)) {
                    sf::ConvexShape arrow(3);
                    arrow.setPoint(0, { 0.f, -9.f });
                    arrow.setPoint(1, { kActionMenuArrowWidth, 0.f });
                    arrow.setPoint(2, { 0.f, 9.f });
                    arrow.setFillColor(sf::Color::White);
                    float arrowX = targetX - kActionMenuArrowSpacing - kActionMenuArrowWidth;
                    float arrowY = targetY;
                    arrow.setPosition({ arrowX, arrowY });
                    target.draw(arrow);
                }
            }
        }
    }
    if (platformsArrived) {
        drawLayer(game.resources.battlePlayerBox, backgroundBounds.size);
        drawLayer(game.resources.battleEnemyBox, backgroundBounds.size);

        sf::FloatRect playerBoxBounds = computeBoxBounds(game.resources.battlePlayerBox);
        sf::FloatRect enemyBoxBounds = computeBoxBounds(game.resources.battleEnemyBox);

        auto playerHealthBar = drawHealthWidget(playerBoxBounds, battle.player, 40.f, 0.f, battle.playerDisplayedHp);
        drawHealthWidget(enemyBoxBounds, battle.enemy, 40.f, -40.f, battle.enemyDisplayedHp);
        drawLevelBadge(playerBoxBounds, battle.player, 10.f, 30.f);
        drawLevelBadge(enemyBoxBounds, battle.enemy, 10.f, 15.f);
        if (playerHealthBar)
            drawPlayerHpValue(*playerHealthBar, playerBoxBounds, battle.player, 10.f, battle.playerDisplayedHp);
        auto playerGender = genderFromName(battle.player.name);
        const sf::Texture* playerIcon = playerGender
            ? genderIconFor(game, *playerGender)
            : genderIconFor(game, game.playerGender);
        auto enemyGender = genderFromName(battle.enemy.name);
        const sf::Texture* enemyIcon = enemyGender ? genderIconFor(game, *enemyGender) : nullptr;
        std::string playerLabel = displayPlayerName(game, battle);
        drawNameField(playerBoxBounds, playerLabel, 50.f, 30.f, playerIcon);
        drawNameField(enemyBoxBounds, "Master Bates", 50.f, 15.f, enemyIcon);
    }

    const sf::Texture* enemyTexture = battle.masterBatesDragonActive
        ? &game.resources.spriteMasterBatesDragon
        : &game.resources.spriteMasterBates;
    sf::Sprite enemySprite(*enemyTexture);
    auto enemyBounds = enemySprite.getLocalBounds();
    sf::Vector2f enemyBottom = normalizedOpaqueBottom(*enemyTexture);
    enemySprite.setOrigin(sf::Vector2f{ enemyBottom.x * enemyBounds.size.x, enemyBottom.y * enemyBounds.size.y });
    enemySprite.setScale(sf::Vector2f{ 0.20f, 0.20f });
    sf::Vector2f enemyCenterNormalized = normalizedOpaqueCenter(game.resources.battleEnemyPlatform);
    sf::Vector2f enemyCenter{
        enemyPlatformBounds.position.x + enemyPlatformBounds.size.x * enemyCenterNormalized.x,
        enemyPlatformBounds.position.y + enemyPlatformBounds.size.y * enemyCenterNormalized.y
    };
    enemyCenter.x += enemyOffsetX;
    sf::Vector2f anchorOffset{
        -enemyPlatformBounds.size.x * 0.03f,
        enemyPlatformBounds.size.y * 0.05f
    };
    bool skipEnemyRetreat = isMasterBatesName(battle.enemy.name) || battle.masterBatesDragonActive;
    float enemyDropProgress = skipEnemyRetreat ? 0.f : computeRetreatProgress(battle.enemyHpPulse, battle.enemy.hp, 0.f);
    float enemyDropOffset = dropDistance * enemyDropProgress;
    enemySprite.setPosition(enemyCenter + anchorOffset + sf::Vector2f{ 0.f, enemyDropOffset });
    sf::FloatRect enemyGlobalBounds = enemySprite.getGlobalBounds();
    sf::Vector2f enemySpriteCenter{
        enemyGlobalBounds.position.x + (enemyGlobalBounds.size.x * 0.5f),
        enemyGlobalBounds.position.y + (enemyGlobalBounds.size.y * 0.5f)
    };
    battle.cachedEnemyCenter = enemySpriteCenter;
    sf::Color enemyTint = sf::Color::White;
    if (evolution.active)
        enemyTint = evolution.tint;
    enemySprite.setColor(enemyTint);
    target.draw(enemySprite);
    drawSkillEffect(
        game,
        battle,
        target,
        playerSpriteCenter,
        enemySpriteCenter,
        playerPlatformBounds,
        enemyPlatformBounds
    );
    drawFriendshipEffect(
        game,
        battle,
        target
    );
    drawMasterBatesSkillEffect(game, battle, target);
    if (battle.swapPrompt.active)
        drawSwapPrompt(game, target, { width, height }, battle);
    restoreView();
}

} // namespace ui::battle
