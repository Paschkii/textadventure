#pragma once
// === SFML Libraries ===
#include <SFML/Graphics/RenderTarget.hpp>  // Provides the rendering target used when drawing quizzes/final choice screens.
#include <SFML/Window/Event.hpp>           // Handles quiz and final-choice keyboard/mouse events.
// === Header Files ===
#include "rendering/locations.hpp"         // Uses LocationId values when starting and resolving quiz routes.

inline constexpr std::size_t kDragonQuizIntroLine = 10;
inline constexpr std::size_t kDragonQuizQuestionLine = 11;
inline constexpr std::size_t kDragonFinalCheerLine = 14;
inline constexpr float kQuizIntroDelay = 10.f;
inline constexpr float kQuizQuestionStartDelay = 4.f;
inline constexpr float kFinalCheerDelay = 5.f;

struct Game;

void drawQuizUI(Game& game, sf::RenderTarget& target);
void handleQuizEvent(Game& game, const sf::Event& event);
void startQuiz(Game& game, LocationId targetLocation, std::size_t questionIndex);
void completeQuizSuccess(Game& game);
void regenerateCurrentQuestion(Game& game);
bool runQuizDevMode(int argc, char** argv);

void startFinalChoice(Game& game);
void handleFinalChoiceEvent(Game& game, const sf::Event& event);
void drawFinalChoiceUI(Game& game, sf::RenderTarget& target);
void updateQuizIntro(Game& game);
void beginQuestionAudio(Game& game);
void stopQuestionAudio(Game& game);
