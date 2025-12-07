#include "game.hpp"
#include "story/dialogueLine.hpp"
#include "story/dialogInput.hpp"
#include "rendering/dialogRender.hpp"
#include "story/storyIntro.hpp"
#include "story/textStyles.hpp"
#include "ui/introTitle.hpp"
#include "ui/weaponSelectionUI.hpp"
#include <iostream>
#include <algorithm>
#include <filesystem>

constexpr unsigned int windowWidth = 1280;
constexpr unsigned int windowHeight = 720;
constexpr unsigned int fpsLimit = 60;

Game::Game()
: window(sf::VideoMode({windowWidth, windowHeight}), "Glandular", sf::Style::Titlebar | sf::Style::Close)
{
    if (!resources.loadAll()) {
        std::cout << "Fatal: konnte Ressourcen nicht laden.\n";
        std::exit(1);
    }
    background.emplace(resources.introBackground);
    returnSprite.emplace(resources.returnSymbol);
    returnSprite->setColor(sf::Color(160, 160, 160));

    locations = Locations::buildLocations(resources);

    textBlipSound.emplace(resources.typewriter);
    enterSound.emplace(resources.enterKey);
    // === Framerate limitieren ===
    window.setFramerateLimit(fpsLimit);
    // === NameBox Style setzen ===
    nameBox.setFillColor(sf::Color::Transparent);
    nameBox.setOutlineColor(sf::Color::White);
    nameBox.setOutlineThickness(2.f);
    // === TextBox Style setzen ===
    textBox.setFillColor(sf::Color::Transparent);
    textBox.setOutlineColor(sf::Color::White);
    textBox.setOutlineThickness(2.f);
    // === LocationBox Style setzen ===
    locationBox.setFillColor(sf::Color::Transparent);
    locationBox.setOutlineColor(sf::Color::White);
    locationBox.setOutlineThickness(2.f);

    weaponPanel.setFillColor(sf::Color::Transparent);
    weaponPanel.setOutlineColor(sf::Color::White);
    weaponPanel.setOutlineThickness(2.f);

    currentDialogue = &intro;

    loadWeaponOptions();
}

// === Layout updaten je nach Fenstergröße ===
void Game::updateLayout() {
    float w = window.getSize().x;
    float h = window.getSize().y;

    float marginX = w * 0.05f;      // 5% vom Rand (64)
    float marginY = h * 0.05f;      // 5% vom Rand (36)

    float boxHeight = h * 0.25f;    // beide Boxen: 25% Höhe (180)
    float nameWidth = w * 0.20f;    // NameBox: 20% Breite (256)
    float textWidth = w * 0.70f;    // TextBox: 70% Breite (896)
    float locationWidth = w * 0.20f; // LocationBox 30% Breite

    // === NameBox: links unten, 5% Abstand ===
    nameBox.setSize({ nameWidth, boxHeight });
    nameBox.setPosition({
        marginX,
        h - boxHeight - marginY
    });

    // === TextBox: rechts unten, 5% Abstand ===
    textBox.setSize({ textWidth - (marginX / 2), boxHeight });
    textBox.setPosition({
        marginX + nameBox.getSize().x + (marginX / 2),
        h - boxHeight - marginY
    });

    locationBox.setSize({ locationWidth, boxHeight / 2 });
    locationBox.setPosition({
        marginX,
        marginY
    });

    float weaponPanelHeight = nameBox.getSize().y * 0.9f;
    float weaponPanelWidth = (textBox.getPosition().x + textBox.getSize().x) - nameBox.getPosition().x;
    float weaponPanelY = nameBox.getPosition().y - weaponPanelHeight - (marginY * 0.5f);

    weaponPanel.setSize({ weaponPanelWidth, weaponPanelHeight });
    weaponPanel.setPosition({ nameBox.getPosition().x, weaponPanelY });

    // === 9-Slice-Textur laden ===
    uiFrame.load("assets/textures/boxborder.png");

    layoutWeaponSelection();
}


// === Öffnet das Fenster und führt Tasks aus ===
void Game::run() {
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {

            if (askingName && event->is<sf::Event::TextEntered>()) {
                const auto& text = event->getIf<sf::Event::TextEntered>();
                char32_t code = text->unicode;

                if (code == U'\r' || code == U'\n') {}
                else if (code == U'\b') {
                    if (!nameInput.empty())
                        nameInput.pop_back();
                }
                else if (code >= 32 && code < 127) {
                    nameInput.push_back(static_cast<char>(code));
                }
            }

            if (auto key = event->getIf<sf::Event::KeyReleased>()) {
                if (key->scancode == sf::Keyboard::Scan::Enter) {

                    if (state == GameState::IntroTitle && titleDropStarted && !introTitleDropComplete(*this))
                        continue;

                    enterSound->stop();
                    enterSound->play();

                    if (state == GameState::IntroScreen) {
                        if (!introFadeOutActive) {
                            introFadeOutActive = true;
                            introClock.restart();
                        }
                    }
                    else if (state == GameState::IntroTitle) {
                        if (introDialogueFinished && introTitleDropComplete(*this)) {
                            triggerIntroTitleExit(*this);
                        }
                        else {
                            waitForEnter(*this, (*currentDialogue)[dialogueIndex]);
                        }
                    }
                    else if (state == GameState::Dialogue) {
                        waitForEnter(*this, (*currentDialogue)[dialogueIndex]);
                    }
                }
            }

            if (event->is<sf::Event::Closed>())
                window.close();
                
            if (state == GameState::WeaponSelection)
                handleWeaponSelectionEvent(*this, *event);
        }

        // sf::Sprite returnSprite(returnSymbol);

        updateLayout();

        window.clear();
        renderGame(*this);
        window.display();
    }
}

void Game::startTypingSound() {
    if (textBlipSound->getStatus() != sf::Sound::Status::Playing) {
        // std::cout << "start on " << this << " / sound " << &textBlipSound << "\n";
        textBlipSound->play();
    }
}

void Game::stopTypingSound() {
    if (textBlipSound->getStatus() == sf::Sound::Status::Playing) {
        // std::cout << "end on " << this << " / sound " << &textBlipSound << "\n";
        textBlipSound->stop();
    }
}

void Game::loadWeaponOptions() {
    namespace fs = std::filesystem;

    const fs::path weaponDir{"assets/gfx/weapons"};
    weaponOptions.clear();

    if (!fs::exists(weaponDir))
        return;

    std::vector<fs::path> weaponFiles;
    for (const auto& entry : fs::directory_iterator(weaponDir)) {
        if (entry.is_regular_file())
            weaponFiles.push_back(entry.path());
    }

    std::sort(weaponFiles.begin(), weaponFiles.end());

    const std::string prefix = "Weapon ";
    for (const auto& path : weaponFiles) {
        WeaponOption option;
        if (!option.texture.loadFromFile(path.string()))
            continue;

        option.sprite.emplace(option.texture);

        std::string stem = path.stem().string();
        auto prefixPos = stem.find(prefix);
        if (prefixPos != std::string::npos)
            stem = stem.substr(prefixPos + prefix.size());

        while (!stem.empty() && stem.front() == ' ')
            stem.erase(stem.begin());

        option.displayName = stem;
        weaponOptions.push_back(std::move(option));
    }
}

void Game::layoutWeaponSelection() {
    if (weaponOptions.empty()) {
        hoveredWeaponIndex = -1;
        selectedWeaponIndex = -1;
        return;
    }

    constexpr float padding = 24.f;
    constexpr float labelHeight = 32.f;

    float availableWidth = weaponPanel.getSize().x - (padding * 2.f);
    float availableHeight = weaponPanel.getSize().y - (padding * 2.f) - labelHeight;

    if (availableWidth <= 0.f || availableHeight <= 0.f)
        return;

    float slotWidth = availableWidth / static_cast<float>(weaponOptions.size());
    float spriteAreaHeight = availableHeight;

    for (std::size_t i = 0; i < weaponOptions.size(); ++i) {
        auto& option = weaponOptions[i];

        float slotCenterX = weaponPanel.getPosition().x + padding + (slotWidth * (static_cast<float>(i) + 0.5f));
        float spriteCenterY = weaponPanel.getPosition().y + padding + (spriteAreaHeight * 0.5f);

        if (!option.sprite)
            continue;

        auto& sprite = *option.sprite;
        sprite.setScale({ 1.f, 1.f });
        auto texSize = option.texture.getSize();
        float scaleX = (slotWidth * 0.7f) / static_cast<float>(texSize.x);
        float scaleY = (spriteAreaHeight * 0.8f) / static_cast<float>(texSize.y);
        float scale = std::min(scaleX, scaleY);
        sprite.setScale({ scale, scale });

        auto localBounds = sprite.getLocalBounds();
        sprite.setOrigin({
            localBounds.position.x + (localBounds.size.x / 2.f),
            localBounds.position.y + (localBounds.size.y / 2.f)
        });
        sprite.setPosition({ slotCenterX, spriteCenterY });
        option.bounds = sprite.getGlobalBounds();

        option.labelPosition = {
            slotCenterX,
            weaponPanel.getPosition().y + padding + spriteAreaHeight + 4.f
        };
    }
}