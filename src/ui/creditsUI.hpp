#pragma once
// === C++ Libraries ===
#include <cstddef>
#include <string>
#include <vector>

// === SFML Libraries ===
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>

class Game;

namespace ui::credits {

enum class Category {
    Acts,
    Dragons,
    Specials
};

struct Entry {
    const sf::Texture* texture = nullptr;
    std::string line1;
    std::string line2;
    Category category = Category::Acts;
};

struct State {
    enum class Phase {
        TitleFadeIn,
        PanelFadeIn,
        EntryDisplay,
        PanelFadeOut,
        BetaDisplay,
        Complete
    };

    bool active = false;
    bool initialized = false;
    Phase phase = Phase::TitleFadeIn;
    std::size_t entryIndex = 0;
    float titleAlpha = 0.f;
    float panelAlpha = 0.f;
    float entryAlpha = 0.f;
    float betaAlpha = 0.f;
    sf::Clock phaseClock;
    std::vector<Entry> entries;
    const sf::Texture* betaTexture = nullptr;
};

void start(Game& game);
void update(Game& game);
void draw(Game& game, sf::RenderTarget& target);

} // namespace ui::credits
