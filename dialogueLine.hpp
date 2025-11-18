#pragma once
#include <string>
#include "speaker.hpp"

struct DialogueLine {
    Speaker speaker;
    std::string text;
    bool triggersNameInput = false;
    bool waitForEnter;
};