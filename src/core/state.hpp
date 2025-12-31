#pragma once
// === Header Files ===
#include <SFML/Graphics.hpp>

// Represents the high-level screens or modes that drive game logic.
enum class GameState {
    BattleDemo,
    Credits,
    IntroScreen,
    IntroTitle,
    Dialogue,
    MapSelection,
    WeaponSelection,
    Quiz,
    TreasureChest,
    FinalChoice,
};

class Game;

// Base class for any per-mode state handler that processes events/updates/draws.
class State {
    public:
        explicit State(Game& game) : game(game) {}
        virtual ~State() = default;

        // Called when the current screen receives an SFML event.
        virtual void handleEvent(const sf::Event& event) = 0;
        // Advances the state by dt seconds.
        virtual void update(sf::Time dt) = 0;
        // Draws the current state contents to the window.
        virtual void draw() = 0;
        
    protected:
        Game& game; // Reference to the owning game instance.
};
