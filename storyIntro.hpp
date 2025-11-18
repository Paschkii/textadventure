#pragma once
#include <vector>
#include "dialogueLine.hpp"

inline const std::vector<DialogueLine> intro = {
    { Speaker::NoNameNPC, "Welcome to this tiny fictional adventure." },
    { Speaker::NoNameNPC, "I am Tory Tailor, your guide and storyteller." },
    { Speaker::StoryTeller, "I will help you on your journey." },
    { Speaker::StoryTeller, "Before we begin, what is your name?\n", true },
    { Speaker::StoryTeller, "Ahhhh, so your name is {playerName}!" }
};