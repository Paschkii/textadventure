#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
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
        , bool showItemBox
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
    void drawBoxHeader(
        Game& game
        , sf::RenderTarget& target
        , const sf::RectangleShape& box
        , const std::string& label
        , float uiAlphaFactor
    );
    void drawItemIcons(
        Game& game
        , sf::RenderTarget& target
        , float uiAlphaFactor
    );
}
