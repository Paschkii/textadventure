#pragma once
#include <SFML/Graphics.hpp>

enum class GameState {
    IntroScreen,
    IntroTitle,
    Dialogue,
    MapSelection,
    WeaponSelection,
    Quiz,
    FinalChoice,
};

class Game;

class State {
    public:
        explicit State(Game& game) : game(game) {}
        virtual ~State() = default;

        virtual void handleEvent(const sf::Event& event) = 0;
        virtual void update(sf::Time dt) = 0;
        virtual void draw() = 0;
        
    protected:
        Game& game;
};
