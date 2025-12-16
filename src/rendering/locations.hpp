#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <vector>
#include <string>
#include "resources/resources.hpp"

enum class LocationId {
    Gonad,
    Blyathyroid,
    Lacrimere,
    Cladrenal,
    Aerobronchi,
    Seminiferous,
};

struct Location {
    LocationId id;
    std::string name;
    sf::Color color;
    sf::Texture* coloredTexture = nullptr;
    sf::Texture* sepiaTexture = nullptr;
};

namespace Locations {
    std::vector<Location> buildLocations(Resources& resources);
    const Location* findById(const std::vector<Location>& locations, LocationId id);
}