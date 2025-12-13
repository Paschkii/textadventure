#include "locations.hpp"
#include "helper/colorHelper.hpp"
#include "story/textStyles.hpp"

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

        sf::Color speakerColor(TextStyles::SpeakerId id) {
            return TextStyles::speakerStyle(id).color;
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
            LocationId::MasterBatesStronghold,
            "Seminiferous",
            ColorHelper::Palette::DarkPurple,
            resources.locationSeminiferousColored,
            resources.locationSeminiferousSepia
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
