// === Header Files ===
#include "locations.hpp"              // Implements Location builder helpers declared in this header.
#include "helper/colorHelper.hpp"     // Supplies palette colors referenced when creating locations.
#include "story/textStyles.hpp"       // Provides speaker colors used as theme tints for certain locations.

namespace Locations {
namespace {
        // Helper to assemble a Location from its components.
        Location makeLocation(
            LocationId id,
            const std::string& name,
            const sf::Color& color,
            sf::Texture& colored,
            sf::Texture& sepia
        ) {
            return Location{
                id,
                name,
                color,
                &colored,
                &sepia
            };
        }

        // Returns the color used for a speaker, reused for locations themed around them.
        sf::Color speakerColor(TextStyles::SpeakerId id) {
            return TextStyles::speakerStyle(id).color;
        }
    }

    // Maps each enum value to a display name, color, and textures.
    std::vector<Location> buildLocations(Resources& resources) {
        std::vector<Location> locations;
        locations.reserve(6);

        locations.push_back(makeLocation(
            LocationId::Gonad,
            "Gonad",
            ColorHelper::Palette::SoftYellow,
            resources.locationGonadColored,
            resources.locationGonadSepia
        ));

        locations.push_back(makeLocation(
            LocationId::Blyathyroid,
            "Blyathyroid",
            speakerColor(TextStyles::SpeakerId::FireDragon),
            resources.locationBlyathyroidColored,
            resources.locationBlyathyroidSepia
        ));

        locations.push_back(makeLocation(
            LocationId::Lacrimere,
            "Lacrimere",
            speakerColor(TextStyles::SpeakerId::WaterDragon),
            resources.locationLacrimereColored,
            resources.locationLacrimereSepia
        ));

        locations.push_back(makeLocation(
            LocationId::Cladrenal,
            "Cladrenal",
            speakerColor(TextStyles::SpeakerId::EarthDragon),
            resources.locationCladrenalColored,
            resources.locationCladrenalSepia
        ));

        locations.push_back(makeLocation(
            LocationId::Aerobronchi,
            "Aerobronchi",
            speakerColor(TextStyles::SpeakerId::AirDragon),
            resources.locationAerobronchiColored,
            resources.locationAerobronchiSepia
        ));

        locations.push_back(makeLocation(
            LocationId::Seminiferous,
            "Seminiferous",
            ColorHelper::Palette::DarkPurple,
            resources.locationSeminiferousColored,
            resources.locationSeminiferousSepia
        ));

        return locations;
    }

    // Finds a location by id or returns nullptr if not present.
    const Location* findById(const std::vector<Location>& locations, LocationId id) {
        for (const auto& location : locations) {
            if (location.id == id)
                return &location;
        }
        return nullptr;
    }
}
