// === C++ Libraries ===
#include <algorithm>  // Uses std::clamp/max when adjusting alpha values and selection handling.
#include <array>      // Holds fixed sets of dragon line triggers and audio indexes.
#include <cmath>      // Computes sine for HP blink timing.
#include <cstdint>    // Provides uint8_t for opacity manipulation on sprites/text.
#include <limits>     // Supplies std::numeric_limits for resetting showcase timers.
#include <optional>   // Manages optional dragon indices/state used by the showcase logic.
#include <string>     // Used for crafting weapon names in the forged-weapon popup.
#include <vector>
// === Header Files ===
#include "dialogUI.hpp"             // Declares drawDialogueUI/related APIs implemented below.
#include "dialogDrawElements.hpp"   // Renders the dialogue boxes, name labels, and text draws.
#include "uiEffects.hpp"             // Computes glow/flicker colors reused by multiple UI elements.
#include "ui/brokenWeaponPreview.hpp"// Draws the temporary preview of broken weapons.
#include "confirmationUI.hpp"       // Displays the confirmation prompt triggered from dialogues.
#include "ui/genderSelectionUI.hpp" // Handles the dragonborn selection panel overlay.
#include "uiVisibility.hpp"         // Computes fade/visibility rules shared across UI modules.
#include "ui/menuUI.hpp"            // Draws and handles the new in-game menu button.
#include "ui/mapSelectionUI.hpp"    // Draws the map popups requested by the menu map tab.
#include "story/dialogInput.hpp"    // Handles Enter logic and quiz transitions used in dialogue flow.
#include "story/storyIntro.hpp"     // Drives the intro, dragon, and quiz dialogues referenced here.
#include "story/textStyles.hpp"     // Provides speaker styles/colors for names and portraits.
#include "helper/colorHelper.hpp"   // Applies palette colors when drawing names/dragon labels.
#include "rendering/textLayout.hpp"
#include "ui/popupStyle.hpp"

namespace {
    UiVisibility computeDialogueVisibility(Game& game) {
        UiElementMask visibilityMask =
            UiElement::TextBox
            | UiElement::NameBox
            | UiElement::IntroTitle;

        return computeUiVisibility(game, visibilityMask);
    }

    void drawSceneBackground(Game& game, sf::RenderTarget& target, float uiAlphaFactor) {
        if (!game.background || (!game.backgroundFadeInActive && !game.backgroundVisible))
            return;

        const sf::Texture& texture = game.background->getTexture();
        auto texSize = texture.getSize();
        auto targetSize = target.getSize();
        if (texSize.x == 0 || texSize.y == 0 || targetSize.x == 0 || targetSize.y == 0)
            return;

        float scaleX = static_cast<float>(targetSize.x) / static_cast<float>(texSize.x);
        float scaleY = static_cast<float>(targetSize.y) / static_cast<float>(texSize.y);
        game.background->setScale({ scaleX, scaleY });
        game.background->setPosition({ 0.f, 0.f });

        float fadeProgress = 1.f;
        if (game.backgroundFadeInActive) {
            float progress = game.backgroundFadeClock.getElapsedTime().asSeconds() / game.introFadeDuration;
            fadeProgress = std::min(1.f, progress);
            if (fadeProgress >= 1.f) {
                game.backgroundFadeInActive = false;
                game.backgroundVisible = true;
            }
        }

        sf::Color bgColor = game.background->getColor();
        bgColor.a = static_cast<std::uint8_t>(255.f * fadeProgress * uiAlphaFactor);
        game.background->setColor(bgColor);
        target.draw(*game.background);
    }

    void drawForgedWeaponPopup(Game& game, sf::RenderTarget& target, float uiAlphaFactor) {
        if (!game.forgedWeaponPopupActive)
            return;
        if (game.selectedWeaponIndex < 0 || static_cast<std::size_t>(game.selectedWeaponIndex) >= game.weaponOptions.size())
            return;

        sf::Vector2f viewSize = target.getView().getSize();
        float popupWidth = std::clamp(viewSize.x * 0.4f, 360.f, 640.f);
        float popupHeight = std::clamp(viewSize.y * 0.35f, 220.f, 420.f);
        float bottomY = game.textBox.getPosition().y - 12.f;
        float popupX = viewSize.x * 0.5f - (popupWidth * 0.5f);
        float popupY = bottomY - popupHeight;
        if (popupY < 20.f)
            popupY = 20.f;

        sf::FloatRect popupBounds{ { popupX, popupY }, { popupWidth, popupHeight } };
        ui::popup::drawPopupFrame(target, popupBounds, uiAlphaFactor);

        const auto& option = game.weaponOptions[game.selectedWeaponIndex];
        if (option.texture.getSize().x > 0 && option.texture.getSize().y > 0) {
            sf::Sprite sprite{ option.texture };
            float spriteAreaHeight = popupHeight * 0.55f;
            float spriteAreaWidth = popupWidth * 0.8f;
            auto textureSize = option.texture.getSize();
            float scaleX = spriteAreaWidth / static_cast<float>(textureSize.x);
            float scaleY = spriteAreaHeight / static_cast<float>(textureSize.y);
            float scale = std::min(scaleX, scaleY);
            sprite.setScale({ scale, scale });
            auto localBounds = sprite.getLocalBounds();
            sprite.setOrigin({
                localBounds.position.x + localBounds.size.x * 0.5f,
                localBounds.position.y + localBounds.size.y * 0.5f
            });
            sprite.setPosition({
                popupX + popupWidth * 0.5f,
                popupY + popupHeight * 0.45f
            });
            sf::Color spriteColor = sprite.getColor();
            spriteColor.a = static_cast<std::uint8_t>(255.f * uiAlphaFactor);
            sprite.setColor(spriteColor);
            target.draw(sprite);
        }

        std::string displayName = game.forgedWeaponName;
        if (displayName.empty() && game.selectedWeaponIndex >= 0) {
            displayName = game.weaponOptions[game.selectedWeaponIndex].displayName;
        }
        sf::Text nameText{ game.resources.titleFont, displayName, 32 };
        nameText.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, uiAlphaFactor));
        auto nameBounds = nameText.getLocalBounds();
        nameText.setOrigin({
            nameBounds.position.x + (nameBounds.size.x * 0.5f),
            nameBounds.position.y + (nameBounds.size.y * 0.5f)
        });
        nameText.setPosition({
            popupX + (popupWidth * 0.5f),
            popupY + popupHeight - 32.f
        });
        target.draw(nameText);
    }

void drawMapItemPopup(Game& game, sf::RenderTarget& target, float uiAlphaFactor) {
    if (!game.mapItemPopupActive)
        return;

        sf::Vector2f viewSize = target.getView().getSize();
        float popupWidth = std::clamp(viewSize.x * 0.33f, 320.f, 440.f);
        float popupHeight = std::clamp(viewSize.y * 0.26f, 200.f, 280.f);
        float bottomY = game.textBox.getPosition().y - 12.f;
        float popupX = viewSize.x * 0.5f - (popupWidth * 0.5f);
        float popupY = bottomY - popupHeight;
        if (popupY < 20.f)
            popupY = 20.f;

        sf::FloatRect popupBounds{ { popupX, popupY }, { popupWidth, popupHeight } };
        ui::popup::drawPopupFrame(target, popupBounds, uiAlphaFactor);

        const sf::Texture& mapTexture = game.resources.mapGlandular;
        if (mapTexture.getSize().x > 0 && mapTexture.getSize().y > 0) {
            sf::Sprite sprite{ mapTexture };
            float spriteAreaWidth = popupWidth * 0.78f;
            float spriteAreaHeight = popupHeight * 0.55f;
            auto texSize = mapTexture.getSize();
            float scale = std::min(spriteAreaWidth / static_cast<float>(texSize.x), spriteAreaHeight / static_cast<float>(texSize.y));
            sprite.setScale({ scale, scale });
            auto bounds = sprite.getLocalBounds();
            sprite.setOrigin({
                bounds.position.x + bounds.size.x * 0.5f,
                bounds.position.y + bounds.size.y * 0.5f
            });
            sprite.setPosition({ popupX + popupWidth * 0.5f, popupY + popupHeight * 0.45f });
            sf::Color spriteColor = sprite.getColor();
            spriteColor.a = static_cast<std::uint8_t>(255.f * uiAlphaFactor);
            sprite.setColor(spriteColor);
            target.draw(sprite);
        }

    sf::Text titleText{ game.resources.uiFont, "Map of Glandular", 28 };
    titleText.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, uiAlphaFactor));
    auto titleBounds = titleText.getLocalBounds();
    titleText.setOrigin({
        titleBounds.position.x + titleBounds.size.x * 0.5f,
        titleBounds.position.y + titleBounds.size.y * 0.5f
    });
    titleText.setPosition({ popupX + popupWidth * 0.5f, popupY + popupHeight - 32.f });
    target.draw(titleText);
}

namespace {
    constexpr float kQuestPopupEnterDuration = 0.45f;
    constexpr float kQuestPopupVisibleDuration = 5.f;
    constexpr float kQuestPopupExitDuration = 0.35f;
    constexpr float kQuestPopupLineSpacing = 2.5f;
    constexpr float kQuestPopupHorizontalPadding = 20.f;
    constexpr float kQuestPopupVerticalPadding = 12.f;

    void advanceQuestPopupState(Game& game) {
        auto& popup = game.questPopup;
        bool transitioned = false;
        do {
            transitioned = false;
            float elapsed = popup.clock.getElapsedTime().asSeconds();
            switch (popup.phase) {
                case Game::QuestPopupState::Phase::Entering:
                    if (elapsed >= kQuestPopupEnterDuration) {
                        popup.phase = Game::QuestPopupState::Phase::Visible;
                        popup.clock.restart();
                        transitioned = true;
                    }
                    break;
                case Game::QuestPopupState::Phase::Visible:
                    if (elapsed >= kQuestPopupVisibleDuration) {
                        popup.phase = Game::QuestPopupState::Phase::Exiting;
                        popup.clock.restart();
                        transitioned = true;
                    }
                    break;
                case Game::QuestPopupState::Phase::Exiting:
                    if (elapsed >= kQuestPopupExitDuration) {
                        popup.phase = Game::QuestPopupState::Phase::Idle;
                        popup.entry.reset();
                        transitioned = true;
                    }
                    break;
                default:
                    break;
            }
        } while (transitioned);
    }

    void drawQuestPopup(Game& game, sf::RenderTarget& target, float uiAlphaFactor) {
        advanceQuestPopupState(game);
        auto& popup = game.questPopup;
        if (!popup.entry || popup.phase == Game::QuestPopupState::Phase::Idle)
            return;

        float viewWidth = static_cast<float>(target.getSize().x);
        float viewHeight = static_cast<float>(target.getSize().y);
        std::string message = popup.message.empty()
            ? "Quest Update"
            : popup.message;
        std::string prefix;
        std::string tail = message;
        if (message.rfind("New Quest: ", 0) == 0) {
            prefix = "New Quest: ";
            tail = message.substr(prefix.size());
        }
        else if (message.rfind("Finished Quest: ", 0) == 0) {
            prefix = "Finished Quest: ";
            tail = message.substr(prefix.size());
        }

        sf::Text prefixMeasurement{ game.resources.uiFont, prefix, 26 };
        prefixMeasurement.setOutlineThickness(0.f);
        sf::Text tailMeasurement{ game.resources.uiFont, tail, 26 };
        tailMeasurement.setOutlineThickness(0.f);
        float prefixWidth = prefix.empty() ? 0.f : prefixMeasurement.getLocalBounds().size.x;
        float tailWidth = tail.empty() ? 0.f : tailMeasurement.getLocalBounds().size.x;
        float textWidth = prefixWidth + tailWidth;
        float minPopupWidth = 320.f;
        float maxPopupWidth = std::min(viewWidth - 24.f, 560.f);
        float minWidth = std::min(minPopupWidth, maxPopupWidth);
        float contentWidth = textWidth + (kQuestPopupHorizontalPadding * 2.f);
        float popupWidth = std::clamp(contentWidth, minWidth, maxPopupWidth);
        float popupHeight = std::clamp(viewHeight * 0.12f, 70.f, 120.f);
        float settleY = 20.f;
        float startY = -popupHeight - 12.f;

        float alpha = 1.f;
        float y = settleY;
        float elapsed = popup.clock.getElapsedTime().asSeconds();
        switch (popup.phase) {
            case Game::QuestPopupState::Phase::Entering: {
                float progress = std::clamp(elapsed / kQuestPopupEnterDuration, 0.f, 1.f);
                y = startY + (settleY - startY) * progress;
                alpha = progress;
                break;
            }
            case Game::QuestPopupState::Phase::Visible:
                y = settleY;
                alpha = 1.f;
                break;
            case Game::QuestPopupState::Phase::Exiting: {
                float progress = std::clamp(elapsed / kQuestPopupExitDuration, 0.f, 1.f);
                y = settleY + (startY - settleY) * progress;
                alpha = 1.f - progress;
                break;
            }
            default:
                return;
        }

        float x = (viewWidth - popupWidth) * 0.5f;
        sf::FloatRect bounds({ x, y }, { popupWidth, popupHeight });
        float compositeAlpha = uiAlphaFactor * alpha;
        ui::popup::drawPopupFrame(target, bounds, compositeAlpha);

        sf::Text prefixText{ game.resources.uiFont, prefix, 26 };
        prefixText.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::PurpleBlue, compositeAlpha));
        prefixText.setOutlineThickness(0.f);
        prefixText.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, compositeAlpha));
        sf::Text tailText{ game.resources.uiFont, tail, 26 };
        tailText.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, compositeAlpha));
        tailText.setOutlineThickness(0.f);
        tailText.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, compositeAlpha));

        float combinedWidth = prefixWidth + tailWidth;
        float startX = bounds.position.x + (bounds.size.x - combinedWidth) * 0.5f;
        float centerY = bounds.position.y + (bounds.size.y * 0.5f);

        if (!prefix.empty()) {
            auto prefixBounds = prefixText.getLocalBounds();
            prefixText.setOrigin({
                prefixBounds.position.x,
                prefixBounds.position.y + (prefixBounds.size.y * 0.5f)
            });
            prefixText.setPosition({ startX, centerY });
            startX += prefixBounds.size.x;
            target.draw(prefixText);
        }

        if (!tail.empty()) {
            auto tailBounds = tailText.getLocalBounds();
            tailText.setOrigin({
                tailBounds.position.x,
                tailBounds.position.y + (tailBounds.size.y * 0.5f)
            });
            tailText.setPosition({ startX, centerY });
            target.draw(tailText);
        }
    }
}

    void drawForgingOverlay(Game& game, sf::RenderTarget& target) {
        float alpha = weaponForgingOverlayAlpha(game);
        if (alpha <= 0.f)
            return;
        auto size = target.getSize();
        sf::RectangleShape overlay({ static_cast<float>(size.x), static_cast<float>(size.y) });
        overlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(std::clamp(alpha, 0.f, 1.f) * 255.f)));
        target.draw(overlay);
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
        auto labelGlobalBounds = label.getGlobalBounds();

        constexpr float kDragonBackdropPadding = 28.f;
        constexpr float kDragonBackdropRadius = 24.f;
        float spriteLeft = spriteBounds.position.x;
        float spriteTop = spriteBounds.position.y;
        float spriteRight = spriteLeft + spriteBounds.size.x;
        float spriteBottom = spriteTop + spriteBounds.size.y;
        float labelLeft = labelGlobalBounds.position.x;
        float labelTop = labelGlobalBounds.position.y;
        float labelRight = labelLeft + labelGlobalBounds.size.x;
        float labelBottom = labelTop + labelGlobalBounds.size.y;
        float backdropLeft = std::min(spriteLeft, labelLeft) - kDragonBackdropPadding;
        float backdropTop = std::min(spriteTop, labelTop) - (kDragonBackdropPadding * 0.5f);
        float backdropRight = std::max(spriteRight, labelRight) + kDragonBackdropPadding;
        float backdropBottom = std::max(spriteBottom, labelBottom) + (kDragonBackdropPadding * 0.5f);

        RoundedRectangleShape backdrop({ backdropRight - backdropLeft, backdropBottom - backdropTop }, kDragonBackdropRadius, 16);
        backdrop.setPosition({ backdropLeft, backdropTop });
        backdrop.setFillColor(ColorHelper::applyAlphaFactor(sf::Color(4, 4, 6, 220), combinedAlpha));
        target.draw(backdrop);

        target.draw(portrait.sprite);
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

        auto statusPos = game.playerStatusBox.getPosition();
        auto statusSize = game.playerStatusBox.getSize();

        constexpr float kFoldMinProgress = 0.25f;
        constexpr float kFoldStep = 0.08f;
        float foldTarget = game.playerStatusFolded ? kFoldMinProgress : 1.f;
        if (game.playerStatusFoldProgress < foldTarget)
            game.playerStatusFoldProgress = std::min(foldTarget, game.playerStatusFoldProgress + kFoldStep);
        else if (game.playerStatusFoldProgress > foldTarget)
            game.playerStatusFoldProgress = std::max(foldTarget, game.playerStatusFoldProgress - kFoldStep);

        constexpr float kPadding = 8.f;
        constexpr float kBarHeight = 10.f;
        constexpr float kBarSpacing = 13.f;
        constexpr float kLevelBoxHeight = 11.f;
        constexpr float kLevelBoxSpacing = 11.f;
        constexpr float kBadgeVerticalShift = 10.f;
        constexpr unsigned int kLevelTextSize = 18;
        constexpr unsigned int kLabelTextSize = 16;
        constexpr unsigned int kBadgeValueTextSize = 12;
        constexpr float kLabelColumnExtra = 8.f;
        constexpr float kBadgeWidth = 40.f;
        constexpr float kBadgeHeight = 16.f;
        constexpr float kXpGainDuration = 4.5f;
        constexpr float kXpGainSpacing = 12.f;

        float panelAlpha = uiAlphaFactor;
        float foldIndicatorAlpha = uiAlphaFactor;
        float levelBaseWidth = std::clamp(statusSize.x * 0.21f, 84.f, statusSize.x - (kPadding * 2.f));
        float collapsedWidth = levelBaseWidth + 10.f;
        float foldRatio = std::clamp((game.playerStatusFoldProgress - kFoldMinProgress) / (1.f - kFoldMinProgress), 0.f, 1.f);
        float contentWidth = collapsedWidth + (statusSize.x - collapsedWidth) * foldRatio;
        float contentLeft = statusPos.x;
        if (contentWidth <= 0.f)
            return;
        bool collapsedView = foldRatio <= 0.05f;

        float xpRatio = (game.playerXpMax > 0.f)
            ? std::clamp(game.playerXp / game.playerXpMax, 0.f, 1.f)
            : 0.f;
        auto& xpDisplay = game.xpBarDisplayRatio;
        constexpr float kXpLerp = 0.05f;
        if (xpRatio >= xpDisplay) {
            xpDisplay += (xpRatio - xpDisplay) * kXpLerp;
        }
        else {
            xpDisplay = xpRatio;
        }
        xpDisplay = std::clamp(xpDisplay, 0.f, 1.f);

        RoundedRectangleShape statusFrame({ contentWidth, statusSize.y }, 20.f, 24);
        statusFrame.setPosition({ contentLeft, statusPos.y });
        statusFrame.setFillColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, panelAlpha));
        statusFrame.setOutlineThickness(game.playerStatusBox.getOutlineThickness());
        statusFrame.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::FrameGoldLight, panelAlpha));
        target.draw(statusFrame);

        float levelBoxWidth = levelBaseWidth;
        float levelBoxX = contentLeft + kPadding;
        float levelBoxY = statusPos.y + kPadding + kBadgeVerticalShift;

        RoundedRectangleShape levelFrame({ levelBoxWidth, kLevelBoxHeight }, 14.f, 18);
        levelFrame.setPosition({ levelBoxX, levelBoxY });
        levelFrame.setFillColor(ColorHelper::applyAlphaFactor(sf::Color::White, panelAlpha * 0.95f));
        levelFrame.setOutlineThickness(2.f);
        levelFrame.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, panelAlpha));
        target.draw(levelFrame);

        sf::Text levelLabel{ game.resources.uiFont, "Lvl: ", kLevelTextSize };
        levelLabel.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, panelAlpha));
        levelLabel.setOutlineColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, panelAlpha));
        levelLabel.setOutlineThickness(1.f);
        levelLabel.setStyle(sf::Text::Bold);

        sf::Text levelValue{ game.resources.uiFont, std::to_string(game.playerLevel), kLevelTextSize };
        levelValue.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftRed, panelAlpha));
        levelValue.setOutlineColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, panelAlpha));
        levelValue.setOutlineThickness(1.f);
        levelValue.setStyle(sf::Text::Bold);

        auto labelBounds = levelLabel.getLocalBounds();
        auto valueBounds = levelValue.getLocalBounds();
        float textCenterY = levelBoxY + (kLevelBoxHeight * 0.5f);
        float totalTextWidth = labelBounds.size.x + valueBounds.size.x;
        float textStartX = levelBoxX + (levelBoxWidth * 0.5f) - (totalTextWidth * 0.5f);

        levelLabel.setOrigin({ labelBounds.position.x, labelBounds.position.y + (labelBounds.size.y * 0.5f) });
        levelLabel.setPosition({ textStartX, textCenterY });
        levelValue.setOrigin({ valueBounds.position.x, valueBounds.position.y + (valueBounds.size.y * 0.5f) });
        levelValue.setPosition({ textStartX + labelBounds.size.x, textCenterY });
        target.draw(levelLabel);
        target.draw(levelValue);

        float hpBarY = levelBoxY + kLevelBoxHeight + kLevelBoxSpacing;
        float xpBarY = hpBarY + kBarHeight + kBarSpacing;

        float labelColumnWidth = kBadgeWidth + kLabelColumnExtra;
        float barX = contentLeft + kPadding + labelColumnWidth;
        float barWidth = std::max(0.f, contentWidth - (kPadding * 2.f) - labelColumnWidth);

        auto formatValue = [](float current, float maximum) {
            int cur = static_cast<int>(std::lround(std::max(0.f, current)));
            int maxv = static_cast<int>(std::lround(std::max(0.f, maximum)));
            return std::to_string(cur) + "/" + std::to_string(maxv);
        };

        auto formatPercent = [](float current, float maximum) {
            float ratio = (maximum > 0.f) ? std::clamp(current / maximum, 0.f, 1.f) : 0.f;
            int percent = static_cast<int>(std::lround(ratio * 100.f));
            return std::to_string(percent) + "%";
        };

        float badgeLeft = statusPos.x + kPadding;
        auto drawStatBadge = [&](const std::string& label, const sf::Color& outlineColor, float centerY) {
            RoundedRectangleShape badge({ kBadgeWidth, kBadgeHeight }, kBadgeHeight * 0.5f, 16);
            badge.setPosition({
                badgeLeft,
                centerY - (kBadgeHeight * 0.5f)
            });
            badge.setFillColor(ColorHelper::applyAlphaFactor(sf::Color::White, panelAlpha));
            badge.setOutlineThickness(2.f);
            badge.setOutlineColor(ColorHelper::applyAlphaFactor(outlineColor, panelAlpha));
            target.draw(badge);

            sf::Text labelText{ game.resources.uiFont, label, kLabelTextSize };
            labelText.setFillColor(ColorHelper::applyAlphaFactor(outlineColor, panelAlpha));
            labelText.setOutlineColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, panelAlpha));
            labelText.setOutlineThickness(1.f);
            labelText.setStyle(sf::Text::Bold);
            auto labelBoundsLocal = labelText.getLocalBounds();
            labelText.setOrigin({
                labelBoundsLocal.position.x,
                labelBoundsLocal.position.y + (labelBoundsLocal.size.y * 0.5f)
            });
            labelText.setPosition({ badgeLeft + 10.f, centerY });
            target.draw(labelText);

        };
        auto drawBarValue = [&](const std::string& value, float centerY) {
            sf::Text valueText{ game.resources.uiFont, value, kBadgeValueTextSize };
            valueText.setFillColor(ColorHelper::applyAlphaFactor(sf::Color::White, panelAlpha));
            valueText.setOutlineColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, panelAlpha));
            valueText.setOutlineThickness(1.f);
            auto valueBoundsLocal = valueText.getLocalBounds();
            valueText.setOrigin({
                valueBoundsLocal.position.x + (valueBoundsLocal.size.x * 0.5f),
                valueBoundsLocal.position.y + (valueBoundsLocal.size.y * 0.5f)
            });
            valueText.setPosition({ barX + (barWidth * 0.5f), centerY });
            target.draw(valueText);
        };

        if (barWidth > 0.f) {
            const std::string hpValueStr = formatValue(game.playerHp, game.playerHpMax);
            const std::string xpValueStr = formatValue(game.playerXp, game.playerXpMax);
            const std::string hpPercentStr = formatPercent(game.playerHp, game.playerHpMax);
            const std::string xpPercentStr = formatPercent(game.playerXp, game.playerXpMax);

            drawStatBadge("HP", ColorHelper::Palette::SoftRed, hpBarY + (kBarHeight * 0.5f));
            drawStatBadge("XP", ColorHelper::Palette::DarkPurple, xpBarY + (kBarHeight * 0.5f));

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

            RoundedRectangleShape hpBackground({ barWidth, kBarHeight }, kBarHeight * 0.5f, 20);
            hpBackground.setPosition({ barX, hpBarY });
            hpBackground.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::BlueNearBlack, panelAlpha * 0.65f));
            target.draw(hpBackground);

            if (hpRatio > 0.f) {
                float hpFillWidth = std::max(barWidth * hpRatio, kBarHeight);
                hpFillWidth = std::min(barWidth, hpFillWidth);
                RoundedRectangleShape hpFill({ hpFillWidth, kBarHeight }, kBarHeight * 0.5f, 20);
                hpFill.setPosition({ barX, hpBarY });
                hpFill.setFillColor(ColorHelper::applyAlphaFactor(hpColor, panelAlpha * blinkAlpha));
                target.draw(hpFill);
            }

            RoundedRectangleShape hpBorder({ barWidth, kBarHeight }, kBarHeight * 0.5f, 20);
            hpBorder.setPosition({ barX, hpBarY });
            hpBorder.setFillColor(sf::Color::Transparent);
            hpBorder.setOutlineThickness(2.f);
            hpBorder.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Dim, panelAlpha));
            target.draw(hpBorder);

            RoundedRectangleShape xpBackground({ barWidth, kBarHeight }, kBarHeight * 0.5f, 20);
            xpBackground.setPosition({ barX, xpBarY });
            xpBackground.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::BlueNearBlack, panelAlpha * 0.45f));
            target.draw(xpBackground);

            if (xpDisplay > 0.f) {
                float xpFillWidth = std::max(barWidth * xpDisplay, kBarHeight);
                xpFillWidth = std::min(barWidth, xpFillWidth);
                RoundedRectangleShape xpFill({ xpFillWidth, kBarHeight }, kBarHeight * 0.5f, 20);
                xpFill.setPosition({ barX, xpBarY });
                xpFill.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::DarkPurple, panelAlpha));
                target.draw(xpFill);
            }

            RoundedRectangleShape xpBorder({ barWidth, kBarHeight }, kBarHeight * 0.5f, 20);
            xpBorder.setPosition({ barX, xpBarY });
            xpBorder.setFillColor(sf::Color::Transparent);
            xpBorder.setOutlineThickness(2.f);
            xpBorder.setOutlineColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Dim, panelAlpha));
            target.draw(xpBorder);

            if (collapsedView) {
                drawBarValue(hpPercentStr, hpBarY + (kBarHeight * 0.5f));
                drawBarValue(xpPercentStr, xpBarY + (kBarHeight * 0.5f));
            }
            else {
                drawBarValue(hpValueStr, hpBarY + (kBarHeight * 0.5f));
                drawBarValue(xpValueStr, xpBarY + (kBarHeight * 0.5f));
            }
        }

        auto& xpGain = game.xpGainDisplay;
        if (xpGain.active) {
            float progress = std::clamp(xpGain.clock.getElapsedTime().asSeconds() / kXpGainDuration, 0.f, 1.f);
            if (progress >= 1.f) {
                xpGain.active = false;
            }
            else {
                float fade = (1.f - progress) * panelAlpha;
                float pop = std::sin(progress * 3.14159265f);
                float scale = 1.f + (0.25f * pop);
                sf::Text gainText{
                    game.resources.uiFont,
                    "+" + std::to_string(xpGain.amount) + " XP",
                    kLabelTextSize + 2
                };
                gainText.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::DarkPurple, fade));
                gainText.setOutlineColor(ColorHelper::applyAlphaFactor(sf::Color::White, fade));
                gainText.setOutlineThickness(2.f);
                gainText.setStyle(sf::Text::Bold);
                auto gainBounds = gainText.getLocalBounds();
                gainText.setOrigin({
                    gainBounds.position.x,
                    gainBounds.position.y + (gainBounds.size.y * 0.5f)
                });
                gainText.setScale({ scale, scale });
                gainText.setPosition({
                    contentLeft + contentWidth + kXpGainSpacing,
                    xpBarY + (kBarHeight * 0.5f)
                });
                target.draw(gainText);
            }
        }

        constexpr float kFoldBarWidth = 12.f;
        constexpr float kFoldBarPadding = 4.f;
        float foldBarHeight = statusSize.y * 0.85f;
        RoundedRectangleShape foldBar({ kFoldBarWidth, foldBarHeight }, kFoldBarWidth * 0.5f, 12);
        float foldBarX = contentLeft - kFoldBarWidth - kFoldBarPadding;
        float foldBarY = statusPos.y + (statusSize.y - foldBarHeight) * 0.5f;
        foldBar.setPosition({ foldBarX, foldBarY });
        foldBar.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::SoftYellow, foldIndicatorAlpha));
        target.draw(foldBar);

        sf::Text foldSymbol{ game.resources.uiFont, "+", 18 };
        foldSymbol.setFillColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, uiAlphaFactor));
        foldSymbol.setStyle(sf::Text::Bold);
        auto foldBounds = foldSymbol.getLocalBounds();
        foldSymbol.setOrigin({
            foldBounds.position.x + (foldBounds.size.x * 0.5f),
            foldBounds.position.y + (foldBounds.size.y * 0.5f)
        });
        foldSymbol.setPosition({ foldBarX + (kFoldBarWidth * 0.5f), foldBarY + (foldBarHeight * 0.5f) });
        target.draw(foldSymbol);

        game.playerStatusFoldBarBounds = sf::FloatRect(
            sf::Vector2f{ foldBarX, foldBarY },
            sf::Vector2f{ kFoldBarWidth, foldBarHeight }
        );
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
    sf::Vector2f boxCenter{
        boxPos.x + (boxSize.x * 0.5f),
        boxPos.y + (boxSize.y * 0.5f)
    };
    locationName.setPosition(boxCenter);

    auto globalBounds = locationName.getGlobalBounds();
    float textY = globalBounds.position.y + (globalBounds.size.y * 0.5f);

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

void drawDialogueUI(Game& game, sf::RenderTarget& target, bool skipConfirmation, float* outAlpha, bool renderMenu) {

    ui::brokenweapon::updatePreview(game);
    UiVisibility visibility = computeDialogueVisibility(game);

    if (game.state == GameState::IntroTitle && !game.uiFadeInActive)
        return;

    if (visibility.hidden)
        return;

    drawSceneBackground(game, target, visibility.alphaFactor);

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
    bool inIntroTransition =
        game.pendingIntroDialogue
        || game.pendingPerigonalDialogue
        || game.pendingGonadPartOneDialogue
        || game.pendingBlacksmithDialogue
        || game.pendingGonadPartTwoDialogue
        || game.uiFadeOutActive;
    bool forgingOverlayActive = weaponForgingOverlayVisible(game);
    bool showLocationBox = !introTitleActive && !inIntroTransition && !forgingOverlayActive && game.currentDialogue && game.currentDialogue != &intro;

    dialogDraw::drawDialogueFrames(game, target, uiAlphaFactor, glowColor);

    if (showLocationBox) {
        drawPlayerStatus(game, target, uiAlphaFactor);
        drawLocationBox(game, target, uiAlphaFactor, glowColor);
    }

    bool keepShowingLastFeedbackLine =
        game.state == GameState::Quiz
        && game.currentDialogue == &game.quiz.feedbackDialogue
        && !game.visibleText.empty();
    bool keepShowingBookshelfPrompt =
        game.state == GameState::Bookshelf
        && game.currentDialogue == &game.transientDialogue
        && !game.visibleText.empty()
        && game.bookshelf.awaitingDragonstoneReward;
    bool skipDragonDuringQuiz = (game.state == GameState::Quiz && !keepShowingLastFeedbackLine);
    if ((game.currentDialogue == &dragon || keepShowingLastFeedbackLine) && !skipDragonDuringQuiz) {
        std::optional<LocationId> highlightLocation;
        if (keepShowingLastFeedbackLine)
            highlightLocation = game.quiz.targetLocation;
        drawDragonShowcase(game, target, uiAlphaFactor, highlightLocation);
    }

    bool hasDialogueLine = game.currentDialogue && game.dialogueIndex < game.currentDialogue->size();
    if (!hasDialogueLine && !game.confirmationPrompt.active && !keepShowingLastFeedbackLine && !keepShowingBookshelfPrompt) {
        game.lastSpeaker.reset();
        return;
    }

    const DialogueLine* line = nullptr;
    std::string fullText;
    if (hasDialogueLine) {
        line = &(*game.currentDialogue)[game.dialogueIndex];
        fullText = injectSpeakerNames(line->text, game);
    }
    else if ((keepShowingLastFeedbackLine || keepShowingBookshelfPrompt) && game.currentDialogue && !game.currentDialogue->empty()) {
        line = &game.currentDialogue->back();
    }

    float delay = 0.02f;
    bool isTyping = false;
    bool isMapTutorialLine =
        hasDialogueLine
        && game.currentDialogue == &gonad_part_two
        && game.mapTutorialActive
        && game.dialogueIndex >= kMapTutorialStartLineIndex
        && game.dialogueIndex <= kMapTutorialEndLineIndex;

    if (isMapTutorialLine) {
        game.visibleText = fullText;
        game.charIndex = fullText.size();
    }

    bool allowTyping = !game.teleportController.active()
        && !game.confirmationPrompt.active
        && hasDialogueLine
        && game.state != GameState::IntroTitle
        && !isMapTutorialLine;
    if (allowTyping)
        isTyping = updateTypewriter(game, fullText, delay);

        if (line) {
            TextStyles::SpeakerStyle info = TextStyles::speakerStyle(line->speaker);
            bool sameSpeakerAsPrevious = game.lastSpeaker && *game.lastSpeaker == line->speaker;
            // Hide the speaker name until some text is visible (prevents leftover
            // speaker names from appearing immediately after the intro title).
            if (!isMapTutorialLine) {
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
                if (game.currentDialogue == &blacksmith
                    && game.dialogueIndex == kBlacksmithRestLineIndex
                    && game.visibleText.size() >= fullText.size()
                    && game.weaponForging.phase == Game::WeaponForgingState::Phase::Idle
                    && game.selectedWeaponIndex >= 0)
                {
                    startWeaponForgingRest(game);
                }
            }
            else {
                game.lastSpeaker = line->speaker;
            }
        }

    if (line
        && line->triggersGenderSelection
        && !game.genderSelectionActive
        && game.visibleText.size() >= fullText.size()) {
        ui::genderSelection::start(game);
    }

    if (game.state == GameState::Dialogue
        && game.currentDialogue == &blacksmith
        && game.dialogueIndex == kBlacksmithSelectionLineIndex
        && !game.confirmationPrompt.active
        && game.state != GameState::WeaponSelection
        && !fullText.empty()
        && game.visibleText.size() >= fullText.size()) {
        openBlacksmithWeaponSelection(game, fullText);
        return;
    }

    ui::genderSelection::draw(game, target, uiAlphaFactor);

    bool deferredConfirmation = game.confirmationPrompt.active && !skipConfirmation && game.forcedDestinationSelection;
    if (game.confirmationPrompt.active && !skipConfirmation && !deferredConfirmation) {
        drawConfirmationPrompt(game, target, uiAlphaFactor);
        drawQuestPopup(game, target, uiAlphaFactor);
        return;
    }

    if (game.askingName)
        dialogDraw::drawNameInput(target, game, uiAlphaFactor);

    if (!game.genderSelectionActive && !isMapTutorialLine)
        dialogDraw::drawReturnPrompt(target, game, uiAlphaFactor, isTyping);

    ui::brokenweapon::drawPreview(game, target);
    drawMapItemPopup(game, target, uiAlphaFactor);
    drawForgedWeaponPopup(game, target, uiAlphaFactor);
    drawForgingOverlay(game, target);
    if (renderMenu)
    ui::menu::draw(game, target);

    if (game.menuMapPopup) {
        drawMapSelectionPopup(game, target, *game.menuMapPopup);
        game.menuMapPopup.reset();
    }

    drawQuestPopup(game, target, uiAlphaFactor);

    if (deferredConfirmation) {
        drawConfirmationPrompt(game, target, uiAlphaFactor);
        return;
    }
}
