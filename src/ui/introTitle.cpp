// === C++ Libraries ===
#include <array>      // Stores the intro menu labels and bounds.
#include <algorithm>  // Uses std::clamp when computing fade progress and pointer targets.

// === SFML Libraries ===
#include <SFML/Graphics/Text.hpp>   // Renders the option labels and outlines.
#include <SFML/Window/Keyboard.hpp> // Reads keyboard scancodes used in navigation.
#include <SFML/Window/Mouse.hpp>    // Reads mouse position when hovering intro options.

// === Header Files ===
#include "introTitle.hpp"
#include "core/game.hpp"
#include "helper/colorHelper.hpp"
#include "ui/confirmationUI.hpp"
#include "ui/battleUI.hpp"

namespace {
    constexpr std::array<const char*, 2> kIntroOptionLabels = {
        "Start Game",
        "Quit"
    };

    constexpr int kIntroOptionCount = static_cast<int>(kIntroOptionLabels.size());

    enum class IntroOption {
        Start,
        Quit,
    };

    constexpr float kOptionTextSize = 32.f;
    constexpr float kOptionSpacing = 46.f;

    bool introMenuReady(const Game& game) {
        return game.introTitleRevealComplete && game.introTitleOptionsFadeProgress >= 1.f;
    }

    void clampHovered(Game& game) {
        if (game.introTitleHoveredOption < 0)
            game.introTitleHoveredOption = 0;
        if (game.introTitleHoveredOption >= kIntroOptionCount)
            game.introTitleHoveredOption = kIntroOptionCount - 1;
    }

    void playIntroTitleHoverSound(Game& game) {
        if (!game.introTitleHoverSound)
            return;
        game.introTitleHoverSound->stop();
        game.introTitleHoverSound->play();
    }

    void executeIntroOption(Game& game, IntroOption option) {
        switch (option) {
            case IntroOption::Start:
                triggerIntroTitleExit(game);
                break;
            case IntroOption::Quit:
                showConfirmationPrompt(game, "Quit the game?", [](Game& g) {
                    g.window.close();
                }, [](Game&) {});
                break;
        }
    }

    int optionIndexAt(const Game& game, const sf::Vector2f& point) {
        for (int idx = 0; idx < kIntroOptionCount; ++idx) {
            if (game.introTitleOptionBounds[idx].contains(point))
                return idx;
        }
        return -1;
    }

    void updateHoveredFromMouse(Game& game, const sf::Vector2f& point) {
        int found = optionIndexAt(game, point);
        if (found < 0) {
            game.introTitleHoveredOption = -1;
            return;
        }
        if (found == game.introTitleHoveredOption)
            return;
        game.introTitleHoveredOption = found;
        playIntroTitleHoverSound(game);
    }
}

void triggerIntroTitleExit(Game& game) {
    if(!game.introTitleFadeOutActive && !game.introTitleHidden) {
        if (!game.startGameSoundPlayed && game.startGameSound) {
            game.startGameSound->stop();
            game.startGameSound->play();
            game.startGameSoundPlayed = true;
        }
        game.fadeOutTitleScreenMusic(game.introTitleFadeOutDuration + 0.5f);
        game.introTitleFadeOutActive = true;
        game.introTitleFadeClock.restart();
        game.queuedBackgroundTexture = &game.resources.backgroundToryTailor;
    }

    if (!game.uiFadeInActive) {
        if (game.introTitleHidden) {
            game.uiFadeInActive = true;
            game.uiFadeClock.restart();
            game.pendingIntroDialogue = true;
        }
        else {
            game.uiFadeInQueued = true;
        }
    }
}

bool handleIntroTitleEvent(Game& game, const sf::Event& event) {
    if (!introMenuReady(game))
        return false;

    if (auto move = event.getIf<sf::Event::MouseMoved>()) {
        auto mousePos = game.window.mapPixelToCoords(move->position);
        updateHoveredFromMouse(game, mousePos);
        return false;
    }

    if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button == sf::Mouse::Button::Left) {
            auto mousePos = game.window.mapPixelToCoords(button->position);
            int clicked = optionIndexAt(game, mousePos);
            if (clicked >= 0) {
                executeIntroOption(game, static_cast<IntroOption>(clicked));
                return true;
            }
        }
    }

    if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        switch (key->scancode) {
            case sf::Keyboard::Scan::Up: {
                int previous = game.introTitleHoveredOption;
                if (game.introTitleHoveredOption <= 0)
                    game.introTitleHoveredOption = kIntroOptionCount - 1;
                else
                    --game.introTitleHoveredOption;
                clampHovered(game);
                if (game.introTitleHoveredOption >= 0 && game.introTitleHoveredOption != previous)
                    playIntroTitleHoverSound(game);
                return true;
            }
            case sf::Keyboard::Scan::Down: {
                int previous = game.introTitleHoveredOption;
                if (game.introTitleHoveredOption < 0)
                    game.introTitleHoveredOption = 0;
                else if (game.introTitleHoveredOption >= kIntroOptionCount - 1)
                    game.introTitleHoveredOption = 0;
                else
                    ++game.introTitleHoveredOption;
                clampHovered(game);
                if (game.introTitleHoveredOption >= 0 && game.introTitleHoveredOption != previous)
                    playIntroTitleHoverSound(game);
                return true;
            }
            case sf::Keyboard::Scan::Enter:
                clampHovered(game);
                executeIntroOption(game, static_cast<IntroOption>(game.introTitleHoveredOption));
                return true;
            default:
                break;
        }
    }
    return false;
}

void drawIntroTitle(Game& game, sf::RenderTarget& target) {
    if (game.introTitleHidden)
        return;

    game.startTitleScreenMusic();

    bool backgroundActive = game.backgroundFadeInActive || game.backgroundVisible;
    if (!game.introTitleRevealStarted
        && game.state == GameState::IntroTitle
        && backgroundActive) {
        game.introTitleRevealStarted = true;
        game.introTitleRevealComplete = false;
        game.introTitleRevealClock.restart();
        game.introTitleOptionsFadeTriggered = false;
        game.introTitleOptionsFadeProgress = 0.f;
        game.introTitleHoveredOption = -1;
    }

    float globalFade = 1.f;
    if(game.introTitleFadeOutActive) {
        float fadeProgress = std::min<float>(1.f, game.introTitleFadeClock.getElapsedTime().asSeconds() / game.introTitleFadeOutDuration);
        globalFade = 1.f - fadeProgress;

        if (fadeProgress >= 1.f) {
            game.introTitleFadeOutActive = false;
            if (game.queuedBackgroundTexture) {
                game.setBackgroundTexture(*game.queuedBackgroundTexture);
                game.queuedBackgroundTexture = nullptr;
            }
            game.introTitleHidden = true;

            if (game.uiFadeInQueued && !game.uiFadeInActive) {
                game.uiFadeInQueued = false;
                game.uiFadeInActive = true;
                game.uiFadeClock.restart();
                game.pendingIntroDialogue = true;
            }
            return;
        }
    }

    if (backgroundActive && game.background) {
        float fadeProgress = 1.f;
        if (game.backgroundFadeInActive) {
            float t = game.backgroundFadeClock.getElapsedTime().asSeconds() / game.introFadeDuration;
            fadeProgress = std::min<float>(1.f, t);

            if (fadeProgress >= 1.f) {
                game.backgroundFadeInActive = false;
                game.backgroundVisible = true;
            }
        }

        auto texSize = game.resources.introBackground.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            float scaleX = static_cast<float>(target.getSize().x) / static_cast<float>(texSize.x);
            float scaleY = static_cast<float>(target.getSize().y) / static_cast<float>(texSize.y);

            game.background->setScale(sf::Vector2f{ scaleX, scaleY });
            game.background->setPosition({ 0.f, 0.f });
        }

        sf::Color bgColor = game.background->getColor();
        bgColor.a = static_cast<std::uint8_t>(255.f * fadeProgress * globalFade);
        game.background->setColor(bgColor);

        target.draw(*game.background);
    }

    float revealProgress = game.introTitleRevealStarted
        ? std::clamp(game.introTitleRevealClock.getElapsedTime().asSeconds() / game.introTitleRevealDuration, 0.f, 1.f)
        : 0.f;
    if (revealProgress >= 1.f)
        game.introTitleRevealComplete = true;

    if (game.introTitleRevealComplete
        && !game.introTitleOptionsFadeTriggered
        && game.introTitleOptionsFadeProgress < 1.f) {
        game.introTitleOptionsFadeTriggered = true;
        game.introTitleOptionsFadeActive = true;
        game.introTitleOptionsFadeClock.restart();
        game.introTitleHoveredOption = -1;
    }

    if (game.introTitleOptionsFadeActive) {
        float optionProgress = std::clamp(game.introTitleOptionsFadeClock.getElapsedTime().asSeconds() / game.introTitleOptionsFadeDuration, 0.f, 1.f);
        game.introTitleOptionsFadeProgress = optionProgress;
        if (optionProgress >= 1.f)
            game.introTitleOptionsFadeActive = false;
    }

    float titleAlphaFactor = revealProgress * globalFade;
    float optionsAlphaFactor = game.introTitleOptionsFadeProgress * globalFade;

    auto drawTitleLine = [&](const std::string& text, unsigned int size, float offsetY) {
        sf::Text back{ game.resources.titleFontExtrude, text, size };
        sf::Text front{ game.resources.titleFont, text, size };

        back.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::TitleBack, titleAlphaFactor));
        front.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::TitleAccent, titleAlphaFactor));

        auto centerOrigin = [](sf::Text& t) {
            auto bounds = t.getLocalBounds();
            t.setOrigin({
                bounds.position.x + bounds.size.x * 0.5f,
                bounds.position.y + bounds.size.y * 0.5f
            });
        };

        centerOrigin(back);
        centerOrigin(front);

        float centerX = static_cast<float>(target.getSize().x) * 0.5f;
        float centerY = offsetY;
        back.setPosition({ centerX, centerY });
        front.setPosition({ centerX, centerY });

        target.draw(back);
        target.draw(front);
    };

    auto windowSize = target.getSize();
    float firstLineY = static_cast<float>(windowSize.y) * 0.33f - 100.f;
    float secondLineY = firstLineY + 80.f;
    drawTitleLine("Glandular Chronicles", 100, firstLineY);
    drawTitleLine("The Dragonborn", 60, secondLineY);

    for (auto& bounds : game.introTitleOptionBounds)
        bounds = {};

    if (optionsAlphaFactor > 0.f) {
        float startY = static_cast<float>(windowSize.y) * 0.63f;
        sf::Color baseFill = ColorHelper::Palette::Amber;
        sf::Color baseOutline = ColorHelper::Palette::DarkBrown;
        sf::Color hoverFill = ColorHelper::Palette::SoftYellow;
        sf::Color hoverOutline = TextStyles::UI::PanelDark;

        constexpr float kHighlightWidthFactor = 0.25f;
        constexpr float kHighlightWidthMaxFactor = 0.325f;
        constexpr float kHighlightMinWidth = 180.f;
        constexpr float kHighlightHeightFactor = 1.2f;
        float highlightWidth = std::clamp(
            static_cast<float>(windowSize.x) * kHighlightWidthFactor,
            kHighlightMinWidth,
            static_cast<float>(windowSize.x) * kHighlightWidthMaxFactor
        );
        float highlightHeight = kOptionSpacing * kHighlightHeightFactor;
        float optionsStackHeight = static_cast<float>(kIntroOptionCount - 1) * kOptionSpacing;
        float backdropWidth = highlightWidth + 20.f;
        float backdropHeight = optionsStackHeight + kOptionTextSize + 60.f;
        float backdropCenterY = startY + (optionsStackHeight * 0.5f);
        float backdropLeft = static_cast<float>(windowSize.x) * 0.5f - (backdropWidth * 0.5f);
        float backdropTop = backdropCenterY - (backdropHeight * 0.5f);
        sf::Color backdropColor = ColorHelper::applyAlphaFactor({ 3, 3, 6, 100 }, optionsAlphaFactor);
        game.introOptionBackdrop.setSize({ backdropWidth, backdropHeight });
        game.introOptionBackdrop.setOrigin({ 0.f, 0.f });
        game.introOptionBackdrop.setPosition({ backdropLeft, backdropTop });
        game.introOptionBackdrop.setFillColor(backdropColor);
        target.draw(game.introOptionBackdrop);

        float outlinePadding = 6.f;
        sf::Vector2f size = game.introOptionBackdrop.getSize();
        sf::Vector2f position = game.introOptionBackdrop.getPosition();
        sf::Color borderColor = ColorHelper::applyAlphaFactor(sf::Color::Black, optionsAlphaFactor);
        game.uiFrame.draw(
            target,
            { position.x - outlinePadding, position.y - outlinePadding },
            { size.x + outlinePadding * 2.f, size.y + outlinePadding * 2.f },
            borderColor);

        for (int idx = 0; idx < kIntroOptionCount; ++idx) {
            sf::Text optionText{ game.resources.uiFont, kIntroOptionLabels[idx], static_cast<unsigned int>(kOptionTextSize) };
            bool hovered = idx == game.introTitleHoveredOption;
            sf::Color fillColor = hovered ? baseFill : baseFill;
            sf::Color outlineColor = hovered ? ColorHelper::Palette::BlueAlsoNearBlack : baseOutline;

            optionText.setFillColor(ColorHelper::applyAlphaFactor(fillColor, optionsAlphaFactor));
            optionText.setOutlineColor(ColorHelper::applyAlphaFactor(outlineColor, optionsAlphaFactor));
            optionText.setOutlineThickness(2.f);
            optionText.setStyle(sf::Text::Bold);

            auto bounds = optionText.getLocalBounds();
            optionText.setOrigin({
                bounds.position.x + bounds.size.x * 0.5f,
                bounds.position.y + bounds.size.y * 0.5f
            });

            float posY = startY + static_cast<float>(idx) * kOptionSpacing;
            optionText.setPosition({
                static_cast<float>(windowSize.x) * 0.5f,
                posY
            });

            if (hovered) {
                game.optionsBox.setSize({ highlightWidth, highlightHeight });
                game.optionsBox.setOrigin({ 0.f, 0.f });
                game.optionsBox.setPosition({
                    static_cast<float>(windowSize.x) * 0.5f - (highlightWidth * 0.5f),
                    posY - (highlightHeight * 0.5f)
                });
                sf::Color highlightColor = TextStyles::Palette::SoftYellow;
                game.uiFrame.draw(target, game.optionsBox, highlightColor);
            }

            target.draw(optionText);
            game.introTitleOptionBounds[idx] = optionText.getGlobalBounds();
        }
    }

    if (game.confirmationPrompt.active)
        drawConfirmationPrompt(game, target, 1.f);
}
