#include <algorithm>
#include <array>
#include <limits>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <SFML/Graphics/Sprite.hpp>

#include "helper/colorHelper.hpp"
#include "ui/bookshelfUI.hpp"
#include "ui/quizUI.hpp"

namespace ui::bookshelf {

namespace {
    constexpr float kBookshelfScale = 0.5f;
    constexpr float kBaseShelfWidth = 515.f;
    constexpr float kBaseShelfHeight = 130.f;
    constexpr float kBaseShelfGap = 30.f;
    constexpr float kBaseShelfPaddingX = 14.f;
    constexpr float kBaseBookSpacing = 6.f;
    constexpr float kShelfTextMargin = 32.f;
    constexpr float kShelfTopMargin = 16.f;
    constexpr std::size_t kClickableBookCount = 5;
    constexpr float kStatusFontSize = 26.f;
    constexpr float kShelfHorizontalOffset = 40.f;
    constexpr float kShelfVerticalOffset = 20.f;
    constexpr float kBookshelfVerticalDrop = 50.f;
    constexpr float kShelfRowVerticalOffset = 8.f;

    const std::array<std::pair<const char*, const char*>, 18> kSillyBooks = { {
        { "How to Train Your Human", "Hmm. Failed experiment." },
        { "1001 Ways to Sit on Gold Without Crushing It", "Surprisingly practical." },
        { "The Complete History of Dragon Fashion", "Oh. So that’s how naked dragons look like!" },
        { "Fireproof Undertrousers – Vol. II", "Vol. I did not survive." },
        { "Advanced Hoarding: When Is It Too Much?", "There is no such thing." },
        { "Maps That Should Not Exist", "Pages are missing." },
        { "Umbra Ossea – Myths, Lies and Bad Ideas", "Someone scribbled notes in the margins." },
        { "Do Not Summon This", "Someone clearly did." },
        { "Forbidden Geography for Curious Minds", "This chapter is torn out." },
        { "So You Accidentally Burned Your Cave", "Happens." },
        { "Humans: A Beginner’s Guide", "Humans have no wings. Weeeird." },
        { "Gold Piles and Personal Space", "Overrated." },
        { "How Heavy Is Too Heavy?", "We’ll find out." },
        { "The Subtle Art of Not Flying", "Highly theoretical." },
        { "Pages That Refuse to Stay in Order", "This is confusing." },
        { "Do Not Fold This Map", "Someone did." },
        { "Places Best Left Unvisited", "Bold statement." },
        { "The Cartographer Who Went Missing", "Unfinished." }
    } };

    void showBookDialogue(Game& game, const std::pair<const char*, const char*>& book) {
        game.transientDialogue.clear();
        game.transientDialogue.reserve(2);
        game.transientDialogue.push_back({ TextStyles::SpeakerId::Player, std::string("\"") + book.first + "\"" });
        game.transientDialogue.push_back({ TextStyles::SpeakerId::Player, std::string(book.second) });
        game.currentDialogue = &game.transientDialogue;
        game.dialogueIndex = 0;
        game.visibleText.clear();
        game.charIndex = 0;
        game.typewriterClock.restart();
        game.bookshelf.returnAfterBookDialogue = true;
        game.state = GameState::Dialogue;
    }

    std::vector<const sf::Texture*> bookshelfBookTextures(const Game& game) {
        return {
            &game.resources.bookSingle01,
            &game.resources.bookSingle02,
            &game.resources.bookSingle03,
            &game.resources.bookSingle04,
            &game.resources.bookSingle05,
            &game.resources.bookStapled01,
            &game.resources.bookStapled02,
            &game.resources.booksRowed01,
            &game.resources.booksRowed02,
            &game.resources.booksRowed03,
            &game.resources.booksTipped01,
            &game.resources.booksTipped02
        };
    }

    float minimumTextureWidth(const std::vector<const sf::Texture*>& textures) {
        float minWidth = std::numeric_limits<float>::infinity();
        for (const auto* texture : textures) {
            const auto width = static_cast<float>(texture->getSize().x);
            minWidth = std::min(minWidth, width);
        }
        return minWidth;
    }


    void layoutShelves(Game& game, const std::vector<const sf::Texture*>& textures) {
        auto& state = game.bookshelf;
        state.books.clear();
        state.books.reserve(40);

        float shelfWidth = kBaseShelfWidth * kBookshelfScale;
        float shelfHeight = kBaseShelfHeight * kBookshelfScale;
        float shelfGap = kBaseShelfGap * kBookshelfScale;
        float shelfPadding = kBaseShelfPaddingX * kBookshelfScale;
        float bookSpacing = kBaseBookSpacing * kBookshelfScale;
        float minWidth = minimumTextureWidth(textures) * kBookshelfScale;

        float windowWidth = static_cast<float>(game.window.getSize().x);
        float windowHeight = static_cast<float>(game.window.getSize().y);
        float textTop = game.textBox.getPosition().y;
        float shelfBottom = textTop - kShelfTextMargin;
        float totalHeight = shelfHeight * static_cast<float>(state.shelfBounds.size())
            + shelfGap * static_cast<float>(state.shelfBounds.size() - 1);
        float desiredTop = (windowHeight - totalHeight) * 0.5f;
        desiredTop = std::max(desiredTop, kShelfTopMargin);
        float maxAllowedTop = shelfBottom - totalHeight;
        if (maxAllowedTop < kShelfTopMargin)
            maxAllowedTop = kShelfTopMargin;
        float shelfTop = std::min(desiredTop, maxAllowedTop);
        shelfTop = std::max(kShelfTopMargin, shelfTop - kShelfVerticalOffset);
        float shelfLeft = (windowWidth - shelfWidth) * 0.5f - kShelfHorizontalOffset;
        shelfLeft = std::max(kShelfTopMargin, shelfLeft);
        const auto& shelfTexture = game.resources.bookshelf;
        float shelfSpriteWidth = static_cast<float>(shelfTexture.getSize().x) * kBookshelfScale;
        float shelfSpriteHeight = static_cast<float>(shelfTexture.getSize().y) * kBookshelfScale;
        float spriteLeft = shelfLeft + (shelfWidth - shelfSpriteWidth) * 0.5f;
        float spriteTop = shelfTop + (totalHeight - shelfSpriteHeight) * 0.5f;
        spriteLeft = std::max(0.f, std::min(spriteLeft, windowWidth - shelfSpriteWidth));
        spriteTop = std::max(0.f, std::min(spriteTop, shelfBottom - shelfSpriteHeight));
        state.shelfPosition = { spriteLeft, spriteTop + kBookshelfVerticalDrop };
        state.shelfScale = kBookshelfScale;
        float layoutTop = spriteTop + (shelfSpriteHeight - totalHeight) * 0.5f;
        layoutTop += kShelfRowVerticalOffset;
        layoutTop += kBookshelfVerticalDrop;

        for (int shelfIndex = 0; shelfIndex < static_cast<int>(state.shelfBounds.size()); ++shelfIndex) {
            const float rowTop = layoutTop + shelfIndex * (shelfHeight + shelfGap);
            state.shelfBounds[shelfIndex] = sf::FloatRect({
                shelfLeft,
                rowTop
            }, {
                shelfWidth,
                shelfHeight
            });

            float cursor = shelfLeft + shelfPadding;
            float boundary = shelfLeft + shelfWidth - shelfPadding;

            while (true) {
                float remaining = boundary - cursor;
                if (remaining < minWidth)
                    break;

                std::vector<const sf::Texture*> fits;
                fits.reserve(textures.size());
                for (const sf::Texture* tex : textures) {
                    float scaledWidth = static_cast<float>(tex->getSize().x) * kBookshelfScale;
                    if (scaledWidth <= remaining)
                        fits.push_back(tex);
                }
                if (fits.empty())
                    break;

                std::uniform_int_distribution<std::size_t> picker(0, fits.size() - 1);
                const sf::Texture* chosen = fits[picker(state.rng)];
                float width = static_cast<float>(chosen->getSize().x) * kBookshelfScale;
                float height = static_cast<float>(chosen->getSize().y) * kBookshelfScale;

                Game::BookshelfState::BookSlot slot{};
                slot.texture = chosen;
                slot.position = { cursor, rowTop + shelfHeight - height };
                slot.bounds = sf::FloatRect(slot.position, { width, height });
                slot.scale = kBookshelfScale;
                slot.sillyIndex = std::numeric_limits<std::size_t>::max();
                state.books.push_back(std::move(slot));

                cursor += width + bookSpacing;
            }
        }
    }


    void markClickableBooks(Game& game) {
        auto& state = game.bookshelf;
        if (state.books.empty())
            return;

        std::vector<std::size_t> indices(state.books.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), state.rng);

        std::size_t clickableCount = std::min(kClickableBookCount, state.books.size());
        indices.resize(clickableCount);
        std::uniform_int_distribution<std::size_t> sillyPicker(0, kSillyBooks.size() - 1);
        for (std::size_t idx : indices) {
            state.books[idx].clickable = true;
            state.books[idx].sillyIndex = sillyPicker(state.rng);
        }

        std::uniform_int_distribution<std::size_t> special(0, clickableCount - 1);
        std::size_t mapIndex = indices[special(state.rng)];
        state.books[mapIndex].mapPiece = true;
    }

    void updateHoveredBook(Game& game, const sf::Vector2f& point) {
        auto& state = game.bookshelf;
        state.hoveredBookIndex = -1;
        for (std::size_t i = 0; i < state.books.size(); ++i) {
            const auto& book = state.books[i];
            if (book.clickable && book.bounds.contains(point)) {
                state.hoveredBookIndex = static_cast<int>(i);
                break;
            }
        }
    }

    sf::Text buildStatus(const Game& game, const std::string& text) {
        sf::Text status(game.resources.uiFont, text, static_cast<unsigned int>(kStatusFontSize));
        status.setFillColor(ColorHelper::Palette::Normal);
        status.setPosition({ 40.f, 620.f });
        return status;
    }
}

void enter(Game& game) {
    auto textures = bookshelfBookTextures(game);
    if (textures.empty())
        return;

    auto& state = game.bookshelf;
    state.mapPieceCollected = false;
    state.returnAfterBookDialogue = false;
    state.hoveredBookIndex = -1;
    for (auto& bounds : state.shelfBounds)
        bounds = {};

    layoutShelves(game, textures);
    markClickableBooks(game);
}

bool handleEvent(Game& game, const sf::Event& event) {
    if (auto move = event.getIf<sf::Event::MouseMoved>()) {
        auto point = game.window.mapPixelToCoords(move->position);
        updateHoveredBook(game, point);
        return true;
    }

    if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button == sf::Mouse::Button::Left) {
            auto point = game.window.mapPixelToCoords(button->position);
            auto& state = game.bookshelf;
            for (auto& book : state.books) {
                if (!book.clickable)
                    continue;

                sf::FloatRect bounds = book.bounds;
                if (!bounds.contains(point))
                    continue;

                if (book.mapPiece) {
                    if (!state.mapPieceCollected) {
                        state.mapPieceCollected = true;
                        state.statusMessage = "The map shard is yours! Return to the dragon to claim the stone.";
                        book.clickable = false;
                        state.hoveredBookIndex = -1;
                        game.exitBookshelfQuest();
                        presentDragonstoneReward(game);
                        return true;
                    }
                    state.statusMessage = "You already took the map shard.";
                } else {
                    book.clickable = false;
                    state.hoveredBookIndex = -1;
                    if (book.sillyIndex < kSillyBooks.size())
                        showBookDialogue(game, kSillyBooks[book.sillyIndex]);
                    else
                        state.statusMessage = "This book refuses to say anything interesting.";
                    return true;
                }

                state.hoveredBookIndex = -1;
                return true;
            }
        }
        return true;
    }

    if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            if (game.bookshelf.awaitingDragonstoneReward && !game.bookshelf.mapPieceCollected) {
                game.bookshelf.statusMessage = "The dragon needs the Umbra Ossea map shard before you leave.";
                return true;
            }
            game.exitBookshelfQuest();
            return true;
        }
    }

    return false;
}

void draw(Game& game, sf::RenderTarget& target) {
    auto& state = game.bookshelf;
    sf::Sprite shelfSprite(game.resources.bookshelf);
    shelfSprite.setPosition(state.shelfPosition);
    shelfSprite.setScale({ state.shelfScale, state.shelfScale });
    target.draw(shelfSprite);

    for (std::size_t i = 0; i < state.books.size(); ++i) {
        const auto& book = state.books[i];
        if (!book.texture)
            continue;
        sf::Sprite sprite(*book.texture);
        sprite.setScale({ book.scale, book.scale });
        sprite.setPosition(book.position);
        if (book.clickable && static_cast<int>(i) == state.hoveredBookIndex)
            sprite.setColor(ColorHelper::darken(sf::Color::White, 0.25f));
        target.draw(sprite);
    }

    auto status = buildStatus(game, state.statusMessage);
    target.draw(status);

}

} // namespace ui::bookshelf
