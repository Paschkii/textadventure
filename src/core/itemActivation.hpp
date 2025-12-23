#pragma once

#include <string>

#include <SFML/Graphics/Texture.hpp>

#include "core/game.hpp"

namespace core {
namespace itemActivation {

const sf::Texture* textureForItemKey(const Game& game, const std::string& key) noexcept;
bool activateItem(Game& game, const std::string& key);

} // namespace itemActivation
} // namespace core
