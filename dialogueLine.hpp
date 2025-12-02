#pragma once
#include <string>
#include "textStyles.hpp"

struct DialogueLine {
    TextStyles::SpeakerId speaker;
    std::string text;
    bool triggersNameInput = false;
    bool waitForEnter;
};