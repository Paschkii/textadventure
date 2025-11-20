#include "introScreen.hpp"
#include "game.hpp"
#include <algorithm>

void renderIntroScreen(Game& game) {
    const std::string titleText = "IMPORTANT! PLEASE READ!";
    const std::string bodyText =
        "To navigate through the game, you will mostly only need to press the Enter Key.\n"
        "However, there will be times when you will be asked to make a choice.\n"
        "In those cases, you will be given a list of options to choose from.\n"
        "You can select an option by typing the corresponding key and pressing Enter.\n"
        "This game is just meant for learning purposes.\n"
        "Some of the dialogues may seem a little trashy and I can assure you:\n"
        "It's meant to be like that - haha!\n\n"
        "Thank you for your understanding! And happy gaming!";

    const std::string promptText = "Press Enter to continue.";

    const float windowWidth = static_cast<float>(game.window.getSize().x);
    const float windowHeight = static_cast<float>(game.window.getSize().y);

    auto centerText = [](sf::Text& text, float x, float y) {
        auto bounds = text.getLocalBounds();
        sf::Vector2f origin{ bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f };
        text.setOrigin(origin);
        text.setPosition({ x, y });
    };

    float fadeProgress = std::min(1.f, game.introClock.getElapsedTime().asSeconds() / game.introFadeDuration);
    std::uint8_t alpha = static_cast<std::uint8_t>(255.f * fadeProgress);

    sf::Text title{ game.font, titleText, 56 };
    sf::Color titleColor = sf::Color::Red;
    titleColor.a = alpha;
    title.setFillColor(titleColor);
    centerText(title, windowWidth / 2.f, windowHeight * 0.18f);
    game.window.draw(title);

    sf::Text body{ game.font, bodyText, 28 };
    sf::Color bodyColor = sf::Color::White;
    bodyColor.a = alpha;
    body.setFillColor(bodyColor);
    centerText(body, windowWidth / 2.f, windowHeight * 0.48f);
    game.window.draw(body);

    sf::Text prompt{ game.font, promptText, 28 };
    sf::Color promptColor = sf::Color(200, 200, 200);
    promptColor.a = alpha;
    prompt.setFillColor(promptColor);
    centerText(prompt, windowWidth / 2.f, windowHeight * 0.8f);
    game.window.draw(prompt);
}