#pragma once
#include <SFML/Graphics.hpp>
#include <stdexcept>
#include "textStyles.hpp"

class NineSliceBox {
    public:
        explicit NineSliceBox(int borderSize = 12)
        : border(borderSize) {}
    
    void load(const std::filesystem::path& path) {
        if (!texture.loadFromFile(path)) {
            throw std::runtime_error("Failed to load 9-slice texture from: " + path.string());
        }
        texSize = texture.getSize();
    }

    void draw(  sf::RenderTarget& target
        , const sf::RectangleShape& box
        , sf::Color color = TextStyles::UI::PanelDark) const {
        draw(target, box.getPosition(), box.getSize(), color);
    }
    // pos = Top Left Corner
    // size = Targeted size of the box inside the winwow
    void draw(sf::RenderTarget& target
            , sf::Vector2f pos
            , sf::Vector2f size
            , sf::Color color = TextStyles::UI::PanelDark) const {
        if (texSize.x == 0 || texSize.y == 0) return;

        const float x = pos.x;
        const float y = pos.y;
        const float w = size.x;
        const float h = size.y;

        const int texW = static_cast<int>(texSize.x);
        const int texH = static_cast<int>(texSize.y);
        const int innerW = texW - 2 * border;
        const int innerH = texH - 2 * border;

        sf::Sprite sprite(texture);
        sprite.setColor(color);
        
        // === Corners ===
        // Top Left
        sprite.setScale({1.f, 1.f});
        sprite.setTextureRect(sf::IntRect({0, 0}, {border, border}));
        sprite.setPosition({x, y});
        target.draw(sprite);

        // Top Right
        sprite.setTextureRect(sf::IntRect({texW - border, 0}, {border, border}));
        sprite.setPosition({x + w - border, y});
        target.draw(sprite);

        // Bottom Left
        sprite.setTextureRect(sf::IntRect({0, texH - border}, {border, border}));
        sprite.setPosition({x, y + h - border});
        target.draw(sprite);

        // Bottom Right
        sprite.setTextureRect(sf::IntRect({texW - border, texH - border}, {border, border}));
        sprite.setPosition({x + w - border, y + h - border});
        target.draw(sprite);

        // === Edges ===
        // Top
        sprite.setTextureRect(sf::IntRect({border, 0}, {innerW, border}));
        sprite.setPosition({x + border, y});
        {
            float scaleX = (w - 2.f * border) / static_cast<float>(innerW);
            sprite.setScale({scaleX, 1.f});
        }
        target.draw(sprite);

        // Bottom
        sprite.setTextureRect(sf::IntRect({border, texH - border}, {innerW, border}));
        sprite.setPosition({x + border, y + h - border});
        {
            float scaleX = (w - 2.f * border) / static_cast<float>(innerW);
            sprite.setScale({scaleX, 1.f});
        }
        target.draw(sprite);

        // Left
        sprite.setTextureRect(sf::IntRect({0, border}, {border, innerH}));
        sprite.setPosition({x, y + border});
        {
            float scaleY = (h - 2.f * border) / static_cast<float>(innerH);
            sprite.setScale({1.f, scaleY});
        }
        target.draw(sprite);

        // Right
        sprite.setTextureRect(sf::IntRect({texW - border, border}, {border, innerH}));
        sprite.setPosition({x + w - border, y + border});
        {
            float scaleY = (h - 2.f * border) / static_cast<float>(innerH);
            sprite.setScale({1.f, scaleY});
        }
        target.draw(sprite);

        // Middle
        sprite.setTextureRect(sf::IntRect({border, border}, {innerW, innerH}));
        sprite.setPosition({x + border, y + border});
        sprite.setColor(sf::Color::Transparent);
        {
            float scaleX = (w - 2.f * border) / static_cast<float>(innerW);
            float scaleY = (h - 2.f * border) / static_cast<float>(innerH);
            sprite.setScale({scaleX, scaleY});
        }
        target.draw(sprite);
    }

    void drawScaled(sf::RenderTarget& target,
                    sf::Vector2f pos,
                    sf::Vector2f size,
                    sf::Color color,
                    float scaleOffset) const
    {
        draw(
            target,
            { pos.x - scaleOffset, pos.y - scaleOffset },
            { size.x + scaleOffset * 2.f, size.y + scaleOffset * 2.f },
            color
        );
    }

    private:
        sf::Texture texture;
        sf::Vector2u texSize{};
        int border;
};