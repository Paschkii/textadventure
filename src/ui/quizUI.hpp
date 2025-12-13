#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include "rendering/locations.hpp"

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

void startFinalChoice(Game& game);
void handleFinalChoiceEvent(Game& game, const sf::Event& event);
void drawFinalChoiceUI(Game& game, sf::RenderTarget& target);
void updateQuizIntro(Game& game);
void beginQuestionAudio(Game& game);
void stopQuestionAudio(Game& game);
