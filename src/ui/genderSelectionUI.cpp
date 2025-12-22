#include "genderSelectionUI.hpp"

#include <algorithm>
#include <array>
#include <string>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include "confirmationUI.hpp"
#include "helper/colorHelper.hpp"
#include "story/dialogInput.hpp"
#include "story/textStyles.hpp"

namespace {
    void restoreGenderSelectionLine(Game& game, const std::string& text) {
        if (!text.empty()) {
            game.visibleText = text;
            game.charIndex = static_cast<std::size_t>(text.size());
            game.currentProcessedLine = text;
        }
        else {
            game.visibleText.clear();
            game.charIndex = 0;
            game.currentProcessedLine.clear();
        }
        game.typewriterClock.restart();
    }

    int genderIndexAtPoint(const Game& game, const sf::Vector2f& point) {
        for (int idx = 0; idx < static_cast<int>(game.genderSelectionBounds.size()); ++idx) {
            if (game.genderSelectionBounds[idx].contains(point))
                return idx;
        }
        return -1;
    }

    void playButtonHoverSound(Game& game) {
        if (game.genderAnimation.phase != Game::GenderSelectionAnimation::Phase::Idle || !game.buttonHoverSound)
            return;
        game.buttonHoverSound->stop();
        game.buttonHoverSound->play();
    }
}

namespace ui::genderSelection {

void start(Game& game) {
    game.genderSelectionActive = true;
    game.genderSelectionHovered = -1;
    game.genderSelectionBounds.fill({});
    game.genderAnimation.phase = Game::GenderSelectionAnimation::Phase::Idle;
    game.genderAnimation.selection = -1;
    game.genderAnimation.labelsHidden = false;
}

void stop(Game& game) {
    game.genderSelectionActive = false;
    game.genderSelectionHovered = -1;
    game.genderAnimation.labelsHidden = false;
}

void choose(Game& game, int idx) {
    if (idx < 0 || idx >= 2)
        return;
    if (game.confirmationPrompt.active)
        return;

    game.genderSelectionHovered = idx;
    auto& animation = game.genderAnimation;
    animation.selection = idx;
    animation.phase = Game::GenderSelectionAnimation::Phase::Approaching;
    animation.clock.restart();
    animation.labelsHidden = true;

    std::string currentLineText;
    if (game.currentDialogue && game.dialogueIndex < game.currentDialogue->size())
        currentLineText = injectSpeakerNames((*game.currentDialogue)[game.dialogueIndex].text, game);

    showConfirmationPrompt(
        game,
        "Are you happy with your choice?",
        [idx, currentLineText](Game& confirmedGame) {
            restoreGenderSelectionLine(confirmedGame, currentLineText);
            auto& animation = confirmedGame.genderAnimation;
            if (animation.phase == Game::GenderSelectionAnimation::Phase::Approaching) {
                animation.phase = Game::GenderSelectionAnimation::Phase::FadingOut;
                animation.clock.restart();
                animation.labelsHidden = true;
            }
        },
        [idx, currentLineText](Game& retryGame) {
            restoreGenderSelectionLine(retryGame, currentLineText);
            auto& animation = retryGame.genderAnimation;
            animation.phase = Game::GenderSelectionAnimation::Phase::Reverting;
            animation.clock.restart();
            retryGame.genderSelectionHovered = -1;
            animation.labelsHidden = false;
        }
    );
}

bool handleEvent(Game& game, const sf::Event& event) {
    if (!game.genderSelectionActive)
        return false;

    if (game.genderAnimation.phase != Game::GenderSelectionAnimation::Phase::Idle)
        return true;

    if (auto move = event.getIf<sf::Event::MouseMoved>()) {
        auto mousePos = game.window.mapPixelToCoords(move->position);
        int hovered = genderIndexAtPoint(game, mousePos);
        if (hovered != game.genderSelectionHovered) {
            game.genderSelectionHovered = hovered;
            if (hovered >= 0)
                playButtonHoverSound(game);
        }
        return true;
    }

    if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button == sf::Mouse::Button::Left) {
            auto mousePos = game.window.mapPixelToCoords(button->position);
            int clicked = genderIndexAtPoint(game, mousePos);
            if (clicked >= 0)
                choose(game, clicked);
        }
        return true;
    }

    if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        switch (key->scancode) {
            case sf::Keyboard::Scan::Left:
                if (game.genderSelectionHovered != 0) {
                    game.genderSelectionHovered = 0;
                    playButtonHoverSound(game);
                }
                return true;
            case sf::Keyboard::Scan::Right:
                if (game.genderSelectionHovered != 1) {
                    game.genderSelectionHovered = 1;
                    playButtonHoverSound(game);
                }
                return true;
            case sf::Keyboard::Scan::Enter:
                if (game.genderSelectionHovered >= 0)
                    choose(game, game.genderSelectionHovered);
                return true;
            default:
                return true;
        }
    }

    return true;
}

void draw(Game& game, sf::RenderTarget& target, float uiAlphaFactor) {
    if (!game.genderSelectionActive)
        return;

    auto windowSize = target.getSize();
    constexpr float kSpriteSpacing = 62.f;
    constexpr float kMaxHeightFactor = 0.35f;
    constexpr float kMaxWidthFactor = 0.28f;
    constexpr float kLabelSpacing = 18.f;
    constexpr float kHintSpacing = 22.f;

    auto scaleForTexture = [&](const sf::Texture& texture) {
        float maxHeight = static_cast<float>(windowSize.y) * kMaxHeightFactor;
        float maxWidth = static_cast<float>(windowSize.x) * kMaxWidthFactor;
        float scaleX = maxWidth / static_cast<float>(texture.getSize().x);
        float scaleY = maxHeight / static_cast<float>(texture.getSize().y);
        return std::min(scaleX, scaleY);
    };

    float femaleScale = scaleForTexture(game.resources.dragonbornFemaleSprite);
    float maleScale = scaleForTexture(game.resources.dragonbornMaleSprite);

    const sf::Texture& femaleTexture = (game.genderSelectionHovered == 0)
        ? game.resources.dragonbornFemaleHoveredSprite
        : game.resources.dragonbornFemaleSprite;
    const sf::Texture& maleTexture = (game.genderSelectionHovered == 1)
        ? game.resources.dragonbornMaleHoveredSprite
        : game.resources.dragonbornMaleSprite;

    sf::Sprite femaleSprite(femaleTexture);
    sf::Sprite maleSprite(maleTexture);
    femaleSprite.setScale({ femaleScale, femaleScale });
    maleSprite.setScale({ maleScale, maleScale });

    auto centerOrigin = [](sf::Sprite& sprite) {
        auto bounds = sprite.getLocalBounds();
        sprite.setOrigin({ bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f });
    };
    centerOrigin(femaleSprite);
    centerOrigin(maleSprite);

    float femaleWidth = femaleSprite.getGlobalBounds().size.x;
    float maleWidth = maleSprite.getGlobalBounds().size.x;
    float totalWidth = femaleWidth + maleWidth + kSpriteSpacing;
    float startX = (static_cast<float>(windowSize.x) - totalWidth) * 0.5f;
    float spriteY = static_cast<float>(windowSize.y) * 0.36f;
    float femaleStartX = startX + femaleWidth * 0.5f;
    float maleStartX = startX + femaleWidth + kSpriteSpacing + maleWidth * 0.5f;

    femaleSprite.setPosition({ femaleStartX, spriteY });
    maleSprite.setPosition({ maleStartX, spriteY });
    femaleSprite.setColor(sf::Color::White);
    maleSprite.setColor(sf::Color::White);

    sf::FloatRect femaleBaseBounds = femaleSprite.getGlobalBounds();
    sf::FloatRect maleBaseBounds = maleSprite.getGlobalBounds();

    auto& animation = game.genderAnimation;
    auto phase = animation.phase;
    float animationProgress = 0.f;
    bool advanceAfterDraw = false;
    if (phase != Game::GenderSelectionAnimation::Phase::Idle) {
        float duration = (phase == Game::GenderSelectionAnimation::Phase::Approaching)
            ? animation.approachDuration
            : animation.fadeDuration;
        animationProgress = duration > 0.f
            ? std::clamp(animation.clock.getElapsedTime().asSeconds() / duration, 0.f, 1.f)
            : 1.f;
    }

    float centerX = static_cast<float>(windowSize.x) * 0.5f;
    if (phase == Game::GenderSelectionAnimation::Phase::Approaching) {
        float drawProgress = animationProgress;
        if (animation.selection == 0) {
            femaleSprite.setPosition({ femaleStartX + (centerX - femaleStartX) * drawProgress, spriteY });
            sf::Color maleColor = maleSprite.getColor();
            maleColor.a = static_cast<std::uint8_t>(std::max(0.f, 1.f - drawProgress) * 255.f);
            maleSprite.setColor(maleColor);
        }
        else {
            maleSprite.setPosition({ maleStartX + (centerX - maleStartX) * drawProgress, spriteY });
            sf::Color femaleColor = femaleSprite.getColor();
            femaleColor.a = static_cast<std::uint8_t>(std::max(0.f, 1.f - drawProgress) * 255.f);
            femaleSprite.setColor(femaleColor);
        }
    }
    else if (phase == Game::GenderSelectionAnimation::Phase::Reverting) {
        float drawProgress = animationProgress;
        if (animation.selection == 0) {
            femaleSprite.setPosition({ centerX + (femaleStartX - centerX) * drawProgress, spriteY });
            sf::Color maleColor = maleSprite.getColor();
            maleColor.a = static_cast<std::uint8_t>(drawProgress * 255.f);
            maleSprite.setColor(maleColor);
        }
        else {
            maleSprite.setPosition({ centerX + (maleStartX - centerX) * drawProgress, spriteY });
            sf::Color femaleColor = femaleSprite.getColor();
            femaleColor.a = static_cast<std::uint8_t>(drawProgress * 255.f);
            femaleSprite.setColor(femaleColor);
        }

        if (animationProgress >= 1.f)
            animation.phase = Game::GenderSelectionAnimation::Phase::Idle;
    }
    else if (phase == Game::GenderSelectionAnimation::Phase::FadingOut) {
        sf::Color selectedColor = sf::Color::White;
        selectedColor.a = static_cast<std::uint8_t>(std::max(0.f, 1.f - animationProgress) * 255.f);
        if (animation.selection == 0) {
            femaleSprite.setPosition({ centerX, spriteY });
            femaleSprite.setColor(selectedColor);
            sf::Color maleColor = maleSprite.getColor();
            maleColor.a = 0;
            maleSprite.setColor(maleColor);
        }
        else {
            maleSprite.setPosition({ centerX, spriteY });
            maleSprite.setColor(selectedColor);
            sf::Color femaleColor = femaleSprite.getColor();
            femaleColor.a = 0;
            femaleSprite.setColor(femaleColor);
        }

        if (animationProgress >= 1.f)
            advanceAfterDraw = true;
    }

    sf::FloatRect femaleBounds = femaleSprite.getGlobalBounds();
    sf::FloatRect maleBounds = maleSprite.getGlobalBounds();

    auto expandBounds = [&](sf::FloatRect bounds) {
        constexpr float kPadding = 12.f;
        bounds.position.x -= kPadding;
        bounds.position.y -= kPadding;
        bounds.size.x += kPadding * 2.f;
        bounds.size.y += kPadding * 2.f + kLabelSpacing;
        return bounds;
    };

    game.genderSelectionBounds[0] = expandBounds(femaleBounds);
    game.genderSelectionBounds[1] = expandBounds(maleBounds);

    float femaleTop = femaleBounds.position.y;
    float maleTop = maleBounds.position.y;
    float hintTop = std::min(femaleTop, maleTop);
    sf::Text hintText{ game.resources.uiFont, "Choose Appearance", 24 };
    hintText.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::PromptGray, uiAlphaFactor));
    auto hintBounds = hintText.getLocalBounds();
    hintText.setOrigin({ hintBounds.position.x + hintBounds.size.x * 0.5f, hintBounds.position.y + hintBounds.size.y * 0.5f });
    hintText.setPosition({
        static_cast<float>(windowSize.x) * 0.5f,
        hintTop - kHintSpacing
    });

    float baseLeft = std::min(femaleBaseBounds.position.x, maleBaseBounds.position.x);
    float baseRight = std::max(
        femaleBaseBounds.position.x + femaleBaseBounds.size.x,
        maleBaseBounds.position.x + maleBaseBounds.size.x
    );
    float halfWidth = (baseRight - baseLeft) * 0.5f;
    float baseCenter = (baseLeft + baseRight) * 0.5f;
    float hintLeft = hintText.getPosition().x - (hintBounds.size.x * 0.5f);
    float hintRight = hintText.getPosition().x + (hintBounds.size.x * 0.5f);
    float leftLimit = hintLeft - 10.f;
    float rightLimit = hintRight + 10.f;
    float maxShrink = std::min({
        std::max(0.f, leftLimit - (baseCenter - halfWidth)),
        std::max(0.f, (baseCenter + halfWidth) - rightLimit),
        halfWidth
    });
    float shrinkAmount = 0.f;
    if (phase == Game::GenderSelectionAnimation::Phase::Approaching)
        shrinkAmount = maxShrink * animationProgress;
    else if (phase == Game::GenderSelectionAnimation::Phase::Reverting)
        shrinkAmount = maxShrink * (1.f - animationProgress);
    else if (phase == Game::GenderSelectionAnimation::Phase::FadingOut)
        shrinkAmount = maxShrink;
    float tintedLeft = baseCenter - (halfWidth - shrinkAmount);
    float tintedRight = baseCenter + (halfWidth - shrinkAmount);
    float tintedTop = std::min(hintText.getPosition().y - hintBounds.size.y * 0.5f, std::min(femaleBounds.position.y, maleBounds.position.y)) - 24.f;
    float labelBottom = std::max(
        femaleBounds.position.y + femaleBounds.size.y,
        maleBounds.position.y + maleBounds.size.y
    ) + kLabelSpacing * 0.5f + 20.f;
    float tintedBottom = labelBottom + 40.f;
    constexpr float kSelectionPad = 28.f;
    sf::RectangleShape selectionBackdrop;
    selectionBackdrop.setPosition({ tintedLeft - kSelectionPad, tintedTop });
    selectionBackdrop.setSize({
        (tintedRight - tintedLeft) + kSelectionPad * 2.f,
        (tintedBottom - tintedTop) + kSelectionPad * 0.5f
    });
    selectionBackdrop.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::DialogBackdrop, uiAlphaFactor));
    target.draw(selectionBackdrop);

    target.draw(femaleSprite);
    target.draw(maleSprite);
    target.draw(hintText);

    if (advanceAfterDraw) {
        animation.phase = Game::GenderSelectionAnimation::Phase::Idle;
        int selection = animation.selection;
        animation.selection = -1;
        game.playerGender = (selection == 0)
            ? Game::DragonbornGender::Female
            : Game::DragonbornGender::Male;
        stop(game);
        advanceDialogueLine(game);
        return;
    }
}

} // namespace ui::genderSelection
