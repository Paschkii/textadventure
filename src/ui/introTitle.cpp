#include "introTitle.hpp"
#include "core/game.hpp"
#include <algorithm>

void drawIntroTitle(Game& game, sf::RenderTarget& target) {
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
        bgColor.a = static_cast<std::uint8_t>(255.f * fadeProgress);
        game.background->setColor(bgColor);

        target.draw(*game.background);
    }

    bool dropFinished = game.titleDropComplete;

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

            frontLayer.setFillColor(sf::Color(255, 186, 59));
            backLayer.setFillColor(sf::Color(92, 64, 35));

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

            if (secondElapsed >= secondDropDuration) {
                game.titleDropComplete = true;
            }
        }

        dropFinished = elapsed >= totalDropDuration;
    }

    if (dropFinished && !game.titleDropComplete) {
        game.titleDropComplete = true;
        game.introPromptVisible = true;
        game.introPromptBlinkClock.restart();
    }

    if (game.backgroundVisible && !game.titleDropStarted) {
        game.titleDropStarted = true;
        game.titleDropComplete = false;
        game.titleDropClock.restart();
    }

    if (game.titleDropComplete) {
        auto windowSize = target.getSize();
        float centerX = static_cast<float>(windowSize.x) * 0.5f;
        float centerY = static_cast<float>(windowSize.y) * 0.82f;

        if (game.introPromptBlinkClock.getElapsedTime().asSeconds() >= game.introPromptBlinkInterval) {
            game.introPromptVisible = !game.introPromptVisible;
            game.introPromptBlinkClock.restart();
        }

        if (game.introPromptVisible) {
            const std::string promptText = "Press Enter to Continue.";
            sf::Text prompt{ game.resources.introFont, promptText, 28 };
            prompt.setFillColor(ColorHelper::Palette::SoftYellow);

            auto centerText = [](sf::Text& text, float x, float y) {
                auto bounds = text.getLocalBounds();
                text.setOrigin({ bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f});
                text.setPosition({ x, y });
            };

            centerText(prompt, centerX, centerY);
            target.draw(prompt);
        }
    }
}