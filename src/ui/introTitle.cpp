#include "introTitle.hpp"
#include "core/game.hpp"
#include <algorithm>

bool introTitleDropComplete(const Game& game) {
    if (!game.titleDropStarted)
        return false;

    constexpr float firstDropDuration = 1.0f;
    constexpr float secondDropDelay = 0.15f;
    constexpr float secondDropDuration = 1.0f;

    float totalDuration = firstDropDuration + secondDropDelay + secondDropDuration;
    return game.titleDropClock.getElapsedTime().asSeconds() >= totalDuration;
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
    }

    if (!game.uiFadeInActive) {
        if (game.introTitleHidden) {
            game.uiFadeInActive = true;
            game.uiFadeClock.restart();
            game.startGonadDialoguePending = true;
        }
        else {
            game.uiFadeInQueued = true;
        }
    }
}

void drawIntroTitle(Game& game, sf::RenderTarget& target) {
    if (game.introTitleHidden)
        return;

    game.startTitleScreenMusic();

    if (!game.titleDropStarted
        && game.state == GameState::IntroTitle
        && (game.backgroundFadeInActive || game.backgroundVisible)) {
        game.titleDropStarted = true;
        game.titleDropComplete = false;
        game.titleDropClock.restart();
    }

    float globalFade = 1.f;
    if(game.introTitleFadeOutActive) {
        float fadeProgress = std::min<float>(1.f, game.introTitleFadeClock.getElapsedTime().asSeconds() / game.introTitleFadeOutDuration);
        globalFade = 1.f - fadeProgress;

        if (fadeProgress >= 1.f) {
            game.introTitleFadeOutActive = false;
            game.introTitleHidden = true;

            if (game.uiFadeInQueued && !game.uiFadeInActive) {
                game.uiFadeInQueued = false;
                game.uiFadeInActive = true;
                game.uiFadeClock.restart();
                game.startGonadDialoguePending = true;
            }
            return;
        }
    }
    bool backgroundActive = game.backgroundFadeInActive || game.backgroundVisible;
    if (backgroundActive && game.background) {  // <- background existiert?
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

    bool dropFinished = game.titleDropComplete;
    bool introPromptJustActivated = false;

    if (game.titleDropStarted) {
        float elapsed = game.titleDropClock.getElapsedTime().asSeconds();
        constexpr float firstDropDuration = 1.0f;
        constexpr float secondDropDelay = 0.15f;
        constexpr float secondDropDuration = 1.0f;
        constexpr float totalDropDuration = firstDropDuration + secondDropDelay + secondDropDuration;

        auto windowSize = target.getSize();
        float centerX = static_cast<float>(windowSize.x) * 0.5f;
        float firstTargetY = static_cast<float>(windowSize.y) * 0.45f;
        float secondTargetY = firstTargetY + 110.f;
        float startY = -200.f;

        auto easeOutCubic = [](float t) {
            float inv = 1.f - t;
            return 1.f - inv * inv * inv;
        };

        auto computeY = [&](float dropElapsed, float duration, float targetY) {
            if (dropElapsed <= 0.f)
                return startY;

            float t = std::clamp(dropElapsed / duration, 0.f, 1.f);
            float eased = easeOutCubic(t);
            return startY + (targetY - startY) * eased;
        };

        auto drawTitle = [&](const std::string& text, unsigned int size, sf::Vector2f position) {
            sf::Text backLayer{ game.resources.titleFontExtrude, text, size };
            sf::Text frontLayer{ game.resources.titleFont,       text, size };

            frontLayer.setFillColor(ColorHelper::Palette::TitleAccent);
            backLayer.setFillColor(ColorHelper::Palette::TitleBack);

            auto centerOrigin = [](sf::Text& t) {
                auto b = t.getLocalBounds();
                t.setOrigin({
                    b.position.x + b.size.x * 0.5f,
                    b.position.y + b.size.y * 0.5f
                });
            };

            centerOrigin(frontLayer);
            centerOrigin(backLayer);

            float offsetY = -200.f;
            sf::Vector2f finalPos = { position.x, position.y + offsetY };

            frontLayer.setPosition(finalPos);
            backLayer.setPosition(finalPos);

            auto applyFade = [&](sf::Text& t) {
                sf::Color c = t.getFillColor();
                c.a = static_cast<std::uint8_t>(static_cast<float>(c.a) * globalFade);
                t.setFillColor(c);
            };

            applyFade(backLayer);
            applyFade(frontLayer);

            target.draw(backLayer);
            target.draw(frontLayer);
        };

        float lineSpacing = -40.f;

        float firstY = computeY(elapsed, firstDropDuration, firstTargetY);
        drawTitle("Glandular Chronicles", 100, { centerX, firstY });

        float secondElapsed = elapsed - firstDropDuration - secondDropDelay;
        if (secondElapsed > 0.f) {
            float secondY = computeY(secondElapsed, secondDropDuration, secondTargetY);
            drawTitle("The Dragonborn", 60, { centerX, secondY + lineSpacing });

            if (secondElapsed >= secondDropDuration && !game.titleDropComplete) {
                game.titleDropComplete = true;
                introPromptJustActivated = true;
            }
        }

        dropFinished = elapsed >= totalDropDuration;
    }

    if (introPromptJustActivated) {
        game.introPromptVisible = false;
        game.introPromptFade = 0.f;
        game.introPromptBlinkActive = true;
        game.introPromptInputEnabled = false;
        game.introPromptFadingIn = true;
        game.introPromptBlinkClock.restart();
    }

    if (game.titleDropComplete) {
        auto windowSize = target.getSize();
        float centerX = static_cast<float>(windowSize.x) * 0.5f;
        // Center the intro prompt vertically in the window (move it up from 82% to 50%)
        float centerY = static_cast<float>(windowSize.y) * 0.5f;

        if (game.introPromptBlinkActive) {
            float elapsed = game.introPromptBlinkClock.getElapsedTime().asSeconds();
            float progress = std::clamp(elapsed / game.introPromptBlinkInterval, 0.f, 1.f);

            if (game.introPromptFadingIn) {
                game.introPromptFade = progress;

                if (progress >= 1.f) {
                    game.introPromptFadingIn = false;
                    game.introPromptVisible = true;
                    game.introPromptInputEnabled = true;
                    game.introPromptBlinkClock.restart();
                }
            }
            else {
                game.introPromptFade = 1.f - progress;

                if (progress >= 1.f) {
                    game.introPromptFadingIn = true;
                    game.introPromptVisible = false;
                    game.introPromptBlinkClock.restart();
                }
            }
        }

        if (game.introPromptVisible || game.introPromptBlinkActive) {
            const std::string promptText = "Press Enter to Continue.";
            sf::Text prompt{ game.resources.introFont, promptText, 28 };
            prompt.setFillColor(ColorHelper::Palette::SoftYellow);

            auto centerText = [](sf::Text& text, float x, float y) {
                auto bounds = text.getLocalBounds();
                text.setOrigin({ bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f});
                text.setPosition({ x, y });
            };

            auto applyPromptFade = [&](sf::Text& text) {
                sf::Color color = text.getFillColor();
                float fade = game.introPromptBlinkActive ? game.introPromptFade : 1.f;
                color.a = static_cast<std::uint8_t>(static_cast<float>(color.a) * fade * globalFade);
                text.setFillColor(color);
            };

            centerText(prompt, centerX, centerY);
            applyPromptFade(prompt);
            target.draw(prompt);
        }
    }
}
