// === C++ Libraries ===
#include <algorithm>
#include <cmath>

// === SFML Libraries ===
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

// === Header Files ===
#include "creditsUI.hpp"
#include "core/game.hpp"
#include "helper/colorHelper.hpp"
#include "story/textStyles.hpp"

namespace ui::credits {

namespace {
constexpr float kTitleFadeInDuration = 1.5f;
constexpr float kPanelFadeInDuration = 1.0f;
constexpr float kEntryFadeInDuration = 1.5f;
constexpr float kEntryHoldDuration = 4.0f;
constexpr float kEntryFadeOutDuration = 1.0f;
constexpr float kPanelFadeOutDuration = 1.0f;
constexpr float kBetaFadeInDuration = 1.0f;
constexpr float kBetaHoldDuration = 6.0f;
constexpr float kCreditsTitleOffset = 80.f;

const char* categoryLabel(Category category) {
    switch (category) {
        case Category::Acts: return "Acts";
        case Category::Dragons: return "Dragons";
        case Category::Specials: return "Specials";
    }
    return "";
}

float clamp01(float value) {
    return std::clamp(value, 0.f, 1.f);
}

void populateEntries(Game& game, State& state) {
    state.entries.clear();
    auto addEntry = [&](const sf::Texture& texture, std::string role, std::string actor, Category category) {
        state.entries.push_back(Entry{
            &texture,
            std::move(role),
            "portrayed by " + std::move(actor),
            category
        });
    };

    addEntry(game.resources.creditActAsha, "Asha Scale", "Ashton Knight", Category::Acts);
    addEntry(game.resources.creditActEmber, "Ember Scale", "Scarlett Knight", Category::Acts);
    addEntry(game.resources.creditActBates, "Master Bates", "Edgar Heggs", Category::Acts);
    addEntry(game.resources.creditActBatesDragon, "Master Bates (Dragon Form)", "Ari Moss", Category::Acts);
    addEntry(game.resources.creditActNoah, "Noah Lott", "Wisely Grant", Category::Acts);
    addEntry(game.resources.creditActWanda, "Wanda Rinn", "Mara Trail", Category::Acts);
    addEntry(game.resources.creditActWill, "Will Figsid", "Mason Hardy", Category::Acts);

    addEntry(game.resources.creditActFlawtin, "Flawtin Seamen", "Zuri Kofi", Category::Dragons);
    addEntry(game.resources.creditActGrounded, "Grounded Claymore", "Dustin Quarry", Category::Dragons);
    addEntry(game.resources.creditActGustavo, "Gustavo Windimaess", "Yuna Sato", Category::Dragons);
    addEntry(game.resources.creditActRowsted, "Rowsted Sheacane", "Imana Zola", Category::Dragons);

    addEntry(game.resources.creditActTory, "Tory Tailor", "Pascal Scholz", Category::Specials);
    state.entries.push_back(Entry{
        &game.resources.creditActKensen,
        "Special thanks to my artist:",
        "Kensen Futagoza",
        Category::Specials
    });

    state.betaTexture = &game.resources.creditBetaTesterAlex;
}

float entryAlphaFor(float elapsed) {
    if (elapsed < kEntryFadeInDuration)
        return clamp01(elapsed / kEntryFadeInDuration);
    if (elapsed < kEntryFadeInDuration + kEntryHoldDuration)
        return 1.f;
    if (elapsed < kEntryFadeInDuration + kEntryHoldDuration + kEntryFadeOutDuration) {
        float fade = (elapsed - kEntryFadeInDuration - kEntryHoldDuration) / kEntryFadeOutDuration;
        return 1.f - clamp01(fade);
    }
    return 0.f;
}

} // namespace

void start(Game& game) {
    auto& state = game.creditsState;
    state.active = true;
    state.initialized = true;
    state.phase = State::Phase::TitleFadeIn;
    state.entryIndex = 0;
    state.titleAlpha = 0.f;
    state.panelAlpha = 0.f;
    state.entryAlpha = 0.f;
    state.betaAlpha = 0.f;
    state.phaseClock.restart();
    populateEntries(game, state);
    game.startTitleScreenMusic();
}

void update(Game& game) {
    if (game.state != GameState::Credits)
        return;

    auto& state = game.creditsState;
    if (!state.initialized)
        start(game);

    float elapsed = state.phaseClock.getElapsedTime().asSeconds();

    switch (state.phase) {
        case State::Phase::TitleFadeIn: {
            state.titleAlpha = clamp01(elapsed / kTitleFadeInDuration);
            if (state.titleAlpha >= 1.f) {
                state.phase = State::Phase::PanelFadeIn;
                state.phaseClock.restart();
            }
            break;
        }
        case State::Phase::PanelFadeIn: {
            state.panelAlpha = clamp01(elapsed / kPanelFadeInDuration);
            if (state.panelAlpha >= 1.f) {
                state.phase = State::Phase::EntryDisplay;
                state.phaseClock.restart();
            }
            break;
        }
        case State::Phase::EntryDisplay: {
            state.entryAlpha = entryAlphaFor(elapsed);
            float total = kEntryFadeInDuration + kEntryHoldDuration + kEntryFadeOutDuration;
            if (elapsed >= total) {
                state.entryIndex++;
                if (state.entryIndex >= state.entries.size()) {
                    state.phase = State::Phase::PanelFadeOut;
                }
                state.phaseClock.restart();
            }
            break;
        }
        case State::Phase::PanelFadeOut: {
            state.panelAlpha = 1.f - clamp01(elapsed / kPanelFadeOutDuration);
            if (elapsed >= kPanelFadeOutDuration) {
                state.panelAlpha = 0.f;
                state.phase = State::Phase::BetaDisplay;
                state.phaseClock.restart();
            }
            break;
        }
        case State::Phase::BetaDisplay: {
            if (elapsed < kBetaFadeInDuration) {
                state.betaAlpha = clamp01(elapsed / kBetaFadeInDuration);
            } else {
                state.betaAlpha = 1.f;
            }
            if (elapsed >= kBetaFadeInDuration + kBetaHoldDuration) {
                state.phase = State::Phase::Complete;
                state.active = false;
                state.initialized = false;
                game.window.close();
            }
            break;
        }
        case State::Phase::Complete:
            break;
    }
}

void draw(Game& game, sf::RenderTarget& target) {
    if (game.state != GameState::Credits)
        return;

    auto& state = game.creditsState;
    game.startTitleScreenMusic();

    auto size = target.getSize();
    float windowWidth = static_cast<float>(size.x);
    float windowHeight = static_cast<float>(size.y);
    float squareSize = std::min(windowWidth, windowHeight) * 0.5f;
    sf::Vector2f squarePos{
        (windowWidth - squareSize) * 0.5f,
        (windowHeight - squareSize) * 0.5f
    };

    sf::Text creditsTitle{ game.resources.battleFont, "Credits", 48 };
    auto titleBounds = creditsTitle.getLocalBounds();
    creditsTitle.setOrigin({ titleBounds.position.x + titleBounds.size.x * 0.5f, titleBounds.position.y + titleBounds.size.y * 0.5f });
    creditsTitle.setPosition({ windowWidth * 0.5f, kCreditsTitleOffset });
    sf::Color titleColor = sf::Color::White;
    titleColor.a = static_cast<std::uint8_t>(255.f * clamp01(state.titleAlpha));
    creditsTitle.setFillColor(titleColor);
    target.draw(creditsTitle);

    if (state.panelAlpha > 0.f) {
        sf::RectangleShape panel({ squareSize, squareSize });
        panel.setPosition(squarePos);
        sf::Color panelFill = TextStyles::UI::PanelDark;
        panelFill.a = static_cast<std::uint8_t>(255.f * state.panelAlpha);
        panel.setFillColor(panelFill);
        panel.setOutlineThickness(2.f);
        sf::Color outlineColor = ColorHelper::Palette::SoftYellow;
        outlineColor.a = static_cast<std::uint8_t>(255.f * state.panelAlpha);
        panel.setOutlineColor(outlineColor);
        target.draw(panel);

    }

    if (state.phase == State::Phase::EntryDisplay && state.entryIndex < state.entries.size()) {
        const auto& entry = state.entries[state.entryIndex];
        float contentAlpha = clamp01(state.panelAlpha * state.entryAlpha);

        sf::Text category{ game.resources.uiFont, categoryLabel(entry.category), 36 };
        auto catBounds = category.getLocalBounds();
        category.setOrigin({ catBounds.position.x + catBounds.size.x * 0.5f, catBounds.position.y + catBounds.size.y * 0.5f });
        float categoryY = squarePos.y - 36.f;
        category.setPosition({ windowWidth * 0.5f, categoryY });
        sf::Color catColor = sf::Color::White;
        catColor.a = static_cast<std::uint8_t>(255.f * contentAlpha);
        category.setFillColor(catColor);
        target.draw(category);

        if (entry.texture && entry.texture->getSize().x > 0 && entry.texture->getSize().y > 0) {
            sf::Sprite sprite(*entry.texture);
            auto texSize = entry.texture->getSize();
            float scale = std::min(
                squareSize / static_cast<float>(texSize.x),
                squareSize / static_cast<float>(texSize.y)
            );
            sprite.setScale({ scale, scale });
            auto bounds = sprite.getLocalBounds();
            sprite.setOrigin({ bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f });
            float imageCenterX = squarePos.x + squareSize * 0.5f;
            float imageCenterY = squarePos.y + squareSize * 0.5f;
            sprite.setPosition({ imageCenterX, imageCenterY });
            sprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(255.f * contentAlpha)));
            target.draw(sprite);
        }

        float textSize = 24.f;
        float lineSpacing = textSize * 1.35f;
        sf::Text line1{ game.resources.uiFont, entry.line1, static_cast<unsigned int>(textSize) };
        sf::Text line2{ game.resources.uiFont, entry.line2, static_cast<unsigned int>(textSize) };
        sf::Color textColor = ColorHelper::Palette::SoftYellow;
        textColor.a = static_cast<std::uint8_t>(255.f * contentAlpha);
        line1.setFillColor(textColor);
        line2.setFillColor(textColor);
        auto line1Bounds = line1.getLocalBounds();
        auto line2Bounds = line2.getLocalBounds();
        float textStartY = squarePos.y + squareSize + 18.f;
        line1.setOrigin({ line1Bounds.position.x + line1Bounds.size.x * 0.5f, line1Bounds.position.y });
        line2.setOrigin({ line2Bounds.position.x + line2Bounds.size.x * 0.5f, line2Bounds.position.y });
        line1.setPosition({ squarePos.x + squareSize * 0.5f, textStartY });
        line2.setPosition({ squarePos.x + squareSize * 0.5f, textStartY + lineSpacing });
        target.draw(line1);
        target.draw(line2);
    }

    if (state.phase == State::Phase::BetaDisplay && state.betaTexture && state.betaAlpha > 0.f) {
        sf::Sprite sprite(*state.betaTexture);
        auto texSize = state.betaTexture->getSize();
        float maxWidth = windowWidth * 0.7f;
        float maxHeight = windowHeight * 0.45f;
        float scale = std::min(
            maxWidth / static_cast<float>(texSize.x),
            maxHeight / static_cast<float>(texSize.y)
        );
        sprite.setScale({ scale, scale });
        auto bounds = sprite.getLocalBounds();
        sprite.setOrigin({ bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f });
        float centerX = windowWidth * 0.5f;
        float centerY = windowHeight * 0.5f;
        sprite.setPosition({ centerX, centerY });
        sprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(255.f * state.betaAlpha)));
        target.draw(sprite);

        sf::Text thanks{ game.resources.uiFont, "And another special thanks to my Beta Tester Alex!", 24 };
        auto thanksBounds = thanks.getLocalBounds();
        thanks.setOrigin({ thanksBounds.position.x + thanksBounds.size.x * 0.5f, thanksBounds.position.y + thanksBounds.size.y * 0.5f });
        thanks.setPosition({ centerX, centerY + (bounds.size.y * scale * 0.5f) + 32.f });
        sf::Color thanksColor = ColorHelper::Palette::SoftYellow;
        thanksColor.a = static_cast<std::uint8_t>(255.f * state.betaAlpha);
        thanks.setFillColor(thanksColor);
        target.draw(thanks);
    }
}

} // namespace ui::credits
