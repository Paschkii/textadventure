#include "items/itemRegistry.hpp"

#include <cctype>
#include <map>
#include <string_view>

namespace {
    using items::Category;
    using items::ItemDefinition;

    inline std::string_view toLower(std::string_view value) {
        static thread_local std::string buffer;
        buffer.clear();
        for (char c : value)
            buffer.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        return buffer;
    }

    const std::array<std::pair<const char*, ItemDefinition>, 33> kDefinitions = {{
        { "heal_potion", ItemDefinition{
            "Healing Potion",
            "Restores a chunk of health over a short span.",
            Category::Consumable,
            { "30 HP recovered over 4 seconds" },
            "Use",
            "Consumables"
        } },
        { "map_glandular", ItemDefinition{
            "Map of Glandular",
            "Shows the waterways and dragon territories around this region.",
            Category::Tool,
            { "Unlocks travel hints", "Triggers Jorge's questline" },
            "Use",
            "Tools"
        } },
        { "dragonstone_fire", ItemDefinition{
            "Fire Dragonstone",
            "The dragonstone infused with fire magic.",
            Category::Artifact,
            { "Temporarily increases attack resistance" },
            "Activate",
            "Dragonstones"
        } },
        { "dragonstone_air", ItemDefinition{
            "Air Dragonstone",
            "Harnesses the breath of the Air Dragon.",
            Category::Artifact,
            { "Increases agility for one encounter" },
            "Activate",
            "Dragonstones"
        } },
        { "dragonstone_water", ItemDefinition{
            "Water Dragonstone",
            "Carries the chill of the Water Dragon.",
            Category::Artifact,
            { "Provides a shielded barrier once" },
            "Activate",
            "Dragonstones"
        } },
        { "dragonstone_earth", ItemDefinition{
            "Earth Dragonstone",
            "Solid earth powers await within this scale.",
            Category::Artifact,
            { "Raises defenses for a short time" },
            "Activate",
            "Dragonstones"
        } },
        { "dragonscale_fire", ItemDefinition{
            "Fire Dragonscale",
            "A charred scale that still hums with cinder energy.",
            Category::Artifact,
            { "Reduces incoming fire damage" },
            "Activate",
            "Dragonscales"
        } },
        { "dragonscale_air", ItemDefinition{
            "Air Dragonscale",
            "Feather-light and shimmering with gusts.",
            Category::Artifact,
            { "Improves evasion for a short time" },
            "Activate",
            "Dragonscales"
        } },
        { "dragonscale_water", ItemDefinition{
            "Water Dragonscale",
            "Cool azure plating that drips with condensation.",
            Category::Artifact,
            { "Grants a refreshing barrier once" },
            "Activate",
            "Dragonscales"
        } },
        { "dragonscale_earth", ItemDefinition{
            "Earth Dragonscale",
            "Dense stone-like plating etched with roots.",
            Category::Artifact,
            { "Massively boosts defense for one hit" },
            "Activate",
            "Dragonscales"
        } },
        { "dragonclaw_fire", ItemDefinition{
            "Fire Dragon Claw",
            "An ember-red claw that crackles when swung.",
            Category::Artifact,
            { "Adds a fire surge to your next attack" },
            "Activate",
            "Dragonclaws"
        } },
        { "dragonclaw_air", ItemDefinition{
            "Air Dragon Claw",
            "A curved claw swirling with turbulent winds.",
            Category::Artifact,
            { "Imbues attacks with gale force" },
            "Activate",
            "Dragonclaws"
        } },
        { "dragonclaw_water", ItemDefinition{
            "Water Dragon Claw",
            "A translucent claw trailing droplets everywhere.",
            Category::Artifact,
            { "Grants a tidal burst that slows enemies" },
            "Activate",
            "Dragonclaws"
        } },
        { "dragonclaw_earth", ItemDefinition{
            "Earth Dragon Claw",
            "A rugged claw rooted in molten stone.",
            Category::Artifact,
            { "Deals a crushing impact that briefly stuns" },
            "Activate",
            "Dragonclaws"
        } },
        { "luckycharm_fire", ItemDefinition{
            "Fire Lucky Charm",
            "A small ember charm sparking with luck.",
            Category::Artifact,
            { "Creates a one-use flame shield" },
            "Activate",
            "Lucky Charms"
        } },
        { "luckycharm_air", ItemDefinition{
            "Air Lucky Charm",
            "A twirling charm that hums with winds.",
            Category::Artifact,
            { "Creates a gusty shield that deflects projectiles" },
            "Activate",
            "Lucky Charms"
        } },
        { "luckycharm_water", ItemDefinition{
            "Water Lucky Charm",
            "A droplet-shaped charm filled with cool glow.",
            Category::Artifact,
            { "Generates a watery shield that soaks one attack" },
            "Activate",
            "Lucky Charms"
        } },
        { "luckycharm_earth", ItemDefinition{
            "Earth Lucky Charm",
            "A stone charm carved with protective sigils.",
            Category::Artifact,
            { "Summons a rocky shield for one hit" },
            "Activate",
            "Lucky Charms"
        } },
        { "ring_fire", ItemDefinition{
            "Fire Ring",
            "Glowing with heat, this band adds ember-hot protection.",
            Category::Equipment,
            { "Boosts resistance against fire for one encounter" },
            "Equip",
            "Rings"
        } },
        { "ring_air", ItemDefinition{
            "Air Ring",
            "A spiraling ring that feels light as a breeze.",
            Category::Equipment,
            { "Improves evasion with a gusty edge" },
            "Equip",
            "Rings"
        } },
        { "ring_water", ItemDefinition{
            "Water Ring",
            "Encased in droplets that never fall.",
            Category::Equipment,
            { "Provides a small aqua shield on next hit" },
            "Equip",
            "Rings"
        } },
        { "ring_earth", ItemDefinition{
            "Earth Ring",
            "Etched with roots and stone fragments.",
            Category::Equipment,
            { "Hardens your skin with earthy steadiness" },
            "Equip",
            "Rings"
        } },
        { "umbra_ussea_one", ItemDefinition{
            "Umbra Ossea Fragment One",
            "First piece of the Umbra Ossea chart.",
            Category::Tool,
            { "Completes part of the regional map" },
            "Inspect",
            "Maps"
        } },
        { "umbra_ussea_two", ItemDefinition{
            "Umbra Ossea Fragment Two",
            "Second piece of the Umbra Ossea chart.",
            Category::Tool,
            { "Completes part of the regional map" },
            "Inspect",
            "Maps"
        } },
        { "umbra_ussea_three", ItemDefinition{
            "Umbra Ossea Fragment Three",
            "Third piece of the Umbra Ossea chart.",
            Category::Tool,
            { "Completes part of the regional map" },
            "Inspect",
            "Maps"
        } },
        { "umbra_ussea_four", ItemDefinition{
            "Umbra Ossea Fragment Four",
            "Fourth piece of the Umbra Ossea chart.",
            Category::Tool,
            { "Completes part of the regional map" },
            "Inspect",
            "Maps"
        } },
        { "umbra_ussea_complete", ItemDefinition{
            "Umbra Ossea Map",
            "All fragments stitched together reveal the complete sea routes.",
            Category::Tool,
            { "Opens the path to Noah Lott's hidden regions" },
            "Inspect",
            "Maps"
        } },
        { "weapon_holmabir_broken", ItemDefinition{
            "Broken Holmabir",
            "The two-handed blade shattered on your last battle.",
            Category::Equipment,
            { "Piece required for reforging", "Can be reforged by Figsid" },
            "Equip",
            "Weapons"
        } },
        { "weapon_kattkavar_broken", ItemDefinition{
            "Broken Kattkavar",
            "The righthand blade that used to swing with a shield.",
            Category::Equipment,
            { "Pair this with Ragnar's shield", "Can be reforged by Figsid" },
            "Equip",
            "Weapons"
        } },
        { "weapon_stiggedin_broken", ItemDefinition{
            "Broken Stiggedin",
            "Twin daggers separated at the hilt.",
            Category::Equipment,
            { "Each dagger must be refitted", "Can be reforged by Figsid" },
            "Equip",
            "Weapons"
        } },
        { "holmabir", ItemDefinition{
            "Holmabir",
            "A massive two-handed blade blunt yet faithful.",
            Category::Equipment,
            { "Crushes armour with sweeping strikes" },
            "Equip",
            "Weapons"
        } },
        { "kattkavar", ItemDefinition{
            "Kattkavar",
            "Twin righthands swinging with shielded might.",
            Category::Equipment,
            { "Pairs with shield +25 defence" },
            "Equip",
            "Weapons"
        } },
        { "stiggedin", ItemDefinition{
            "Stiggedin",
            "Dual daggers for precise and rapid strikes.",
            Category::Equipment,
            { "Bonus crit chance within 2 seconds" },
            "Equip",
            "Weapons"
        } }
    }};

    const std::map<std::string, ItemDefinition> buildDefinitionMap() {
        std::map<std::string, ItemDefinition> result;
        for (const auto& entry : kDefinitions) {
            result.emplace(entry.first, entry.second);
        }
        return result;
    }

    const std::map<std::string, ItemDefinition> kDefinitionMap = buildDefinitionMap();
}

namespace items {
    const ItemDefinition* definitionFor(const std::string& key) noexcept {
        auto lowerKey = toLower(key);
        auto it = kDefinitionMap.find(std::string(lowerKey));
        if (it != kDefinitionMap.end())
            return &it->second;
        return nullptr;
    }

    std::string defaultActionLabel(Category category) noexcept {
        switch (category) {
            case Category::Equipment: return "Equip";
            case Category::Consumable:
            case Category::Tool:
            case Category::Artifact: return "Use";
            case Category::Misc: return "Inspect";
        }
        return "Use";
    }
}
