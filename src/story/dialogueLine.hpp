#pragma once
// === C++ Libraries ===
#include <cstdint>              // Defines fixed-size integers for bit masks.
#include <optional>             // Stores quest names when lines trigger quests.
#include <string>               // Stores the text of each dialogue entry.
#include <type_traits>          // Provides std::underlying_type used by flag helpers.
// === Header Files ===
#include "textStyles.hpp"        // References speaker IDs/colors for the line metadata.

// Describes extra actions that can fire after a dialogue line completes.
enum class DialogueLineAction : std::uint32_t {
    None = 0,
    OpensMapFromMenu = 1u << 0,
    StartsQuest = 1u << 1,
    CompletesQuest = 1u << 2,
    StartsQuiz = 1u << 3,
    OpensUmbraMapFromMenu = 1u << 4,
    StartsSeminiferousTeleport = 1u << 5,
    FinalCheer = 1u << 6,
    StartsBattle = 1u << 7,
};

// Provides bitwise helpers so flag combinations can be stored in a single field.
inline DialogueLineAction operator|(DialogueLineAction lhs, DialogueLineAction rhs) {
    using Underlying = std::underlying_type_t<DialogueLineAction>;
    return static_cast<DialogueLineAction>(
        static_cast<Underlying>(lhs) | static_cast<Underlying>(rhs)
    );
}

inline DialogueLineAction& operator|=(DialogueLineAction& lhs, DialogueLineAction rhs) {
    lhs = lhs | rhs;
    return lhs;
}

inline bool dialogueLineHasAction(DialogueLineAction flags, DialogueLineAction flag) {
    using Underlying = std::underlying_type_t<DialogueLineAction>;
    return (static_cast<Underlying>(flags) & static_cast<Underlying>(flag)) != 0;
}

// Represents one line of dialogue along with speaker metadata and optional behaviors.
struct DialogueLine {
    TextStyles::SpeakerId speaker;
    std::string text;
    bool triggersNameInput = false; // Prompt for player name when true.
    bool triggersGenderSelection = false; // Launches the dragonborn choice UI when true.
    bool waitForEnter = true; // Indicates whether Enter advances this line.
    DialogueLineAction actions = DialogueLineAction::None; // Triggers mapped behaviors after the line completes.
    std::optional<std::string> questStart; // Optional quest that should start when this line finishes.
    std::optional<std::string> questComplete; // Optional quest that should complete when this line finishes.
};
