// === C++ Libraries ===
#include <cmath>       // Uses trigonometric helpers when placing location icons on the map.
#include <cctype>      // Applies std::tolower when normalizing location names for lookups.
#include <algorithm>   // Uses std::clamp/max/min while sizing and positioning map elements.
#include <optional>    // Returns optional LocationId values for keyboard/mouse handling.
// === SFML Libraries ===
#include <SFML/Window/Mouse.hpp>     // Reads mouse coordinates inside the map area.
#include <SFML/Window/Keyboard.hpp>  // Processes direct key travel shortcuts.
// === Header Files ===
#include "mapSelectionUI.hpp"        // Declares the map UI functions defined here.
#include "quizUI.hpp"                // Starts quizzes when a dragon destination is selected.
#include "core/game.hpp"             // Accesses map/dialogue state, sprite bounds, and resources.
#include "story/textStyles.hpp"      // Formats dragon names and colors shown in location popups.
#include "story/storyIntro.hpp"      // Supplies dragon dialogue references used by the UI.
#include "helper/colorHelper.hpp"    // Applies color palettes to map labels and node outlines.
#include "helper/textColorHelper.hpp"// Draws highlighted text segments inside popups.
#include "rendering/textLayout.hpp"  // Wraps multi-line descriptions shown near the map.

namespace {
    constexpr std::size_t kLocationCount = 5;

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

    void drawLocationPopup(
        Game& game,
        sf::RenderTarget& target,
        const MapPopupRenderData& popup
    );

    std::optional<LocationId> locationIdFromName(const std::string& name) {
        if (name == "Gonad") return LocationId::Gonad;
        if (name == "Blyathyroid") return LocationId::Blyathyroid;
        if (name == "Lacrimere") return LocationId::Lacrimere;
        if (name == "Cladrenal") return LocationId::Cladrenal;
        if (name == "Aerobronchi") return LocationId::Aerobronchi;
        return std::nullopt;
    }

    std::size_t locationIndex(LocationId id) {
        switch (id) {
            case LocationId::Gonad: return 0;
            case LocationId::Lacrimere: return 1;
            case LocationId::Blyathyroid: return 2;
            case LocationId::Aerobronchi: return 3;
            case LocationId::Cladrenal: return 4;
            default: return 0;
        }
    }

    std::optional<LocationId> keyToLocation(sf::Keyboard::Scan code) {
        switch (code) {
            case sf::Keyboard::Scan::G: return LocationId::Gonad;
            case sf::Keyboard::Scan::A: return LocationId::Aerobronchi;
            case sf::Keyboard::Scan::C: return LocationId::Cladrenal;
            case sf::Keyboard::Scan::B: return LocationId::Blyathyroid;
            case sf::Keyboard::Scan::L: return LocationId::Lacrimere;
            default: return std::nullopt;
        }
    }

    bool canTravelTo(const Game& game, LocationId id) {
        if (game.locationCompleted[locationIndex(id)])
            return false;
        if (!game.currentLocation)
            return true;
        return game.currentLocation->id != id;
    }

    void promptTravel(Game& game, LocationId id) {
        auto locPtr = Locations::findById(game.locations, id);
        if (!locPtr)
            return;
        std::string message = "Travel to " + locPtr->name + "?";
        auto prevText = game.visibleText;
        auto prevChar = game.charIndex;
        showConfirmationPrompt(
            game,
            message,
            [id](Game& confirmed) { confirmed.beginTeleport(id); },
            [](Game&) {}
        );
        game.visibleText = prevText;
        game.charIndex = prevChar;
    }

    std::optional<LocationId> locationAtPoint(const Game& game, sf::Vector2f pt) {
        const std::array<LocationId, kLocationCount> ids{
            LocationId::Gonad,
            LocationId::Lacrimere,
            LocationId::Blyathyroid,
            LocationId::Aerobronchi,
            LocationId::Cladrenal
        };

        for (std::size_t i = 0; i < ids.size(); ++i) {
            const auto& rect = game.mapLocationHitboxes[i];
            if (rect.size.x <= 0.f || rect.size.y <= 0.f)
                continue;
            if (rect.contains(pt))
                return ids[i];
        }
        return std::nullopt;
    }
}

std::optional<MapPopupRenderData> drawMapSelectionUI(Game& game, sf::RenderTarget& target) {
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

    sf::FloatRect mapBounds = mapSprite.getGlobalBounds();
    sf::Vector2f mapCenter{
        mapBounds.position.x + (mapBounds.size.x * 0.5f),
        mapBounds.position.y + (mapBounds.size.y * 0.5f)
    };

    auto moveToward = [](sf::Vector2f point, sf::Vector2f target, float distance) {
        sf::Vector2f dir{ target.x - point.x, target.y - point.y };
        float len = std::sqrt((dir.x * dir.x) + (dir.y * dir.y));
        if (len <= distance || len == 0.f)
            return target;
        dir.x /= len;
        dir.y /= len;
        return sf::Vector2f{ point.x + (dir.x * distance), point.y + (dir.y * distance) };
    };

    auto drawLocationLabel = [&](const std::string& name, char hotkey, const sf::FloatRect& bounds, bool highlight, bool dimmed) {
        const unsigned int labelSize = 22;
        sf::Color labelColor = ColorHelper::Palette::TitleAccent;
        if (highlight)
            labelColor = ColorHelper::Palette::SoftYellow;
        if (dimmed)
            labelColor = ColorHelper::applyAlphaFactor(labelColor, 0.6f);

        sf::Text label{ game.resources.uiFont, name, labelSize };
        label.setFillColor(labelColor);
        auto b = label.getLocalBounds();
        label.setOrigin({ b.position.x + (b.size.x * 0.5f), b.position.y + b.size.y });

        float baseX = bounds.position.x + (bounds.size.x * 0.5f);
        float baseY = bounds.position.y + bounds.size.y + 20.f;
        label.setPosition({ baseX, baseY });
        target.draw(label);

        // Draw a manual underline under the hotkey character to avoid shifting text.
        std::size_t hotkeyIndex = 0;
        for (std::size_t i = 0; i < name.size(); ++i) {
            if (std::toupper(static_cast<unsigned char>(name[i])) == std::toupper(static_cast<unsigned char>(hotkey))) {
                hotkeyIndex = i;
                break;
            }
        }

        sf::Vector2f firstPos = label.findCharacterPos(static_cast<unsigned>(hotkeyIndex));
        sf::Vector2f nextPos = label.findCharacterPos(static_cast<unsigned>(std::min(name.size(), hotkeyIndex + 1)));
        float underlineStartX = firstPos.x;
        float underlineEndX = nextPos.x;
        if (underlineEndX <= underlineStartX)
            underlineEndX = underlineStartX + b.size.x * 0.08f;

        float underlineY = baseY + 3.f;
        float underlineThickness = 2.f;
        sf::Color underlineColor = sf::Color::White;
        if (dimmed)
            underlineColor.a = static_cast<std::uint8_t>(underlineColor.a * 0.6f);

        sf::RectangleShape underline({ underlineEndX - underlineStartX, underlineThickness });
        underline.setPosition({ underlineStartX, underlineY });
        underline.setFillColor(underlineColor);
        target.draw(underline);
    };

    std::optional<MapPopupRenderData> pendingPopup;
    game.mouseMapHover.reset();

    for (auto& loc : locationsCache) {
        auto locIdOpt = locationIdFromName(loc.name);
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

        sf::Vector2f markerPos{ posX, posY };
        if (loc.name != "Gonad") {
            markerPos = moveToward(markerPos, mapCenter, 30.f);
            if (loc.name == "Lacrimere")
                markerPos.x -= 20.f;
            else if (loc.name == "Aerobronchi")
                markerPos.x += 20.f;
            else if (loc.name == "Blyathyroid")
                markerPos.y -= 20.f;
            else if (loc.name == "Cladrenal")
                markerPos.y += 20.f;
        }

        posX = markerPos.x;
        posY = markerPos.y;

        char hotkeyChar = loc.name.empty()
            ? ' '
            : static_cast<char>(std::toupper(static_cast<unsigned char>(loc.name.front())));
        if (locIdOpt) {
            switch (*locIdOpt) {
                case LocationId::Gonad: hotkeyChar = 'G'; break;
                case LocationId::Lacrimere: hotkeyChar = 'L'; break;
                case LocationId::Blyathyroid: hotkeyChar = 'B'; break;
                case LocationId::Aerobronchi: hotkeyChar = 'A'; break;
                case LocationId::Cladrenal: hotkeyChar = 'C'; break;
                default: break;
            }
        }

        // create a local sprite for this location (sprite requires a texture at construction)
        sf::Sprite sprite( loc.sepia ? *loc.sepia : game.resources.locationGonadSepia );

        // scale location sprite to a reasonable size relative to map
        if (loc.sepia) {
            auto t = loc.sepia->getSize();
            float maxDim = std::min(mapBounds.size.x, mapBounds.size.y) * 0.12f; // 12% of map
            float scale = std::min(maxDim / static_cast<float>(t.x), maxDim / static_cast<float>(t.y));
            // increase size by another 20% on top of the previous 40%
            // (1.4 * 1.2 = 1.68 total => ~+68%) to satisfy "another 20%" request
            scale *= 1.68f;
            sprite.setScale({ scale, scale });
        }

        // origin and position
        auto lb = sprite.getLocalBounds();
        sprite.setOrigin({ lb.position.x + lb.size.x / 2.f, lb.position.y + lb.size.y / 2.f });
        sprite.setPosition({ posX, posY });

        // hover detection (use sprite global bounds)
        auto g = sprite.getGlobalBounds();
        bool isCompleted = locIdOpt && game.locationCompleted[locationIndex(*locIdOpt)];
        bool allowedForHover = !isCompleted;

        if (locIdOpt)
            game.mapLocationHitboxes[locationIndex(*locIdOpt)] = g;

        bool hoveredByMouse = allowedForHover && g.contains(mousePos);
        if (hoveredByMouse && locIdOpt)
            game.mouseMapHover = locIdOpt;

        bool hoveredByKey = allowedForHover && locIdOpt && game.keyboardMapHover && *game.keyboardMapHover == *locIdOpt;
        bool hovered = hoveredByMouse || hoveredByKey;

        if (hovered && loc.color) {
            sprite.setTexture(*loc.color);
            // recompute origin/scale in case texture dimensions differ
            if (loc.color) {
                auto t = loc.color->getSize();
                float maxDim = std::min(mapBounds.size.x, mapBounds.size.y) * 0.12f;
                float scale = std::min(maxDim / static_cast<float>(t.x), maxDim / static_cast<float>(t.y));
                // increase size by another 20% on top of the previous 40%
                // keep hover texture the same total multiplier
                scale *= 1.68f;
                sprite.setScale({ scale, scale });
                auto lb2 = sprite.getLocalBounds();
                sprite.setOrigin({ lb2.position.x + lb2.size.x / 2.f, lb2.position.y + lb2.size.y / 2.f });
                sprite.setPosition({ posX, posY });
            }
            // update global bounds
            g = sprite.getGlobalBounds();
        }

        // draw a subtle drop-shadow under the sprite for contrast
        sf::Sprite shadow = sprite;
        // small offset in pixels; scale with map size a little if needed
        float shadowOffset = std::max(4.f, std::min(mapBounds.size.x, mapBounds.size.y) * 0.005f);
        shadow.move({ shadowOffset, shadowOffset });
        sf::Color shadowColor(0, 0, 0, 110); // semi-transparent black
        shadow.setColor(shadowColor);
        target.draw(shadow);

        // make sepia a bit dimmer for non-hover state
        sf::Color spriteColor = hovered ? ColorHelper::Palette::Normal : ColorHelper::Palette::Sepia;
        spriteColor.a = 255;
        sprite.setColor(spriteColor);

        target.draw(sprite);

        if (isCompleted) {
            sf::Color crossColor = ColorHelper::Palette::SoftRed;
            crossColor.a = 220;
            sf::Vertex lines[4];

            lines[0].position = sf::Vector2f{ g.position.x, g.position.y };
            lines[1].position = sf::Vector2f{ g.position.x + g.size.x, g.position.y + g.size.y };
            lines[2].position = sf::Vector2f{ g.position.x + g.size.x, g.position.y };
            lines[3].position = sf::Vector2f{ g.position.x, g.position.y + g.size.y };

            for (auto& v : lines) v.color = crossColor;

            target.draw(lines, 2, sf::PrimitiveType::Lines);
            target.draw(lines + 2, 2, sf::PrimitiveType::Lines);
        }

        drawLocationLabel(loc.name, hotkeyChar, g, hovered, isCompleted);

        // Hover popup: show small info box with title, generated short description
        if (hovered) {
            std::string title = loc.name;
            std::string shortDesc;
            std::string residentTitle;
            std::string residentDesc;

            if (loc.name == "Gonad") {
                auto villageElderName = TextStyles::speakerStyle(TextStyles::SpeakerId::VillageElder).name;
                shortDesc = "A sleepy village where your journey begins. " + villageElderName + " the Village Elder helps you search for the Dragon Stones and defeat Master Bates.";
            }
            else if (loc.name == "Lacrimere") {
                auto dragonName = TextStyles::speakerStyle(TextStyles::SpeakerId::WaterDragon).name;
                shortDesc = "This is the home of " + dragonName + " - the Water Dragon.\nHe holds one of the Dragon Stones.";
            }
            else if (loc.name == "Blyathyroid") {
                auto dragonName = TextStyles::speakerStyle(TextStyles::SpeakerId::FireDragon).name;
                shortDesc = "This is the home of " + dragonName + " - the Fire Dragon.\nHe holds one of the Dragon Stones.";
            }
            else if (loc.name == "Aerobronchi") {
                auto dragonName = TextStyles::speakerStyle(TextStyles::SpeakerId::AirDragon).name;
                shortDesc = "This is the home of " + dragonName + " - the Air Dragon.\nHe holds one of the Dragon Stones.";
            }
            else if (loc.name == "Cladrenal") {
                auto dragonName = TextStyles::speakerStyle(TextStyles::SpeakerId::EarthDragon).name;
                shortDesc = "This is the home of " + dragonName + " - the Earth Dragon.\nHe holds one of the Dragon Stones.";
            }

            pendingPopup = MapPopupRenderData{
                title,
                shortDesc,
                residentTitle,
                residentDesc,
                posX,
                posY,
                g,
                mapBounds,
                winW,
                winH
            };
        }
    }

    return pendingPopup;
}

namespace {
    void drawLocationPopup(
        Game& game,
        sf::RenderTarget& target,
        const MapPopupRenderData& popup
    ) {
        float popupW = std::min(380.f, popup.mapBounds.size.x * 0.28f);
        const float minPopupH = 160.f;
        const unsigned int minPopupTextSize = 12;
        const unsigned int maxPopupTextSize = 16;
        float pad = 12.f;
        unsigned int popupTextSize = maxPopupTextSize;

        sf::Text titleText{ game.resources.uiFont, popup.title, 20 };
        titleText.setFillColor(ColorHelper::Palette::TitleAccent);
        float titleBlockHeight = titleText.getLocalBounds().size.y;

        std::string combined = popup.shortDesc + "\n";
        if (!popup.residentTitle.empty()) {
            combined += popup.residentTitle + ":\n" + popup.residentDesc;
        }

        auto segments = buildColoredSegments(combined);
        float maxTextW = popupW - (pad * 2.f);

        auto layoutPopupText = [&](sf::Vector2f startPos, unsigned int charSize, float maxW, bool draw) {
            if (segments.empty())
                return 0.f;

            const float baseLineStartX = startPos.x;
            const float wrapLimit = baseLineStartX + std::max(0.f, maxW);
            sf::Vector2f cursor = startPos;
            sf::Text metrics(game.resources.uiFont, sf::String(), charSize);
            metrics.setString("Hg");
            float lineSpacing = metrics.getLineSpacing();
            const float lineAdvance = lineSpacing * 10.0f;
            float maxY = cursor.y;

            for (const auto& segment : segments) {
                if (segment.text.empty())
                    continue;

                std::size_t offset = 0;
                while (offset <= segment.text.size()) {
                    std::size_t newlinePos = segment.text.find('\n', offset);
                    std::string part = (newlinePos == std::string::npos)
                        ? segment.text.substr(offset)
                        : segment.text.substr(offset, newlinePos - offset);

                    if (!part.empty()) {
                        sf::Text drawable(game.resources.uiFont, sf::String(), charSize);
                        sf::Color drawableColor = segment.color;
                        drawableColor.a = segment.color.a;
                        drawable.setFillColor(drawableColor);

                        std::size_t partIndex = 0;
                        while (partIndex < part.size()) {
                            bool isSpace = std::isspace(static_cast<unsigned char>(part[partIndex]));
                            std::string token;

                            while (partIndex < part.size()) {
                                char c = part[partIndex];
                                bool currentIsSpace = std::isspace(static_cast<unsigned char>(c));
                                if (currentIsSpace != isSpace)
                                    break;
                                token.push_back(c);
                                ++partIndex;
                            }

                            if (token.empty())
                                continue;

                            if (isSpace) {
                                drawable.setString(token);
                                float tokenWidth = drawable.getLocalBounds().size.x;

                                if (cursor.x == baseLineStartX)
                                    continue;

                                if (cursor.x + tokenWidth > wrapLimit) {
                                    cursor.x = baseLineStartX;
                                    cursor.y += lineAdvance;
                                    maxY = std::max(maxY, cursor.y);
                                    continue;
                                }

                                if (draw) {
                                    drawable.setPosition(cursor);
                                    target.draw(drawable);
                                }
                                cursor.x += tokenWidth;
                                maxY = std::max(maxY, cursor.y);
                            } else {
                                auto drawSplitToken = [&](const std::string& word) {
                                    std::string currentChunk;

                                    for (char c : word) {
                                        std::string nextChunk = currentChunk + c;
                                        drawable.setString(nextChunk);
                                        float chunkWidth = drawable.getLocalBounds().size.x;

                                        float availableWidth = wrapLimit - cursor.x;
                                        if (availableWidth <= 0.f) {
                                            cursor.x = baseLineStartX;
                                            cursor.y += lineAdvance;
                                            maxY = std::max(maxY, cursor.y);
                                            availableWidth = wrapLimit - cursor.x;
                                        }

                                        if (chunkWidth > availableWidth && !currentChunk.empty()) {
                                            drawable.setString(currentChunk);
                                            if (draw) {
                                                drawable.setPosition(cursor);
                                                target.draw(drawable);
                                            }
                                            cursor.x += drawable.getLocalBounds().size.x;
                                            cursor.x = baseLineStartX;
                                            cursor.y += lineAdvance;
                                            maxY = std::max(maxY, cursor.y);
                                            currentChunk.clear();
                                            drawable.setString(nextChunk = std::string(1, c));
                                            chunkWidth = drawable.getLocalBounds().size.x;
                                        }

                                        if (cursor.x + chunkWidth > wrapLimit && currentChunk.empty()) {
                                            drawable.setPosition(cursor);
                                            if (draw)
                                                target.draw(drawable);
                                            cursor.x += chunkWidth;
                                            maxY = std::max(maxY, cursor.y);
                                            continue;
                                        }

                                        currentChunk = nextChunk;
                                    }

                                    if (!currentChunk.empty()) {
                                        drawable.setString(currentChunk);
                                        if (draw) {
                                            drawable.setPosition(cursor);
                                            target.draw(drawable);
                                        }
                                        cursor.x += drawable.getLocalBounds().size.x;
                                        maxY = std::max(maxY, cursor.y);
                                    }
                                };

                                drawable.setString(token);
                                float tokenWidth = drawable.getLocalBounds().size.x;

                                float availableWidth = wrapLimit - cursor.x;
                                if (availableWidth <= 0.f) {
                                    cursor.x = baseLineStartX;
                                    cursor.y += lineAdvance;
                                    maxY = std::max(maxY, cursor.y);
                                    availableWidth = wrapLimit - cursor.x;
                                }

                                if (tokenWidth <= availableWidth) {
                                    if (draw) {
                                        drawable.setPosition(cursor);
                                        target.draw(drawable);
                                    }
                                    cursor.x += tokenWidth;
                                } else if (tokenWidth <= maxW) {
                                    cursor.x = baseLineStartX;
                                    cursor.y += lineAdvance;
                                    maxY = std::max(maxY, cursor.y);
                                    drawable.setPosition(cursor);
                                    if (draw)
                                        target.draw(drawable);
                                    cursor.x += tokenWidth;
                                } else {
                                    drawSplitToken(token);
                                }

                                maxY = std::max(maxY, cursor.y);
                            }
                        }
                    }

                    if (newlinePos == std::string::npos)
                        break;

                    offset = newlinePos + 1;
                    cursor.x = baseLineStartX;
                    cursor.y += lineAdvance;
                    maxY = std::max(maxY, cursor.y);
                }
            }

            return (maxY - startPos.y) + metrics.getLineSpacing();
        };

        auto measureTextHeight = [&](unsigned int charSize) {
            return layoutPopupText({ 0.f, 0.f }, charSize, maxTextW, false);
        };

        const float dividerThickness = 1.f;
        const float dividerSpacing = 14.f;
        const float textTopSpacing = 10.f;
        float textOffset = pad + titleBlockHeight + dividerThickness + dividerSpacing + textTopSpacing;
        float maxPopupHeight = std::max(minPopupH, popup.winH - 16.f);
        float textHeight = measureTextHeight(popupTextSize);

        while (textOffset + textHeight + pad > maxPopupHeight && popupTextSize > minPopupTextSize) {
            --popupTextSize;
            textHeight = measureTextHeight(popupTextSize);
        }

        float popupH = std::clamp(textOffset + textHeight + pad, minPopupH, maxPopupHeight);

        float popupX = popup.posX + (popup.iconBounds.size.x / 2.f) + 8.f;
        if (popupX + popupW > popup.mapBounds.position.x + popup.mapBounds.size.x)
            popupX = popup.posX - (popup.iconBounds.size.x / 2.f) - 8.f - popupW;
        float popupY = popup.posY - popupH / 2.f;
        popupY = std::clamp(popupY, 8.f, popup.winH - popupH - 8.f);

        sf::RectangleShape shadowRect({ popupW, popupH });
        shadowRect.setPosition({ popupX + 6.f, popupY + 6.f });
        shadowRect.setFillColor(ColorHelper::Palette::Shadow120);
        target.draw(shadowRect);

        sf::RectangleShape bgRect({ popupW, popupH });
        bgRect.setPosition({ popupX, popupY });
        bgRect.setFillColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, 0.96f));
        bgRect.setOutlineThickness(1.5f);
        bgRect.setOutlineColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, 0.9f));
        target.draw(bgRect);

        titleText.setPosition({ popupX + pad, popupY + pad });
        target.draw(titleText);

        float dividerY = popupY + pad + titleBlockHeight + dividerSpacing;
        sf::RectangleShape divider({ popupW - (pad * 2.f), dividerThickness });
        divider.setPosition({ popupX + pad, dividerY });
        divider.setFillColor(ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, 0.85f));
        target.draw(divider);

        float textStartY = dividerY + dividerThickness + textTopSpacing;
        layoutPopupText({ popupX + pad, textStartY }, popupTextSize, maxTextW, true);
    }
}

void drawMapSelectionPopup(Game& game, sf::RenderTarget& target, const MapPopupRenderData& popup) {
    drawLocationPopup(game, target, popup);
}

void handleMapSelectionEvent(Game& game, const sf::Event& event) {
    if (event.is<sf::Event::MouseMoved>()) {
        auto mousePos = game.window.mapPixelToCoords(sf::Mouse::getPosition(game.window));
        game.mouseMapHover = locationAtPoint(game, mousePos);
    }
    else if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button != sf::Mouse::Button::Left)
            return;
        sf::Vector2f clickPos = game.window.mapPixelToCoords(button->position);
        auto target = locationAtPoint(game, clickPos);
        if (target && canTravelTo(game, *target))
            promptTravel(game, *target);
    }
    else if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        if (auto locFromKey = keyToLocation(key->scancode)) {
            if (game.keyboardMapHover && *game.keyboardMapHover == *locFromKey)
                game.keyboardMapHover.reset();
            else
                game.keyboardMapHover = locFromKey;
            return;
        }

        if (key->scancode == sf::Keyboard::Scan::Enter || key->scancode == sf::Keyboard::Scan::Enter) {
            std::optional<LocationId> target = game.keyboardMapHover;
            if (!target)
                target = game.mouseMapHover;

            if (target && canTravelTo(game, *target))
                promptTravel(game, *target);
        }
    }
}
