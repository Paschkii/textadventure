#include "locations.hpp"
#include "colorHelper.hpp"

namespace Locations {
    namespace {
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
    }

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
            ColorHelper::Palette::Amber,
            resources.locationBlyathyroidColored,
            resources.locationBlyathyroidSepia
        ));

        locations.push_back(makeLocation(
            LocationId::Lacrimere,
            "Lacrimere",
            ColorHelper::Palette::SkyBlue,
            resources.locationLacrimereColored,
            resources.locationLacrimereSepia
        ));

        locations.push_back(makeLocation(
            LocationId::Cladrenal,
            "Cladrenal",
            ColorHelper::Palette::LightBrown,
            resources.locationCladrenalColored,
            resources.locationCladrenalSepia
        ));

        locations.push_back(makeLocation(
            LocationId::Aerobronchi,
            "Aerobronchi",
            ColorHelper::Palette::Mint,
            resources.locationAerobronchiColored,
            resources.locationAerobronchiSepia
        ));

        locations.push_back(makeLocation(
            LocationId::MasterBatesStronghold,
            "Master Bates Stronghold",
            ColorHelper::Palette::Shadow40,
            resources.locationMasterBatesHideoutColored,
            resources.locationMasterBatesHideoutSepia
        ));

        return locations;
    }

    const Location* findById(const std::vector<Location>& locations, LocationId id) {
        for (const auto& location : locations) {
            if (location.id == id)
                return &location;
        }
        return nullptr;
    }
}