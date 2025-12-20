#pragma once
// === C++ Libraries ===
#include <limits>      // Supplies std::numeric_limits for trigger defaults.
#include <optional>    // Stores optional loot descriptions for quests.
#include <string>      // Holds quest name/giver/goal/loot text.
#include <vector>      // Aggregates the quest definitions.
// === Header Files ===
#include "dialogueLine.hpp"  // Reuses the DialogueLine pointers as triggers.
#include "storyIntro.hpp"    // References the specific dialogue pools used for triggers.

namespace Story {

struct QuestDefinition {
    std::string name;
    std::string giver;
    std::string goal;
    int xpReward = 0;
    std::optional<std::string> loot;
    const std::vector<DialogueLine>* triggerDialogue = nullptr;
    std::size_t triggerIndex = std::numeric_limits<std::size_t>::max();
    const std::vector<DialogueLine>* completionDialogue = nullptr;
    std::size_t completionIndex = std::numeric_limits<std::size_t>::max();
};

inline const std::vector<QuestDefinition> kQuestDefinitions = {
    {
        "Forged Destiny",
        "Wanda Rinn",
        "Find your shattered weapons and convince Will Figsid to forge a\nnew blade.",
        125,
        std::optional<std::string>("Reforged Weapon"),
        &perigonal,
        31,
        &blacksmith,
        20
    },
    {
        "Dragonbound Destinations",
        "Noah Lott",
        "Open the map and choose one of the Elemental Dragon regions to journey to first.",
        150,
        std::nullopt,
        &gonad_part_two,
        34,
        nullptr,
        std::numeric_limits<std::size_t>::max()
    }
};

inline const QuestDefinition* questForTrigger(const std::vector<DialogueLine>* dialogue, std::size_t index) {
    for (const auto& quest : kQuestDefinitions) {
        if (quest.triggerDialogue == dialogue && quest.triggerIndex == index)
            return &quest;
    }
    return nullptr;
}

inline const QuestDefinition* questForCompletionTrigger(const std::vector<DialogueLine>* dialogue, std::size_t index) {
    for (const auto& quest : kQuestDefinitions) {
        if (quest.completionDialogue == dialogue && quest.completionIndex == index)
            return &quest;
    }
    return nullptr;
}

inline const QuestDefinition* questNamed(const std::string& name) {
    for (const auto& quest : kQuestDefinitions) {
        if (quest.name == name)
            return &quest;
    }
    return nullptr;
}

} // namespace Story
