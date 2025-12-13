#include "dialogUI.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include "story/dialogInput.hpp"
#include "story/storyIntro.hpp"
#include "dialogDrawElements.hpp"
#include "story/textStyles.hpp"
#include "helper/colorHelper.hpp"
#include "confirmationUI.hpp"
#include "uiVisibility.hpp"

namespace {
    UiVisibility computeDialogueVisibility(Game& game) {
        UiElementMask visibilityMask =
            UiElement::TextBox
            | UiElement::NameBox
            | UiElement::IntroTitle;

        return computeUiVisibility(game, visibilityMask);
    }

    bool updateTypewriter(Game& game, const std::string& fullText, float delay) {
        bool isTyping = !game.askingName && game.charIndex < fullText.size();

    if (isTyping) {
            game.startTypingSound();
            if (game.typewriterClock.getElapsedTime().asSeconds() >= delay) {
                game.visibleText += fullText[game.charIndex];
                game.charIndex++;
                game.typewriterClock.restart();
            }
        }
        else {
            game.stopTypingSound();
        }

        return isTyping;
    }

    std::optional<std::size_t> dragonPortraitIndexForLocation(LocationId id) {
        switch (id) {
            case LocationId::Blyathyroid: return 0;
            case LocationId::Lacrimere: return 1;
            case LocationId::Cladrenal: return 2;
            case LocationId::Aerobronchi: return 3;
            default: return std::nullopt;
        }
    }

    struct DragonLineTrigger {
        std::size_t lineIndex;
        std::size_t dragonIndex;
    };

    constexpr std::array<DragonLineTrigger, 4> kDragonNameLines{{
        { 1, 0 },
        { 5, 1 },
        { 9, 2 },
        { 13, 3 }
    }};

    constexpr std::array<DragonLineTrigger, 4> kDragonFadeOutLines{{
        { 4, 0 },
        { 8, 1 },
        { 12, 2 },
        { 16, 3 }
    }};

    std::optional<std::size_t> dragonIndexForNameLine(std::size_t lineIndex) {
        for (const auto& entry : kDragonNameLines) {
            if (entry.lineIndex == lineIndex)
                return entry.dragonIndex;
        }
        return std::nullopt;
    }

    std::optional<std::size_t> dragonIndexForFadeOutLine(std::size_t lineIndex) {
        for (const auto& entry : kDragonFadeOutLines) {
            if (entry.lineIndex == lineIndex)
                return entry.dragonIndex;
        }
        return std::nullopt;
    }

    constexpr float kDragonFadeDuration = 1.f;
    constexpr float kDragonScaleMultiplier = 2.5f;
    constexpr unsigned int kDragonNameLabelSize = 32;
    constexpr float kDragonLabelSpacing = 28.f;

    void updateDragonShowcaseState(Game& game) {
        auto& state = game.dragonShowcase;

        if (game.currentDialogue != &dragon) {
            state.phase = Game::DragonShowcaseState::Phase::Hidden;
            state.activeDragon.reset();
            state.pendingDragon.reset();
            state.lastProcessedDialogueIndex = std::numeric_limits<std::size_t>::max();
            return;
        }

        std::size_t index = game.dialogueIndex;
        if (state.lastProcessedDialogueIndex == index)
            return;
        state.lastProcessedDialogueIndex = index;

        if (auto dragonId = dragonIndexForNameLine(index)) {
            if (state.phase == Game::DragonShowcaseState::Phase::FadingOut) {
                state.pendingDragon = dragonId;
            }
            else {
                state.pendingDragon.reset();
                state.activeDragon = dragonId;
                state.phase = Game::DragonShowcaseState::Phase::FadingIn;
                state.fadeClock.restart();
            }
            return;
        }

        if (auto dragonId = dragonIndexForFadeOutLine(index)) {
            state.activeDragon = dragonId;
            state.phase = Game::DragonShowcaseState::Phase::FadingOut;
            state.fadeClock.restart();
        }
    }

    void drawCenteredDragon(
        Game& game,
        sf::RenderTarget& target,
        Game::DragonPortrait& portrait,
        float combinedAlpha
    ) {
        if (combinedAlpha <= 0.f)
            return;

        auto view = target.getView();
        sf::Vector2f center = view.getCenter();

        auto prevScale = portrait.sprite.getScale();
        auto prevPosition = portrait.sprite.getPosition();
        auto prevColor = portrait.sprite.getColor();

        float displayScale = portrait.baseScale * kDragonScaleMultiplier;
        portrait.sprite.setScale({ displayScale, displayScale });
        portrait.sprite.setPosition(center);

        sf::Color spriteColor = prevColor;
        spriteColor.a = static_cast<std::uint8_t>(std::clamp(combinedAlpha, 0.f, 1.f) * 255.f);
        portrait.sprite.setColor(spriteColor);
        target.draw(portrait.sprite);

        auto spriteBounds = portrait.sprite.getGlobalBounds();

        sf::Text label{ game.resources.titleFont, portrait.displayName, kDragonNameLabelSize };
        auto speakerId = TextStyles::speakerFromName(portrait.displayName);
        auto speakerInfo = TextStyles::speakerStyle(speakerId);
        label.setFillColor(ColorHelper::applyAlphaFactor(speakerInfo.color, combinedAlpha));

        auto labelBounds = label.getLocalBounds();
        label.setOrigin({
            labelBounds.position.x + (labelBounds.size.x / 2.f),
            labelBounds.position.y + (labelBounds.size.y / 2.f)
        });
        label.setPosition({ center.x, center.y + (spriteBounds.size.y / 2.f) + kDragonLabelSpacing });
        target.draw(label);

        portrait.sprite.setScale(prevScale);
        portrait.sprite.setPosition(prevPosition);
        portrait.sprite.setColor(prevColor);
    }

    void drawDragonShowcase(
        Game& game,
        sf::RenderTarget& target,
        float uiAlphaFactor,
        std::optional<LocationId> highlightLocation = std::nullopt
    ) {
        if (game.dragonPortraits.empty())
            return;

        updateDragonShowcaseState(game);

        if (game.currentDialogue == &dragon) {
            auto& state = game.dragonShowcase;

            if (state.phase == Game::DragonShowcaseState::Phase::Hidden || !state.activeDragon)
                return;

            if (state.phase == Game::DragonShowcaseState::Phase::FadingOut) {
                float elapsed = state.fadeClock.getElapsedTime().asSeconds();
                if (elapsed >= kDragonFadeDuration) {
                    state.phase = Game::DragonShowcaseState::Phase::Hidden;
                    state.activeDragon.reset();
                    if (state.pendingDragon) {
                        state.activeDragon = state.pendingDragon;
                        state.pendingDragon.reset();
                        state.phase = Game::DragonShowcaseState::Phase::FadingIn;
                        state.fadeClock.restart();
                    }
                }
            }

            if (state.phase == Game::DragonShowcaseState::Phase::Hidden || !state.activeDragon)
                return;

            float elapsed = state.fadeClock.getElapsedTime().asSeconds();
            float alpha = 0.f;

            if (state.phase == Game::DragonShowcaseState::Phase::FadingIn) {
                if (elapsed >= kDragonFadeDuration) {
                    alpha = 1.f;
                    state.phase = Game::DragonShowcaseState::Phase::Visible;
                }
                else {
                    alpha = elapsed / kDragonFadeDuration;
                }
            }
            else if (state.phase == Game::DragonShowcaseState::Phase::Visible) {
                alpha = 1.f;
            }
            else if (state.phase == Game::DragonShowcaseState::Phase::FadingOut) {
                float progress = std::clamp(elapsed / kDragonFadeDuration, 0.f, 1.f);
                alpha = 1.f - progress;
            }

            float combinedAlpha = std::clamp(alpha * uiAlphaFactor, 0.f, 1.f);
            if (combinedAlpha <= 0.f)
                return;

            std::size_t dragonIndex = *state.activeDragon;
            if (dragonIndex >= game.dragonPortraits.size())
                return;

            drawCenteredDragon(game, target, game.dragonPortraits[dragonIndex], combinedAlpha);
            return;
        }

        if (highlightLocation) {
            if (auto highlightIndex = dragonPortraitIndexForLocation(*highlightLocation)) {
                std::size_t dragonIndex = *highlightIndex;
                if (dragonIndex < game.dragonPortraits.size()) {
                    drawCenteredDragon(game, target, game.dragonPortraits[dragonIndex], uiAlphaFactor);
                }
            }
        }
    }
}

void drawLocationBox(Game& game, sf::RenderTarget& target, float uiAlphaFactor) {
    if (!game.currentLocation)
        return;

    constexpr unsigned int kLocationTextSize = 32;
    constexpr float kLocationOutlineThickness = 1.f;

    sf::Text locationName{ game.resources.uiFont, game.currentLocation->name, kLocationTextSize };
    locationName.setFillColor(ColorHelper::applyAlphaFactor(game.currentLocation->color, uiAlphaFactor));
    locationName.setOutlineColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, uiAlphaFactor));
    locationName.setOutlineThickness(kLocationOutlineThickness);

    auto boxPos = game.locationBox.getPosition();
    auto boxSize = game.locationBox.getSize();
    sf::Vector2f boxCenter{ boxPos.x + (boxSize.x / 2.f), boxPos.y + (boxSize.y / 2.f) };

    auto textBounds = locationName.getLocalBounds();
    sf::Vector2f origin{
        textBounds.position.x + (textBounds.size.x / 2.f),
        textBounds.position.y + (textBounds.size.y / 2.f)
    };
    locationName.setOrigin(origin);
    locationName.setPosition(boxCenter);

    target.draw(locationName);
}

void drawDialogueUI(Game& game, sf::RenderTarget& target, bool skipConfirmation, float* outAlpha) {

    UiVisibility visibility = computeDialogueVisibility(game);

    if (visibility.hidden)
        return;

    float uiAlphaFactor = visibility.alphaFactor;
    if (outAlpha)
        *outAlpha = uiAlphaFactor;
    float glowElapsedSeconds = game.uiGlowClock.getElapsedTime().asSeconds();

    // Hide location/item boxes during the intro title/state; they should only appear afterward.
    bool introTitleActive = game.state == GameState::IntroTitle || game.state == GameState::IntroScreen;
    bool showLocationBox = !introTitleActive && game.currentDialogue != &intro;
    bool showItemBox = !introTitleActive && game.currentDialogue != &intro;

    dialogDraw::drawDialogueFrames(game, target, uiAlphaFactor, glowElapsedSeconds, showLocationBox, showItemBox);

    if (showLocationBox) {
        dialogDraw::drawBoxHeader(game, target, game.locationBox, "Location", uiAlphaFactor);
        drawLocationBox(game, target, uiAlphaFactor);
    }
    if (showItemBox) {
        dialogDraw::drawBoxHeader(game, target, game.itemBox, "Items", uiAlphaFactor);
        dialogDraw::drawItemIcons(game, target, uiAlphaFactor);
    }

    bool keepShowingLastFeedbackLine =
        game.state == GameState::Quiz
        && game.currentDialogue == &game.quiz.feedbackDialogue
        && !game.visibleText.empty();
    if (game.currentDialogue == &dragon || keepShowingLastFeedbackLine) {
        std::optional<LocationId> highlightLocation;
        if (keepShowingLastFeedbackLine)
            highlightLocation = game.quiz.targetLocation;
        drawDragonShowcase(game, target, uiAlphaFactor, highlightLocation);
    }

    bool hasDialogueLine = game.currentDialogue && game.dialogueIndex < game.currentDialogue->size();
    if (!hasDialogueLine && !game.confirmationPrompt.active && !keepShowingLastFeedbackLine) {
        game.lastSpeaker.reset();
        return;
    }

    const DialogueLine* line = nullptr;
    std::string fullText;
    if (hasDialogueLine) {
        line = &(*game.currentDialogue)[game.dialogueIndex];
        fullText = injectSpeakerNames(line->text, game);
    }
    else if (keepShowingLastFeedbackLine && game.currentDialogue && !game.currentDialogue->empty()) {
        line = &game.currentDialogue->back();
    }

    float delay = 0.02f;
    bool isTyping = false;
    if (!game.teleportActive && !game.confirmationPrompt.active && hasDialogueLine)
        isTyping = updateTypewriter(game, fullText, delay);

    if (line) {
        TextStyles::SpeakerStyle info = TextStyles::speakerStyle(line->speaker);
        bool sameSpeakerAsPrevious = game.lastSpeaker && *game.lastSpeaker == line->speaker;
        // Hide the speaker name until some text is visible (prevents leftover
        // speaker names from appearing immediately after the intro title).
        if (game.visibleText.empty() && !game.askingName && !sameSpeakerAsPrevious) {
            TextStyles::SpeakerStyle emptyInfo = info;
            emptyInfo.name.clear();
            dialogDraw::drawSpeakerName(target, game, emptyInfo, uiAlphaFactor);
        }
        else {
            dialogDraw::drawSpeakerName(target, game, info, uiAlphaFactor);
        }
        game.lastSpeaker = line->speaker;

        std::string textToDraw = game.visibleText;
        if (game.askingName) {
            auto newlinePos = textToDraw.find('\n');
            if (newlinePos != std::string::npos)
                textToDraw = textToDraw.substr(0, newlinePos + 1);
        }

        dialogDraw::drawDialogueText(target, game, textToDraw, uiAlphaFactor);
    }

    if (game.confirmationPrompt.active && !skipConfirmation) {
        drawConfirmationPrompt(game, target, uiAlphaFactor);
        return;
    }

    if (game.askingName)
        dialogDraw::drawNameInput(target, game, uiAlphaFactor);

    dialogDraw::drawReturnPrompt(target, game, uiAlphaFactor, isTyping);
}
