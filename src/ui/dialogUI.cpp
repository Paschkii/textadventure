// === C++ Libraries ===
#include <algorithm>  // Uses std::clamp/max when adjusting alpha values and selection handling.
#include <array>      // Holds fixed sets of dragon line triggers and audio indexes.
#include <cmath>      // Computes sine for HP blink timing.
#include <cstdint>    // Provides uint8_t for opacity manipulation on sprites/text.
#include <limits>     // Supplies std::numeric_limits for resetting showcase timers.
#include <optional>   // Manages optional dragon indices/state used by the showcase logic.
// === Header Files ===
#include "dialogUI.hpp"             // Declares drawDialogueUI/related APIs implemented below.
#include "dialogDrawElements.hpp"   // Renders the dialogue boxes, name labels, and text draws.
#include "uiEffects.hpp"             // Computes glow/flicker colors reused by multiple UI elements.
#include "confirmationUI.hpp"       // Displays the confirmation prompt triggered from dialogues.
#include "ui/genderSelectionUI.hpp" // Handles the dragonborn selection panel overlay.
#include "uiVisibility.hpp"         // Computes fade/visibility rules shared across UI modules.
#include "story/dialogInput.hpp"    // Handles Enter logic and quiz transitions used in dialogue flow.
#include "story/storyIntro.hpp"     // Drives the intro, dragon, and quiz dialogues referenced here.
#include "story/textStyles.hpp"     // Provides speaker styles/colors for names and portraits.
#include "helper/colorHelper.hpp"   // Applies palette colors when drawing names/dragon labels.

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

namespace {
    void drawPlayerStatus(Game& game, sf::RenderTarget& target, float uiAlphaFactor) {
        if (game.playerStatusBox.getSize().x <= 0.f || game.playerStatusBox.getSize().y <= 0.f)
            return;

        sf::RectangleShape statusBox = game.playerStatusBox;
        statusBox.setFillColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, uiAlphaFactor));
        statusBox.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::FrameGoldLight, uiAlphaFactor));
        statusBox.setOutlineThickness(game.playerStatusBox.getOutlineThickness());
        target.draw(statusBox);

        constexpr float kBarHeight = 16.f;
        constexpr float kPadding = 8.f;
        constexpr float kLabelColumnWidth = 32.f;
        constexpr float kBarSpacing = 5.f;
        constexpr unsigned int kLabelTextSize = 16;

        float barX = statusBox.getPosition().x + kPadding + kLabelColumnWidth;
        float barWidth = std::max(0.f, statusBox.getSize().x - (kPadding * 2.f) - kLabelColumnWidth);
        if (barWidth <= 0.f)
            return;

        float hpBarY = statusBox.getPosition().y + kPadding;
        float xpBarY = hpBarY + kBarHeight + kBarSpacing;

        sf::Text hpLabel{ game.resources.uiFont, "HP", kLabelTextSize };
        hpLabel.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Health, uiAlphaFactor));
        auto hpLabelBounds = hpLabel.getLocalBounds();
        hpLabel.setOrigin({
            hpLabelBounds.position.x,
            hpLabelBounds.position.y + (hpLabelBounds.size.y * 0.5f)
        });
        hpLabel.setPosition({ statusBox.getPosition().x + kPadding, hpBarY + (kBarHeight * 0.5f) });
        target.draw(hpLabel);

        float hpRatio = (game.playerHpMax > 0.f)
            ? std::clamp(game.playerHp / game.playerHpMax, 0.f, 1.f)
            : 0.f;
        float blinkAlpha = 1.f;
        sf::Color hpColor = ColorHelper::Palette::Health;
        if (hpRatio < 0.2f) {
            float blinkPhase = (std::sin(game.uiGlowClock.getElapsedTime().asSeconds() * 8.f) + 1.f) * 0.5f;
            blinkAlpha = 0.45f + (0.45f * blinkPhase);
            hpColor = ColorHelper::lighten(hpColor, blinkPhase * 0.35f);
        }
        sf::RectangleShape hpFill({ barWidth * hpRatio, kBarHeight });
        hpFill.setPosition({ barX, hpBarY });
        hpFill.setFillColor(ColorHelper::applyAlphaFactor(hpColor, uiAlphaFactor * blinkAlpha));
        target.draw(hpFill);

        sf::RectangleShape hpBorder({ barWidth, kBarHeight });
        hpBorder.setPosition({ barX, hpBarY });
        hpBorder.setFillColor(sf::Color::Transparent);
        hpBorder.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Dim, uiAlphaFactor));
        hpBorder.setOutlineThickness(2.f);
        target.draw(hpBorder);

        sf::Text xpLabel{ game.resources.uiFont, "XP", kLabelTextSize };
        xpLabel.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::DarkPurple, uiAlphaFactor));
        auto xpLabelBounds = xpLabel.getLocalBounds();
        xpLabel.setOrigin({
            xpLabelBounds.position.x,
            xpLabelBounds.position.y + (xpLabelBounds.size.y * 0.5f)
        });
        xpLabel.setPosition({ statusBox.getPosition().x + kPadding, xpBarY + (kBarHeight * 0.5f) });
        target.draw(xpLabel);

        float xpRatio = (game.playerXpMax > 0.f)
            ? std::clamp(game.playerXp / game.playerXpMax, 0.f, 1.f)
            : 0.f;
        sf::RectangleShape xpFill({ barWidth * xpRatio, kBarHeight });
        xpFill.setPosition({ barX, xpBarY });
        xpFill.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::DarkPurple, uiAlphaFactor));
        target.draw(xpFill);

        sf::RectangleShape xpBorder({ barWidth, kBarHeight });
        xpBorder.setPosition({ barX, xpBarY });
        xpBorder.setFillColor(sf::Color::Transparent);
        xpBorder.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Dim, uiAlphaFactor));
        xpBorder.setOutlineThickness(2.f);
        target.draw(xpBorder);
    }
}

void drawLocationBox(Game& game, sf::RenderTarget& target, float uiAlphaFactor, const sf::Color& glowColor) {
    if (!game.currentLocation)
        return;

    constexpr unsigned int kLocationTextSize = 32;
    constexpr float kLocationTopInset = 12.f;
    constexpr float kDividerGap = 20.f;
    constexpr float kDividerSideInset = 6.f;

    auto boxPos = game.locationBox.getPosition();
    auto boxSize = game.locationBox.getSize();

    sf::Text locationName{ game.resources.uiFont, game.currentLocation->name, kLocationTextSize };
    locationName.setFillColor(ColorHelper::applyAlphaFactor(game.currentLocation->color, uiAlphaFactor));
    locationName.setOutlineColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, uiAlphaFactor));
    locationName.setOutlineThickness(1.f);

    auto localBounds = locationName.getLocalBounds();
    locationName.setOrigin({
        localBounds.position.x + (localBounds.size.x * 0.5f),
        localBounds.position.y + (localBounds.size.y * 0.5f)
    });
    float textY = boxPos.y + kLocationTopInset + (localBounds.size.y * 0.5f);
    locationName.setPosition({ boxPos.x + (boxSize.x / 2.f), textY });

    auto globalBounds = locationName.getGlobalBounds();

    sf::Sprite leftDivider{ game.resources.dividerLeft };
    sf::Sprite rightDivider{ game.resources.dividerRight };
    leftDivider.setColor(glowColor);
    rightDivider.setColor(glowColor);

    auto leftSize = game.resources.dividerLeft.getSize();
    auto rightSize = game.resources.dividerRight.getSize();
    float leftY = textY - (static_cast<float>(leftSize.y) * 0.5f);
    float rightY = textY - (static_cast<float>(rightSize.y) * 0.5f);

    float leftX = globalBounds.position.x - kDividerGap - static_cast<float>(leftSize.x);
    float rightX = globalBounds.position.x + globalBounds.size.x + kDividerGap;

    leftDivider.setPosition({ leftX, leftY });
    rightDivider.setPosition({ rightX, rightY });

    target.draw(leftDivider);
    target.draw(rightDivider);
    target.draw(locationName);
}

void drawDialogueUI(Game& game, sf::RenderTarget& target, bool skipConfirmation, float* outAlpha) {

    UiVisibility visibility = computeDialogueVisibility(game);

    if (game.state == GameState::IntroTitle && !game.uiFadeInActive)
        return;

    if (visibility.hidden)
        return;

    float uiAlphaFactor = visibility.alphaFactor;
    if (outAlpha)
        *outAlpha = uiAlphaFactor;
    float glowElapsedSeconds = game.uiGlowClock.getElapsedTime().asSeconds();
    sf::Color glowColor = uiEffects::computeGlowColor(
        ColorHelper::Palette::BlueLight,
        glowElapsedSeconds,
        uiAlphaFactor,
        140.f,
        30.f,
        { 25.f, 41.f }
    );

    // Hide location/item boxes during the intro title/state; they should only appear afterward.
    bool introTitleActive = game.state == GameState::IntroTitle || game.state == GameState::IntroScreen;
    bool inIntroTransition = game.pendingIntroDialogue || game.pendingGonadDialogue || game.uiFadeOutActive;
    bool showLocationBox = !introTitleActive && !inIntroTransition && game.currentDialogue && game.currentDialogue != &intro;

    dialogDraw::drawDialogueFrames(game, target, uiAlphaFactor, glowColor);

    if (showLocationBox) {
        drawPlayerStatus(game, target, uiAlphaFactor);
        drawLocationBox(game, target, uiAlphaFactor, glowColor);
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
    bool allowTyping = !game.teleportController.active()
        && !game.confirmationPrompt.active
        && hasDialogueLine
        && game.state != GameState::IntroTitle;
    if (allowTyping)
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

    if (line
        && line->triggersGenderSelection
        && !game.genderSelectionActive
        && game.visibleText.size() >= fullText.size()) {
        ui::genderSelection::start(game);
    }

    ui::genderSelection::draw(game, target, uiAlphaFactor);

    if (game.confirmationPrompt.active && !skipConfirmation) {
        drawConfirmationPrompt(game, target, uiAlphaFactor);
        return;
    }

    if (game.askingName)
        dialogDraw::drawNameInput(target, game, uiAlphaFactor);

    if (!game.genderSelectionActive)
        dialogDraw::drawReturnPrompt(target, game, uiAlphaFactor, isTyping);
}
