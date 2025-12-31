#pragma once
// === C++ Libraries ===
#include <vector>  // Holds the list of Location entries returned by the builder.
#include <string>  // Stores display names attached to each location.
// === SFML Libraries ===
#include <SFML/Graphics/Color.hpp>    // Provides sf::Color for tinting backgrounds.
#include <SFML/Graphics/Texture.hpp>  // References texture pointers used for map art.
// === Header Files ===
#include "resources/resources.hpp"  // Supplies textures consumed when constructing Location instances.

// Unique identifier for every in-game location that can be visited.
enum class LocationId {
    Perigonal,
    Gonad,
    FigsidsForge,
    Blyathyroid,
    Lacrimere,
    Cladrenal,
    Aerobronchi,
    Seminiferous,
    UmbraOssea,
};

// Describes a location entry, including display name, tint color, and textures.
struct Location {
    LocationId id;                // Enum value identifying this location.
    std::string name;             // Display name shown in the UI.
    sf::Color color;              // Tinted background color for overlays.
    sf::Texture* coloredTexture = nullptr; // Color version of the map art.
    sf::Texture* sepiaTexture = nullptr;   // Sepia version used for certain transitions.
};

namespace Locations {
    // Builds the static list of locations together with their textures.
    std::vector<Location> buildLocations(Resources& resources);
    // Searches for a location with the requested id and returns a pointer or nullptr.
    const Location* findById(const std::vector<Location>& locations, LocationId id);
}
