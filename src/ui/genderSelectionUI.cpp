#include "genderSelectionUI.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <string_view>

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
    constexpr std::array<std::string_view, 2> kGenderLabels = { "Left", "Right" };

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
        for (int idx = 0; idx < static_cast<int>(kGenderLabels.size()); ++idx) {
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
    game.genderSelectionHovered = 0;
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
    if (idx < 0 || idx >= static_cast<int>(kGenderLabels.size()))
        return;
    if (game.confirmationPrompt.active)
        return;

    game.genderSelectionHovered = idx;
    auto& animation = game.genderAnimation;
    animation.selection = idx;
    animation.phase = Game::GenderSelectionAnimation::Phase::Approaching;
    animation.clock.restart();
    animation.labelsHidden = false;

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
            case sf::Keyboard::Scan::L:
                if (game.genderSelectionHovered != 0) {
                    game.genderSelectionHovered = 0;
                    playButtonHoverSound(game);
                }
                choose(game, 0);
                return true;
            case sf::Keyboard::Scan::R:
                if (game.genderSelectionHovered != 1) {
                    game.genderSelectionHovered = 1;
                    playButtonHoverSound(game);
                }
                choose(game, 1);
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
    sf::Text hintText{ game.resources.uiFont, "Choose your Dragonborn", 18 };
    hintText.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::PromptGray, uiAlphaFactor));
    auto hintBounds = hintText.getLocalBounds();
    hintText.setOrigin({ hintBounds.position.x + hintBounds.size.x * 0.5f, hintBounds.position.y + hintBounds.size.y * 0.5f });
    hintText.setPosition({
        static_cast<float>(windowSize.x) * 0.5f,
        hintTop - kHintSpacing
    });

    target.draw(femaleSprite);
    target.draw(maleSprite);
    target.draw(hintText);

    auto drawLabel = [&](const sf::FloatRect& bounds, std::string_view label, bool hovered) {
        sf::Text labelText{ game.resources.uiFont, std::string(label), 24 };
        float alphaFactor = hovered ? uiAlphaFactor : std::max(0.f, uiAlphaFactor * 0.85f);
        labelText.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, alphaFactor));
        labelText.setStyle(sf::Text::Bold);
        auto textBounds = labelText.getLocalBounds();
        labelText.setOrigin({ textBounds.position.x + textBounds.size.x * 0.5f, textBounds.position.y + textBounds.size.y * 0.5f });
        float baseY = bounds.position.y + bounds.size.y - kLabelSpacing * 0.5f + 20.f;
        labelText.setPosition({ bounds.position.x + bounds.size.x * 0.5f, baseY });
        target.draw(labelText);

        std::size_t underlineIndex = 0;
        auto firstPos = labelText.findCharacterPos(static_cast<unsigned>(underlineIndex));
        auto nextPos = labelText.findCharacterPos(static_cast<unsigned>(std::min(label.size(), underlineIndex + 1)));
        float underlineStart = firstPos.x;
        float underlineEnd = nextPos.x;
        if (underlineEnd <= underlineStart)
            underlineEnd = underlineStart + textBounds.size.x * 0.1f;
        float underlineY = labelText.getPosition().y + textBounds.size.y * 0.5f + 4.f;
        sf::RectangleShape underline({ underlineEnd - underlineStart, 2.f });
        underline.setPosition({ underlineStart, underlineY });
        sf::Color underlineColor = ColorHelper::Palette::SoftYellow;
        underlineColor = ColorHelper::applyAlphaFactor(underlineColor, uiAlphaFactor);
        underline.setFillColor(underlineColor);
        target.draw(underline);
    };

    if (!game.genderAnimation.labelsHidden) {
        drawLabel(femaleBounds, kGenderLabels[0], game.genderSelectionHovered == 0);
        drawLabel(maleBounds, kGenderLabels[1], game.genderSelectionHovered == 1);
    }

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
