#include "mapSelectionUI.hpp"
#include "core/game.hpp"

#include "story/textStyles.hpp"
#include "rendering/colorHelper.hpp"
#include <SFML/Window/Mouse.hpp>

namespace {
    struct LocationItem {
        const sf::Texture* sepia = nullptr;
        const sf::Texture* color = nullptr;
        sf::Vector2f mapRelativePos; // x,y in [-0.5..0.5] relative to map center
        std::string name;
    };

    // Build the five locations with relative positions on the map
    std::vector<LocationItem> buildLocations(const Game& game) {
        std::vector<LocationItem> out;

        // Gonad - center
        LocationItem gonad;
        gonad.sepia = &game.resources.locationGonadSepia;
        gonad.color = &game.resources.locationGonadColored;
        gonad.mapRelativePos = { 0.f, 0.f };
        gonad.name = "Gonad";
        out.push_back(std::move(gonad));

        // Lacrimere - top (move close to border, keep 2% margin)
        LocationItem lac;
        lac.sepia = &game.resources.locationLacrimereSepia;
        lac.color = &game.resources.locationLacrimereColored;
        lac.mapRelativePos = { 0.f, -0.48f };
        lac.name = "Lacrimere";
        out.push_back(std::move(lac));

        // Blyathyroid - right (move close to border)
        LocationItem bly;
        bly.sepia = &game.resources.locationBlyathyroidSepia;
        bly.color = &game.resources.locationBlyathyroidColored;
        bly.mapRelativePos = { 0.48f, 0.f };
        bly.name = "Blyathyroid";
        out.push_back(std::move(bly));

        // Aerobronchi - bottom (move close to border)
        LocationItem aero;
        aero.sepia = &game.resources.locationAerobronchiSepia;
        aero.color = &game.resources.locationAerobronchiColored;
        aero.mapRelativePos = { 0.f, 0.48f };
        aero.name = "Aerobronchi";
        out.push_back(std::move(aero));

        // Cladrenal - left (move close to border)
        LocationItem cla;
        cla.sepia = &game.resources.locationCladrenalSepia;
        cla.color = &game.resources.locationCladrenalColored;
        cla.mapRelativePos = { -0.48f, 0.f };
        cla.name = "Cladrenal";
        out.push_back(std::move(cla));

        return out;
    }
}

void drawMapSelectionUI(Game& game, sf::RenderTarget& target) {
    // Draw the map background centered above the name/text boxes
    const sf::Texture& mapTex = game.resources.mapBackground;
    sf::Sprite mapSprite(mapTex);

    // Determine desired map width (use ~60% of window width) and keep aspect
    float winW = static_cast<float>(game.window.getSize().x);
    float winH = static_cast<float>(game.window.getSize().y);
    // Make the map 30% smaller than before and shrink another ~5% for testing
    float desiredWidth = winW * 0.6f * 0.7f * 0.95f; // ~0.399 of window width
    auto texSize = mapTex.getSize();
    float mapScale = desiredWidth / static_cast<float>(texSize.x);
    mapSprite.setScale({ mapScale, mapScale });

    auto localBounds = mapSprite.getLocalBounds();
    mapSprite.setOrigin({ localBounds.position.x + localBounds.size.x / 2.f, localBounds.position.y + localBounds.size.y / 2.f });

    // Position map so its top aligns with the top border (y = 0)
    float mapWidthPx = static_cast<float>(texSize.x) * mapScale;
    float mapHeightPx = static_cast<float>(texSize.y) * mapScale;
    float mapX = winW / 2.f;
    float mapY = mapHeightPx / 2.f; // top will be at 0
    mapSprite.setPosition({ mapX, mapY });

    // Draw map
    target.draw(mapSprite);

    // Build location items
    static std::vector<LocationItem> locationsCache;
    if (locationsCache.empty())
        locationsCache = buildLocations(game);

    // mouse pos in world coords
    auto mousePos = game.window.mapPixelToCoords(sf::Mouse::getPosition(game.window));

    for (auto& loc : locationsCache) {
        // place sprite relative to map center
        // compute map inner size after scale
        sf::FloatRect mapBounds = mapSprite.getGlobalBounds();
        // compute positions relative to the map bounds (use normalized [0..1])
        float marginRatio = 0.10f; // 10% inside the edges (closer to center)
        float fracX = 0.5f;
        float fracY = 0.5f;
        if (loc.name == "Gonad") {
            fracX = 0.5f; fracY = 0.5f;
        }
        else if (loc.name == "Lacrimere") {
            fracX = 0.5f; fracY = marginRatio;
        }
        else if (loc.name == "Blyathyroid") {
            fracX = 1.f - marginRatio; fracY = 0.5f;
        }
        else if (loc.name == "Aerobronchi") {
            fracX = 0.5f; fracY = 1.f - marginRatio;
        }
        else if (loc.name == "Cladrenal") {
            fracX = marginRatio; fracY = 0.5f;
        }

        float posX = mapBounds.position.x + (fracX * mapBounds.size.x);
        float posY = mapBounds.position.y + (fracY * mapBounds.size.y);

        // create a local sprite for this location (sprite requires a texture at construction)
        sf::Sprite sprite( loc.sepia ? *loc.sepia : game.resources.locationGonadSepia );

        // scale location sprite to a reasonable size relative to map
        if (loc.sepia) {
            auto t = loc.sepia->getSize();
            float maxDim = std::min(mapBounds.size.x, mapBounds.size.y) * 0.12f; // 12% of map
            float scale = std::min(maxDim / static_cast<float>(t.x), maxDim / static_cast<float>(t.y));
            // increase size by 30% as requested earlier
            scale *= 1.3f;
            sprite.setScale({ scale, scale });
        }

        // origin and position
        auto lb = sprite.getLocalBounds();
        sprite.setOrigin({ lb.position.x + lb.size.x / 2.f, lb.position.y + lb.size.y / 2.f });
        sprite.setPosition({ posX, posY });

        // hover detection (use sprite global bounds)
        auto g = sprite.getGlobalBounds();
        bool hovered = g.contains(mousePos);
        if (hovered && loc.color) {
            sprite.setTexture(*loc.color);
            // recompute origin/scale in case texture dimensions differ
            if (loc.color) {
                auto t = loc.color->getSize();
                float maxDim = std::min(mapBounds.size.x, mapBounds.size.y) * 0.12f;
                float scale = std::min(maxDim / static_cast<float>(t.x), maxDim / static_cast<float>(t.y));
                // increase size by 30% for hover texture as well
                scale *= 1.3f;
                sprite.setScale({ scale, scale });
                auto lb2 = sprite.getLocalBounds();
                sprite.setOrigin({ lb2.position.x + lb2.size.x / 2.f, lb2.position.y + lb2.size.y / 2.f });
                sprite.setPosition({ posX, posY });
            }
            // update global bounds
            g = sprite.getGlobalBounds();
        }

        // make sepia a bit dimmer
        sf::Color spriteColor = hovered ? sf::Color::White : sf::Color(180, 160, 130);
        spriteColor.a = 255;
        sprite.setColor(spriteColor);

        target.draw(sprite);
    }
}