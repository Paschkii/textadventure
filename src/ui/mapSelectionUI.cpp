// === C++ Libraries ===
#include <cmath>       // Uses trigonometric helpers when placing location icons on the map.
#include <cctype>      // Applies std::tolower when normalizing location names for lookups.
#include <algorithm>   // Uses std::clamp/max/min while sizing and positioning map elements.
#include <limits>
#include <optional>    // Returns optional LocationId values for keyboard/mouse handling.
#include <SFML/Graphics/Image.hpp>    // Loads texture pixels when deriving overlay hitboxes.
#include <SFML/Window/Mouse.hpp>     // Reads mouse coordinates inside the map area.
#include <SFML/Window/Keyboard.hpp>  // Processes direct key travel shortcuts.
// === Header Files ===
#include "mapSelectionUI.hpp"        // Declares the map UI functions defined here.
#include "quizUI.hpp"                // Starts quizzes when a dragon destination is selected.
#include "core/game.hpp"             // Accesses map/dialogue state, sprite bounds, and resources.
#include "story/textStyles.hpp"      // Formats dragon names and colors shown in location popups.
#include "story/storyIntro.hpp"      // Supplies dragon dialogue references used by the UI.
#include "story/quests.hpp"          // Reads quest metadata to finish quests once destinations are chosen.
#include "helper/colorHelper.hpp"    // Applies color palettes to map labels and node outlines.
#include "helper/textColorHelper.hpp"// Draws highlighted text segments inside popups.
#include "rendering/textLayout.hpp"  // Wraps multi-line descriptions shown near the map.

namespace {
    constexpr std::size_t kLocationCount = 5;

    std::optional<sf::FloatRect> computeNormalizedContentBounds(const sf::Texture* texture) {
        if (!texture)
            return std::nullopt;
        sf::Image image = texture->copyToImage();
        auto size = image.getSize();
        if (size.x == 0 || size.y == 0)
            return std::nullopt;

        unsigned int minX = size.x;
        unsigned int minY = size.y;
        unsigned int maxX = 0;
        unsigned int maxY = 0;
        bool found = false;

        for (unsigned int y = 0; y < size.y; ++y) {
            for (unsigned int x = 0; x < size.x; ++x) {
                if (image.getPixel({ x, y }).a <= 16)
                    continue;
                found = true;
                minX = std::min(minX, x);
                minY = std::min(minY, y);
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
            }
        }

        if (!found)
            return std::nullopt;

        float width = static_cast<float>(std::max(1u, maxX - minX + 1));
        float height = static_cast<float>(std::max(1u, maxY - minY + 1));

        return sf::FloatRect{
            { static_cast<float>(minX) / static_cast<float>(size.x), static_cast<float>(minY) / static_cast<float>(size.y) },
            { width / static_cast<float>(size.x), height / static_cast<float>(size.y) }
        };
    }

    struct LocationItem {
        const sf::Texture* overlay = nullptr;
        std::optional<LocationId> id;
        sf::Vector2f mapRelativePos; // x,y in [-0.5..0.5] relative to map center
        std::string name;
        std::optional<sf::FloatRect> normalizedContentBounds;
    };

    // Build the five locations with relative positions on the map
    std::vector<LocationItem> buildLocations(const Game& game) {
        std::vector<LocationItem> out;

        // Gonad - center
        LocationItem gonad;
        gonad.overlay = &game.resources.menuMapGonad;
        gonad.id = LocationId::Gonad;
        gonad.mapRelativePos = { 0.f, 0.f };
        gonad.name = "Gonad";
        gonad.normalizedContentBounds = computeNormalizedContentBounds(gonad.overlay);
        out.push_back(std::move(gonad));

        // Lacrimere - top (move close to border, keep 2% margin)
        LocationItem lac;
        lac.overlay = &game.resources.menuMapLacrimere;
        lac.id = LocationId::Lacrimere;
        lac.mapRelativePos = { 0.f, -0.48f };
        lac.name = "Lacrimere";
        lac.normalizedContentBounds = computeNormalizedContentBounds(lac.overlay);
        out.push_back(std::move(lac));

        // Blyathyroid - right (move close to border)
        LocationItem bly;
        bly.overlay = &game.resources.menuMapBlyathyroid;
        bly.id = LocationId::Blyathyroid;
        bly.mapRelativePos = { 0.48f, 0.f };
        bly.name = "Blyathyroid";
        bly.normalizedContentBounds = computeNormalizedContentBounds(bly.overlay);
        out.push_back(std::move(bly));

        // Aerobronchi - bottom (move close to border)
        LocationItem aero;
        aero.overlay = &game.resources.menuMapAerobronchi;
        aero.id = LocationId::Aerobronchi;
        aero.mapRelativePos = { 0.f, 0.48f };
        aero.name = "Aerobronchi";
        aero.normalizedContentBounds = computeNormalizedContentBounds(aero.overlay);
        out.push_back(std::move(aero));

        // Cladrenal - left (move close to border)
        LocationItem cla;
        cla.overlay = &game.resources.menuMapCladrenal;
        cla.id = LocationId::Cladrenal;
        cla.mapRelativePos = { -0.48f, 0.f };
        cla.name = "Cladrenal";
        cla.normalizedContentBounds = computeNormalizedContentBounds(cla.overlay);
        out.push_back(std::move(cla));

        LocationItem petri;
        petri.overlay = &game.resources.menuMapPetrigonal;
        petri.mapRelativePos = { 0.f, 0.1f };
        petri.name = "Petrigonal";
        petri.normalizedContentBounds = computeNormalizedContentBounds(petri.overlay);
        out.push_back(std::move(petri));

        return out;
    }

    void drawLocationPopup(
        Game& game,
        sf::RenderTarget& target,
        const MapPopupRenderData& popup
    );

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
        auto onConfirm = [id](Game& confirmed) {
            if (confirmed.forcedDestinationSelection) {
                if (auto quest = Story::questNamed("Dragonbound Destinations"))
                    confirmed.completeQuest(*quest);
                confirmed.exitForcedDestinationSelection();
            }
            confirmed.beginTeleport(id);
        };
        showConfirmationPrompt(
            game,
            message,
            onConfirm,
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

    std::optional<LocationId> best;
    float bestArea = std::numeric_limits<float>::infinity();
    for (std::size_t i = 0; i < ids.size(); ++i) {
        const auto& rect = game.mapLocationHitboxes[i];
        if (rect.size.x <= 0.f || rect.size.y <= 0.f)
            continue;
        if (!rect.contains(pt))
            continue;

        LocationId id = ids[i];

        float area = rect.size.x * rect.size.y;
        if (area <= 0.f)
            continue;

        if (area < bestArea) {
            bestArea = area;
            best = id;
        }
    }
    return best;
}

} // namespace

std::optional<MapPopupRenderData> drawMapSelectionUI(Game& game, sf::RenderTarget& target) {
    // Draw the map background centered above the name/text boxes
    const sf::Texture& mapTex = game.resources.menuMapBackground;
    sf::Sprite mapSprite(mapTex);

    // Determine desired map size based on the current view (menu panel)
    sf::Vector2f viewSize = target.getView().getSize();
    float winW = viewSize.x;
    float winH = viewSize.y;
    auto texSize = mapTex.getSize();
    float desiredWidth = winW * 0.95f;
    float desiredHeight = winH * 0.95f;
    float mapScale = std::min(desiredWidth / static_cast<float>(texSize.x), desiredHeight / static_cast<float>(texSize.y));
    mapSprite.setScale({ mapScale, mapScale });

    auto localBounds = mapSprite.getLocalBounds();
    mapSprite.setOrigin({ localBounds.position.x + localBounds.size.x / 2.f, localBounds.position.y + localBounds.size.y / 2.f });

    float mapX = winW * 0.5f;
    float mapY = winH * 0.5f;
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

    std::optional<MapPopupRenderData> pendingPopup;
    game.mouseMapHover.reset();
    std::optional<std::size_t> highlightedOverlayIndex;
    std::optional<std::size_t> bestHoverIndex;
    float bestHoverArea = std::numeric_limits<float>::infinity();

    auto drawOverlaySprite = [&](const sf::Texture* texture, const sf::Color& color) {
        if (!texture)
            return;
        sf::Sprite overlaySprite(*texture);
        overlaySprite.setScale(mapSprite.getScale());
        overlaySprite.setOrigin(mapSprite.getOrigin());
        overlaySprite.setPosition(mapSprite.getPosition());
        overlaySprite.setColor(color);
        target.draw(overlaySprite);
    };

    auto toGlobalRect = [&](const sf::FloatRect& normalized) {
        return sf::FloatRect{
            { mapBounds.position.x + (normalized.position.x * mapBounds.size.x), mapBounds.position.y + (normalized.position.y * mapBounds.size.y) },
            { normalized.size.x * mapBounds.size.x, normalized.size.y * mapBounds.size.y }
        };
    };

    for (std::size_t idx = 0; idx < locationsCache.size(); ++idx) {
        auto& loc = locationsCache[idx];
        auto locIdOpt = loc.id;
        float marginRatio = 0.10f;
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
        else if (loc.name == "Petrigonal") {
            fracX = 0.5f; fracY = 0.6f;
        }

        float posX = mapBounds.position.x + (fracX * mapBounds.size.x);
        float posY = mapBounds.position.y + (fracY * mapBounds.size.y);

        sf::Vector2f markerPos{ posX, posY };
        if (loc.name != "Gonad" && loc.name != "Petrigonal") {
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

        sf::FloatRect regionArea = mapBounds;
        if (loc.normalizedContentBounds)
            regionArea = toGlobalRect(*loc.normalizedContentBounds);

        bool allowedForHover = !game.mapTutorialActive;

        if (locIdOpt)
            game.mapLocationHitboxes[locationIndex(*locIdOpt)] = regionArea;

        bool hoveredByMouse = allowedForHover && regionArea.contains(mousePos);
        if (hoveredByMouse && locIdOpt)
            game.mouseMapHover = locIdOpt;

        bool hoveredByKey = allowedForHover && locIdOpt && game.keyboardMapHover && *game.keyboardMapHover == *locIdOpt;
        bool hovered = hoveredByMouse || hoveredByKey;

        if (!hovered)
            continue;

        float area = regionArea.size.x * regionArea.size.y;
        if (area <= 0.f)
            area = mapBounds.size.x * mapBounds.size.y;

        if (!bestHoverIndex || area < bestHoverArea) {
            bestHoverIndex = idx;
            bestHoverArea = area;
            highlightedOverlayIndex = bestHoverIndex;

            MapPopupRenderData popup{
                loc.name,
                {},
                {},
                {},
                loc.id,
                posX,
                posY,
                regionArea,
                mapBounds,
                winW,
                winH,
                mousePos
            };

            if (loc.name == "Gonad") {
                auto villageElderName = TextStyles::speakerStyle(TextStyles::SpeakerId::VillageElder).name;
                popup.shortDesc = "A sleepy village where your journey begins. " + villageElderName + " the Village Elder helps you search for the Dragon Stones and defeat Master Bates.";
            }
            else if (loc.name == "Lacrimere") {
                auto dragonName = TextStyles::speakerStyle(TextStyles::SpeakerId::WaterDragon).name;
                popup.shortDesc = "This is the home of " + dragonName + " - the Water Dragon.\nHe holds one of the Dragon Stones.";
            }
            else if (loc.name == "Blyathyroid") {
                auto dragonName = TextStyles::speakerStyle(TextStyles::SpeakerId::FireDragon).name;
                popup.shortDesc = "This is the home of " + dragonName + " - the Fire Dragon.\nHe holds one of the Dragon Stones.";
            }
            else if (loc.name == "Aerobronchi") {
                auto dragonName = TextStyles::speakerStyle(TextStyles::SpeakerId::AirDragon).name;
                popup.shortDesc = "This is the home of " + dragonName + " - the Air Dragon.\nHe holds one of the Dragon Stones.";
            }
            else if (loc.name == "Cladrenal") {
                auto dragonName = TextStyles::speakerStyle(TextStyles::SpeakerId::EarthDragon).name;
                popup.shortDesc = "This is the home of " + dragonName + " - the Earth Dragon.\nHe holds one of the Dragon Stones.";
            }
            else if (loc.name == "Petrigonal") {
                popup.shortDesc = "This is where Wanda Rinn found you and picked you up.";
            }

            pendingPopup = popup;
        }
    }

    if (game.mapTutorialHighlight) {
        highlightedOverlayIndex = locationIndex(*game.mapTutorialHighlight);
    }

    for (std::size_t idx = 0; idx < locationsCache.size(); ++idx) {
        if (!locationsCache[idx].overlay)
            continue;
        if (highlightedOverlayIndex && *highlightedOverlayIndex == idx)
            continue;
        drawOverlaySprite(locationsCache[idx].overlay, ColorHelper::applyAlphaFactor(ColorHelper::Palette::Normal, 0.9f));
    }

    if (highlightedOverlayIndex) {
        const auto& picked = locationsCache[*highlightedOverlayIndex];
        drawOverlaySprite(picked.overlay, ColorHelper::darken(ColorHelper::Palette::Normal, 0.18f));
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
        bool visitedLocation = popup.locationId && game.locationCompleted[locationIndex(*popup.locationId)];

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
        float mainTextHeight = measureTextHeight(popupTextSize);
        constexpr float kVisitedLabelSpacing = 6.f;
        float visitedLabelHeight = visitedLocation ? (popupTextSize + kVisitedLabelSpacing) : 0.f;
        float textHeight = mainTextHeight + visitedLabelHeight;

        while (textOffset + textHeight + pad > maxPopupHeight && popupTextSize > minPopupTextSize) {
            --popupTextSize;
            textHeight = measureTextHeight(popupTextSize);
        }

        float popupH = std::clamp(textOffset + textHeight + pad, minPopupH, maxPopupHeight);

        sf::Vector2f screenCenter{ popup.winW * 0.5f, popup.winH * 0.5f };
        sf::Vector2f direction{ screenCenter.x - popup.cursorPos.x, screenCenter.y - popup.cursorPos.y };
        float bias = 0.35f;
        sf::Vector2f anchor{ popup.cursorPos.x + direction.x * bias, popup.cursorPos.y + direction.y * bias };
        float popupX = anchor.x - (popupW * 0.5f);
        float popupY = anchor.y - (popupH * 0.5f);
        popupX = std::clamp(popupX, 8.f, popup.winW - popupW - 8.f);
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
        if (visitedLocation) {
            sf::Text visitedText{ game.resources.uiFont, "(Visited)", popupTextSize };
            visitedText.setFillColor(ColorHelper::Palette::SoftRed);
            auto visitedBounds = visitedText.getLocalBounds();
            float visitedX = popupX + pad;
            float visitedY = textStartY + mainTextHeight + kVisitedLabelSpacing - visitedBounds.position.y;
            visitedText.setPosition({ visitedX, visitedY });
            target.draw(visitedText);
        }
    }
}

void drawMapSelectionPopup(Game& game, sf::RenderTarget& target, const MapPopupRenderData& popup) {
    drawLocationPopup(game, target, popup);
}

void handleMapSelectionEvent(Game& game, const sf::Event& event, const sf::View* viewOverride) {
    if (game.mapTutorialActive)
        return;
    if (!game.mapInteractionUnlocked)
        return;
    auto convertPixel = [&](const sf::Vector2i& pixel) {
        if (viewOverride)
            return game.window.mapPixelToCoords(pixel, *viewOverride);
        return game.window.mapPixelToCoords(pixel);
    };
    if (event.is<sf::Event::MouseMoved>()) {
        auto mousePos = convertPixel(sf::Mouse::getPosition(game.window));
        game.mouseMapHover = locationAtPoint(game, mousePos);
    }
    else if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button != sf::Mouse::Button::Left)
            return;
        sf::Vector2f clickPos = convertPixel(button->position);
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
