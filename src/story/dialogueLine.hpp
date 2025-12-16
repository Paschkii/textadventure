#pragma once
// === C++ Libraries ===
#include <string>               // Stores the text of each dialogue entry.
// === Header Files ===
#include "textStyles.hpp"        // References speaker IDs/colors for the line metadata.

// Represents one line of dialogue along with speaker metadata and optional behaviors.
struct DialogueLine {
    TextStyles::SpeakerId speaker;
    std::string text;
    bool triggersNameInput = false; // Prompt for player name when true.
    bool triggersGenderSelection = false; // Launches the dragonborn choice UI when true.
    bool waitForEnter = true; // Indicates whether Enter advances this line.
};
