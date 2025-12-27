#pragma once
// === C++ Libraries ===
#include <optional>    // Stores optional loot descriptions for quests.
#include <string>      // Holds quest name/giver/goal/loot text.
#include <vector>      // Aggregates the quest definitions.

namespace Story {

struct QuestDefinition {
    std::string name;
    std::string giver;
    std::string goal;
    int xpReward = 0;
    std::optional<std::string> loot;
};

inline const std::vector<QuestDefinition> kQuestDefinitions = {
    {
        "Forged Destiny",
        "Wanda Rinn",
        "Find your shattered weapons and convince Will Figsid to forge a\nnew blade.",
        250,
        std::optional<std::string>("Reforged Weapon"),
    },
    {
        "Map of Many Lands",
        "Noah Lott",
        "Speak with the Village Elder in Gonad and obtain the map that reveals the four dragon regions.",
        250,
        std::nullopt
    },
    {
        "Trial of Resolve",
        "Rowsted Sheacane",
        "Reach Blyathyroid and endure Rowsted Sheacane's trial of resolve in the flames.",
        1500,
        std::nullopt
    },
    {
        "Trial of Mind",
        "Flawtin Seamen",
        "Reach Lacrimere and clear Flawtin Seamen's trial of the mind in the cold depths.",
        1500,
        std::nullopt
    },
    {
        "Trial of Body",
        "Grounded Claymore",
        "Reach Cladrenal and withstand Grounded Claymore's trial of the body in the mud and stone.",
        1500,
        std::nullopt
    },
    {
        "Trial of Soul",
        "Gustavo Windimaess",
        "Reach Aerobronchi and complete Gustavo Windimaess's trial of the soul among the open skies.",
        1500,
        std::nullopt
    },
    {
        "Fragments of Home",
        "Noah Lott",
        "Collect all four map fragments from the elemental dragons and return to Gonad.",
        2000,
        std::nullopt
    },
    {
        "Face of the Master",
        "Tory Tailor",
        "Enter the dark castle in Seminiferous and confront Master Bates at its heart.",
        2500,
        std::nullopt
    },
};

inline const QuestDefinition* questNamed(const std::string& name) {
    for (const auto& quest : kQuestDefinitions) {
        if (quest.name == name)
            return &quest;
    }
    return nullptr;
}

} // namespace Story
