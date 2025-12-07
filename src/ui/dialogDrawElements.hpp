#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <string>

class Game;

namespace TextStyles {
    struct SpeakerStyle;
}

namespace dialogDraw {
    void drawDialogueFrames(
        Game& game
        , sf::RenderTarget& target
        , float uiAlphaFactor
        , float glowElapsedSeconds
        , bool showLocationBox
    );
    void drawSpeakerName(
        sf::RenderTarget& target
        , Game& game
        , const TextStyles::SpeakerStyle& info
        , float uiAlphaFactor
    );
    void drawDialogueText(
        sf::RenderTarget& target
        , Game& game
        , const std::string& textToDraw
        , float uiAlphaFactor
    );
    void drawNameInput(
        sf::RenderTarget& target
        , Game& game
        , float uiAlphaFactor
    );
    void drawReturnPrompt(
        sf::RenderTarget& target
        , Game & game
        , float uiAlphaFactor
        , bool isTyping
    );
}