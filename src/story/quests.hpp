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
        125,
        std::optional<std::string>("Reforged Weapon"),
    },
    {
        "Dragonbound Destinations",
        "Noah Lott",
        "Open the map and choose one of the Elemental Dragon regions to journey to first.",
        150,
        std::nullopt,
    }
};

inline const QuestDefinition* questNamed(const std::string& name) {
    for (const auto& quest : kQuestDefinitions) {
        if (quest.name == name)
            return &quest;
    }
    return nullptr;
}

} // namespace Story
