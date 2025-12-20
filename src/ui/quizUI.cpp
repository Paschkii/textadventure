// === C++ Libraries ===
#include <algorithm>   // Uses std::equal and std::max when parsing command-line dialect requests.
#include <array>       // Stores the fixed name/dialect lists referenced by CLI helpers.
#include <cctype>      // Applies std::tolower when normalizing names and hotkeys.
#include <iostream>    // Prints dialect diagnostics when command-line parsing fails.
#include <optional>    // Returns optional speaker/Location IDs from helper lookups.
#include <random>      // Creates dice rolls when selecting silly names and quiz orders.
#include <string_view> // Parses command-line arguments without copying full strings.
#include <utility>     // Moves replacement questions into place without copies.
// === Header Files ===
#include "quizUI.hpp"             // Declares quiz UI functions implemented in this file.
#include "confirmationUI.hpp"     // Uses the confirmation modal for quiz transitions.
#include "core/game.hpp"          // Mutates Game quiz state, dialog flow, and resources.
#include "helper/colorHelper.hpp" // Applies palette colors for quiz text and prompts.
#include "helper/layoutHelpers.hpp" // Recalculates UI layout when the quiz pops up.
#include "rendering/locations.hpp" // Maps LocationId values to the correct dragon speaker info.
#include "story/textStyles.hpp"   // Provides dragon speaker names/colors used by quizzes.
#include "story/storyIntro.hpp"   // Retrieves quiz scripts and responses linked to each dragon.
#include "ui/sillyRiddles.hpp"     // Supplies the silly riddle pool for bonus questions.

namespace {
    const std::array<std::string, 20> kSillyNames{
        "Neil Down", "Anita Bath", "Ella Vator", "Willy Maykit", "Ray D. Ater",
        "Earl E. Bird", "Chris P. Bacon", "Tom Morrow", "Ben Dover", "Dee Liver",
        "Luke Atmyaz", "May I. Tutchem", "Pat Myaz", "Clee Torres", "I. P. Freely",
        "Yuri Nator", "Annie Position", "Dil Doe", "Wilma Fingerdoo", "Lou Briccant"
    };

    const std::array<TextStyles::SpeakerId, 4> kDialectSpeakers{
        TextStyles::SpeakerId::FireDragon,
        TextStyles::SpeakerId::WaterDragon,
        TextStyles::SpeakerId::EarthDragon,
        TextStyles::SpeakerId::AirDragon
    };

    std::string toLowerAscii(std::string_view input) {
        std::string result;
        result.reserve(input.size());
        for (unsigned char c : input)
            result.push_back(static_cast<char>(std::tolower(c)));
        return result;
    }

    std::string_view stripPrefix(std::string_view value, std::string_view prefix) {
        if (value.size() >= prefix.size() &&
            std::equal(prefix.begin(), prefix.end(), value.begin()))
        {
            return value.substr(prefix.size());
        }
        return value;
    }

    std::optional<TextStyles::SpeakerId> speakerForName(const std::string& normalized) {
        if (normalized == "fire" || normalized == "firedragon" || normalized == "fire-dragon" || normalized == "fire dragon")
            return TextStyles::SpeakerId::FireDragon;
        if (normalized == "water" || normalized == "waterdragon" || normalized == "water-dragon" || normalized == "water dragon")
            return TextStyles::SpeakerId::WaterDragon;
        if (normalized == "earth" || normalized == "earthdragon" || normalized == "earth-dragon" || normalized == "earth dragon")
            return TextStyles::SpeakerId::EarthDragon;
        if (normalized == "air" || normalized == "airdragon" || normalized == "air-dragon" || normalized == "air dragon")
            return TextStyles::SpeakerId::AirDragon;
        return std::nullopt;
    }

    struct DialectRequest {
        bool showAll = true;
        std::optional<TextStyles::SpeakerId> speaker;
    };

    DialectRequest parseDialectRequest(int argc, char** argv) {
        DialectRequest request;
        if (argc <= 2)
            return request;

        std::string_view raw{argv[2]};
        auto candidate = stripPrefix(raw, "--dialect=");
        candidate = stripPrefix(candidate, "--dragon=");
        if (candidate.empty())
            return request;

        auto normalized = toLowerAscii(candidate);
        if (normalized == "all" || normalized == "alldragons" || normalized == "all-dragons" || normalized == "all dragons")
            return request;

        if (auto speaker = speakerForName(normalized)) {
            request.showAll = false;
            request.speaker = *speaker;
            return request;
        }

        std::cout << "Unknown dialect '" << raw << "'. Showing all dialects.\n";
        return request;
    }

    void printDialectPreview(TextStyles::SpeakerId speaker) {
        const auto& style = TextStyles::speakerStyle(speaker);
        std::cout << style.name << " dialect:\n";
        std::cout << "  math prompt: " << StoryIntro::quizMathPrompt(speaker) << "\n";
        std::cout << "  correct response: " << StoryIntro::quizCorrectResponse(speaker) << "\n\n";
    }

    void printDialectPreviews(const DialectRequest& request) {
        if (request.showAll) {
            for (auto speaker : kDialectSpeakers)
                printDialectPreview(speaker);
            return;
        }
        if (request.speaker)
            printDialectPreview(*request.speaker);
    }

    std::size_t locIndex(LocationId id) {
        switch (id) {
            case LocationId::Gonad: return 0;
            case LocationId::Lacrimere: return 1;
            case LocationId::Blyathyroid: return 2;
            case LocationId::Aerobronchi: return 3;
            case LocationId::Cladrenal: return 4;
            default: return 0;
        }
    }

    std::string dragonNameFor(LocationId id) {
        switch (id) {
            case LocationId::Blyathyroid: return TextStyles::speakerStyle(TextStyles::SpeakerId::FireDragon).name;
            case LocationId::Aerobronchi: return TextStyles::speakerStyle(TextStyles::SpeakerId::AirDragon).name;
            case LocationId::Lacrimere: return TextStyles::speakerStyle(TextStyles::SpeakerId::WaterDragon).name;
            case LocationId::Cladrenal: return TextStyles::speakerStyle(TextStyles::SpeakerId::EarthDragon).name;
            default: return "Dragon";
        }
    }

    std::string dragonElementFor(LocationId id) {
        switch (id) {
            case LocationId::Blyathyroid: return "Fire";
            case LocationId::Aerobronchi: return "Air";
            case LocationId::Lacrimere: return "Water";
            case LocationId::Cladrenal: return "Earth";
            default: return "Unknown";
        }
    }

    TextStyles::SpeakerId speakerFor(LocationId id) {
        switch (id) {
            case LocationId::Blyathyroid: return TextStyles::SpeakerId::FireDragon;
            case LocationId::Aerobronchi: return TextStyles::SpeakerId::AirDragon;
            case LocationId::Lacrimere: return TextStyles::SpeakerId::WaterDragon;
            case LocationId::Cladrenal: return TextStyles::SpeakerId::EarthDragon;
            default: return TextStyles::SpeakerId::NoNameNPC;
        }
    }

    std::string mathPromptFor(LocationId id) {
        return StoryIntro::quizMathPrompt(speakerFor(id));
    }

    std::string quizCorrectFeedback(LocationId id, const std::string& explanation) {
        std::string text = StoryIntro::quizCorrectResponse(speakerFor(id));
        if (!explanation.empty()) {
            text += "\n";
            text += explanation;
        }
        return text;
    }

    constexpr float kSelectionLoggingDuration = 3.f;
    constexpr float kSelectionBlinkDuration = 3.f;
    constexpr float kSelectionBlinkInterval = 0.5f;
    constexpr unsigned kQuizFontSize = 28;

    struct MixedTextMetrics {
        float width = 0.f;
        float height = 0.f;
    };

    float measureSegmentWidth(const sf::Font& font, const std::string& text, unsigned size) {
        if (text.empty())
            return 0.f;
        sf::Text metrics(font, text, size);
        return metrics.getLocalBounds().size.x;
    }

    MixedTextMetrics measureMixedText(
        const std::string& text,
        unsigned size,
        const sf::Font& uiFont,
        const sf::Font& quizFont
    ) {
        float lineSpacing = uiFont.getLineSpacing(size);
        float maxWidth = 0.f;
        float lineWidth = 0.f;
        MixedTextMetrics metrics{ 0.f, lineSpacing };
        std::string buffer;
        bool bufferIsDigit = false;

        auto flushBuffer = [&]() {
            if (buffer.empty())
                return;
            const sf::Font& font = bufferIsDigit ? quizFont : uiFont;
            lineWidth += measureSegmentWidth(font, buffer, size);
            buffer.clear();
        };

        for (char raw : text) {
            if (raw == '\n') {
                flushBuffer();
                maxWidth = std::max(maxWidth, lineWidth);
                lineWidth = 0.f;
                metrics.height += lineSpacing;
                bufferIsDigit = false;
                continue;
            }
            bool isDigit = std::isdigit(static_cast<unsigned char>(raw));
            if (buffer.empty()) {
                buffer.push_back(raw);
                bufferIsDigit = isDigit;
            }
            else if (isDigit == bufferIsDigit) {
                buffer.push_back(raw);
            }
            else {
                flushBuffer();
                buffer.push_back(raw);
                bufferIsDigit = isDigit;
            }
        }

        flushBuffer();
        maxWidth = std::max(maxWidth, lineWidth);
        metrics.width = maxWidth;
        return metrics;
    }

    void drawMixedText(
        sf::RenderTarget& target,
        const std::string& text,
        unsigned size,
        const sf::Font& uiFont,
        const sf::Font& quizFont,
        const sf::Color& color,
        sf::Vector2f position
    ) {
        float lineSpacing = uiFont.getLineSpacing(size);
        float x = position.x;
        float y = position.y;
        std::string buffer;
        bool bufferIsDigit = false;

        auto flushBuffer = [&]() {
            if (buffer.empty())
                return;
            const sf::Font& font = bufferIsDigit ? quizFont : uiFont;
            sf::Text drawable(font, buffer, size);
            drawable.setFillColor(color);
            auto bounds = drawable.getLocalBounds();
            drawable.setPosition({ x - bounds.position.x, y - bounds.position.y });
            target.draw(drawable);
            x += bounds.size.x;
            buffer.clear();
        };

        for (char raw : text) {
            if (raw == '\n') {
                flushBuffer();
                x = position.x;
                y += lineSpacing;
                bufferIsDigit = false;
                continue;
            }
            bool isDigit = std::isdigit(static_cast<unsigned char>(raw));
            if (buffer.empty()) {
                buffer.push_back(raw);
                bufferIsDigit = isDigit;
            }
            else if (isDigit == bufferIsDigit) {
                buffer.push_back(raw);
            }
            else {
                flushBuffer();
                buffer.push_back(raw);
                bufferIsDigit = isDigit;
            }
        }

        flushBuffer();
    }

    bool selectionBlinkHighlight(const Game::QuizData& quiz) {
        float elapsed = quiz.blinkClock.getElapsedTime().asSeconds();
        int cycle = static_cast<int>(elapsed / kSelectionBlinkInterval);
        return (cycle % 2) == 0;
    }

    void triggerRiddleAnnouncement(Game& game);

    void applyPendingFeedback(Game& game) {
        const auto& pending = game.quiz.pendingFeedback;
        game.quiz.feedbackDialogue.clear();
        game.quiz.feedbackDialogue.push_back({ pending.speaker, pending.text });
        if (!pending.followup.empty())
            game.quiz.feedbackDialogue.push_back({ pending.speaker, pending.followup });
        game.quiz.feedbackActive = true;
        game.quiz.pendingQuestionAdvance = pending.advance;
        game.quiz.pendingRetry = pending.retry;
        game.quiz.pendingFinish = pending.finish;
        game.quiz.active = false;
        game.state = GameState::Dialogue;
        game.currentDialogue = &game.quiz.feedbackDialogue;
        game.dialogueIndex = 0;
        game.visibleText.clear();
        game.charIndex = 0;
        game.typewriterClock.restart();
        game.quiz.hoveredIndex = -1;
    }

    void updateQuizSelection(Game& game) {
        auto& quiz = game.quiz;
        if (!quiz.pendingFeedbackActive)
            return;

        switch (quiz.selectionPhase) {
        case Game::QuizData::SelectionPhase::Logging: {
            if (quiz.selectionClock.getElapsedTime().asSeconds() >= kSelectionLoggingDuration) {
                quiz.selectionPhase = Game::QuizData::SelectionPhase::Blinking;
                quiz.blinkClock.restart();
                if (quiz.selectionCorrect) {
                    if (game.quizCorrectSound) {
                        game.quizCorrectSound->stop();
                        game.quizCorrectSound->play();
                    }
                }
                else if (game.quizIncorrectSound) {
                    game.quizIncorrectSound->stop();
                    game.quizIncorrectSound->play();
                }
            }
            break;
        }
        case Game::QuizData::SelectionPhase::Blinking: {
            if (quiz.selectionCorrect && quiz.pendingQuestionStartAnnouncement) {
                bool answerSoundPlaying = false;
                if (game.quizCorrectSound)
                    answerSoundPlaying = game.quizCorrectSound->getStatus() == sf::Sound::Status::Playing;
                if (!answerSoundPlaying) {
                    triggerRiddleAnnouncement(game);
                    quiz.pendingQuestionStartAnnouncement = false;
                }
            }
            if (quiz.blinkClock.getElapsedTime().asSeconds() >= kSelectionBlinkDuration) {
                applyPendingFeedback(game);
                quiz.selectionPhase = Game::QuizData::SelectionPhase::Idle;
                quiz.pendingFeedbackActive = false;
                quiz.selectionIndex = -1;
                quiz.selectionCorrect = false;
            }
            break;
        }
            case Game::QuizData::SelectionPhase::Idle:
            default:
                break;
        }
    }

    void triggerRiddleAnnouncement(Game& game) {
        auto& quiz = game.quiz;
        if (game.quizQuestionStartSound) {
            game.quizQuestionStartSound->stop();
            game.quizQuestionStartSound->play();
        }
        quiz.questionAudioPhase = Game::QuizData::QuestionAudioPhase::QuestionStart;
        quiz.questionStartClock.restart();
        quiz.suppressNextQuestionStartRestart = true;
    }

    quiz::Question makeNameQuestion(Game& game, std::mt19937& rng) {
        std::vector<std::string> pool(kSillyNames.begin(), kSillyNames.end());
        pool.push_back(game.quiz.dragonName);
        std::shuffle(pool.begin(), pool.end(), rng);

        std::array<std::string, 4> opts{};
        opts[0] = game.quiz.dragonName;
        std::size_t poolIndex = 0;
        for (int i = 1; i < 4; ++i) {
            while (poolIndex < pool.size() && pool[poolIndex] == game.quiz.dragonName)
                ++poolIndex;
            if (poolIndex < pool.size())
                opts[i] = pool[poolIndex++];
            else
                opts[i] = "???";
        }

        std::array<int, 4> order{ 0, 1, 2, 3 };
        std::shuffle(order.begin(), order.end(), rng);

        quiz::Question q;
        q.prompt = "What is my name?";
        q.category = quiz::Category::Name;
        q.explanation = "You picked the only correct dragon name.";

        for (std::size_t i = 0; i < order.size(); ++i) {
            int src = order[i];
            q.options[i] = opts[src];
            if (src == 0)
                q.correctIndex = static_cast<int>(i);
        }

        return q;
    }

    quiz::Question* currentQuestion(Game& game) {
        if (game.quiz.currentQuestion >= game.quiz.questions.size())
            return nullptr;
        return &game.quiz.questions[game.quiz.currentQuestion];
    }

    const quiz::Question* currentQuestion(const Game& game) {
        if (game.quiz.currentQuestion >= game.quiz.questions.size())
            return nullptr;
        return &game.quiz.questions[game.quiz.currentQuestion];
    }

    void regenerateCurrentQuestionInternal(Game& game) {
        if (game.quiz.questions.empty() || game.quiz.currentQuestion >= game.quiz.questions.size())
            return;

        std::mt19937 rng(std::random_device{}());
        if (game.quiz.currentQuestion == 0 || game.quiz.questions[game.quiz.currentQuestion].category == quiz::Category::Name) {
            game.quiz.questions[game.quiz.currentQuestion] = makeNameQuestion(game, rng);
            return;
        }

        auto category = game.quiz.questions[game.quiz.currentQuestion].category;
        game.quiz.questions[game.quiz.currentQuestion] = quiz::generateQuestion(category, rng);
    }

    void handleSelection(Game& game, int index) {
        if (!game.quiz.quizDialogue)
            return;
        if (game.quiz.pendingFeedbackActive)
            return;
        stopQuestionAudio(game);

        auto* question = currentQuestion(game);
        if (!question)
            return;

        auto questionIdx = game.quiz.questionIndex;
        auto speaker = speakerFor(game.quiz.targetLocation);
        game.quiz.pendingQuestionStartAnnouncement = false;

        auto feedbackFromStory = [&](std::size_t offset, const std::string& fallback) -> std::string {
            if (game.quiz.quizDialogue && questionIdx + offset < game.quiz.quizDialogue->size())
                return (*game.quiz.quizDialogue)[questionIdx + offset].text;
            return fallback;
        };

        bool isLastQuestion = game.quiz.currentQuestion + 1 >= game.quiz.questions.size();
        bool isNameQuestion = game.quiz.currentQuestion == 0;

        std::string feedback;
        std::string followup;
        bool advance = false;
        bool retry = false;
        bool finish = false;

        bool questionCorrect = question->acceptAnyAnswer || index == question->correctIndex;
        if (questionCorrect) {
            if (isNameQuestion) {
                feedback = feedbackFromStory(2, "Correct, my name is " + game.quiz.dragonName + "!");
            } else {
                feedback = quizCorrectFeedback(game.quiz.targetLocation, question->explanation);
            }

            if (!isLastQuestion) {
                auto nextIdx = game.quiz.currentQuestion + 1;
                if (nextIdx < game.quiz.questions.size()) {
                    auto nextCat = game.quiz.questions[nextIdx].category;
                    if (nextCat != quiz::Category::Name)
                        followup = mathPromptFor(game.quiz.targetLocation);
                }
            }

            if (!isLastQuestion) {
                std::string announcement = "Ladies and Gentleman, it's Riddle Number "
                    + std::to_string(static_cast<int>(game.quiz.currentQuestion + 2)) + "!";
                if (!followup.empty())
                    followup = announcement + "\n" + followup;
                else
                    followup = announcement;
                game.quiz.pendingQuestionStartAnnouncement = true;
            }

        advance = !isLastQuestion;
        finish = isLastQuestion;
        } else {
            feedback = feedbackFromStory(1, "Wrong!");
            retry = true;
            game.totalRiddleFaults++;

            if (question->category == quiz::Category::Silly) {
                std::string currentPrompt = question->prompt;
                std::optional<quiz::Question> replacement;
                for (int attempts = 0; attempts < 8; ++attempts) {
                    auto sample = sillyRiddles::sample(game.quiz.rng, 1);
                    if (sample.empty())
                        break;
                    if (sample[0].prompt == currentPrompt)
                        continue;
                    replacement = std::move(sample[0]);
                    break;
                }
                if (replacement)
                    game.quiz.pendingSillyReplacement = std::move(replacement);
                else
                    game.quiz.pendingSillyReplacement.reset();
            } else {
                game.quiz.pendingSillyReplacement.reset();
            }
        }

        game.quiz.pendingFeedback = {
            speaker,
            feedback,
            followup,
            advance,
            retry,
            finish
        };
        game.quiz.pendingFeedbackActive = true;
        game.quiz.selectionIndex = index;
        game.quiz.selectionCorrect = questionCorrect;
        game.quiz.selectionPhase = Game::QuizData::SelectionPhase::Logging;
        game.quiz.selectionClock.restart();
        game.quiz.hoveredIndex = -1;

        if (game.quizLoggingSound) {
            game.quizLoggingSound->stop();
            game.quizLoggingSound->play();
        }
    }

    std::string injectSpeakerNamesForQuiz(const std::string& text, const Game& game) {
        std::string out = text;
        auto replaceToken = [&](const std::string& token, const std::string& value) {
            if (value.empty())
                return;
            std::size_t pos = 0;
            while ((pos = out.find(token, pos)) != std::string::npos) {
                out.replace(pos, token.size(), value);
                pos += value.size();
            }
        };
        auto selectedWeaponName = [&]() -> std::string {
            if (game.selectedWeaponIndex >= 0 && game.selectedWeaponIndex < static_cast<int>(game.weaponOptions.size()))
                return game.weaponOptions[game.selectedWeaponIndex].displayName;
            return "your weapon";
        };

        replaceToken("{player}", game.playerName);
        replaceToken("{playerName}", game.playerName);
        replaceToken("{fireDragon}", TextStyles::speakerStyle(TextStyles::SpeakerId::FireDragon).name);
        replaceToken("{waterDragon}", TextStyles::speakerStyle(TextStyles::SpeakerId::WaterDragon).name);
        replaceToken("{earthDragon}", TextStyles::speakerStyle(TextStyles::SpeakerId::EarthDragon).name);
        replaceToken("{airDragon}", TextStyles::speakerStyle(TextStyles::SpeakerId::AirDragon).name);
        replaceToken("{lastDragonName}", game.lastDragonName);
        replaceToken("{weapon}", selectedWeaponName());

        return out;
    }
}

bool runQuizDevMode(int argc, char** argv) {
    if (argc <= 1 || std::string(argv[1]) != "--quiz-dev")
        return false;

    auto dialectRequest = parseDialectRequest(argc, argv);
    std::mt19937 rng(std::random_device{}());
    auto questions = quiz::generateNumberQuiz(rng);
    const char labels[] = { 'A', 'B', 'C', 'D' };

    std::cout << "Generating " << questions.size() << " quiz questions.\n\n";
    for (std::size_t i = 0; i < questions.size(); ++i) {
        const auto& q = questions[i];
        std::cout << "Question " << (i + 1) << " (" << quiz::toString(q.category) << ")\n";
        std::cout << q.prompt << "\n";
        for (int j = 0; j < 4; ++j) {
            bool correct = j == q.correctIndex;
            std::cout << "  " << labels[j] << ") " << q.options[j];
            if (correct)
                std::cout << "  <-- correct";
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "Dialect previews:\n";
    printDialectPreviews(dialectRequest);
    return true;
}

static void updateQuestionAudio(Game& game) {
    auto& quiz = game.quiz;
    if (quiz.questionAudioPhase != Game::QuizData::QuestionAudioPhase::QuestionStart)
        return;
    if (quiz.questionStartClock.getElapsedTime().asSeconds() < kQuizQuestionStartDelay)
        return;
    if (game.quizQuestionStartSound)
        game.quizQuestionStartSound->stop();
    if (game.quizQuestionThinkingSound) {
        game.quizQuestionThinkingSound->setLooping(true);
        game.quizQuestionThinkingSound->play();
    }
    quiz.questionAudioPhase = Game::QuizData::QuestionAudioPhase::Thinking;
}

void regenerateCurrentQuestion(Game& game) {
    regenerateCurrentQuestionInternal(game);
}

void startQuiz(Game& game, LocationId targetLocation, std::size_t questionIndex) {
    auto& quiz = game.quiz;
    quiz.rng.seed(std::random_device{}());
    quiz.pendingSillyReplacement.reset();
    auto& rng = quiz.rng;
    quiz.quizAutoStarted = false;
    quiz.intro.active = false;
    quiz.intro.dialogue = nullptr;
    quiz.finalCheerActive = false;
    quiz.finalCheerTriggered = false;
    quiz.locationMusicMuted = false;
    quiz.suppressNextQuestionStartRestart = false;
    stopQuestionAudio(game);

    game.quiz.active = true;
    game.state = GameState::Quiz;
    game.quiz.targetLocation = targetLocation;
    game.quiz.dragonName = dragonNameFor(targetLocation);
    game.quiz.questions.clear();
    game.quiz.questions.push_back(makeNameQuestion(game, rng));
    auto numberQuiz = quiz::generateNumberQuiz(rng);
    std::shuffle(numberQuiz.begin(), numberQuiz.end(), rng);
    if (numberQuiz.size() > 2)
        numberQuiz.resize(2);
    game.quiz.questions.insert(game.quiz.questions.end(), numberQuiz.begin(), numberQuiz.end());

    auto sillyQuestions = sillyRiddles::sample(rng, 2);
    game.quiz.questions.insert(game.quiz.questions.end(), sillyQuestions.begin(), sillyQuestions.end());
    game.quiz.currentQuestion = 0;
    game.quiz.questionIndex = questionIndex;
    game.quiz.quizDialogue = game.currentDialogue;
    game.quiz.hoveredIndex = -1;
    game.quiz.pendingSuccess = false;
    game.quiz.feedbackActive = false;
    game.quiz.pendingQuestionAdvance = false;
    game.quiz.pendingRetry = false;
    game.quiz.pendingFinish = false;
    game.quiz.pendingQuestionStartAnnouncement = false;
    beginQuestionAudio(game);
}

void handleQuizEvent(Game& game, const sf::Event& event) {
    if (!game.quiz.active)
        return;
    if (game.quiz.pendingFeedbackActive)
        return;

    if (event.is<sf::Event::MouseMoved>()) {
        auto pos = game.window.mapPixelToCoords(sf::Mouse::getPosition(game.window));
        game.quiz.hoveredIndex = -1;
        for (int i = 0; i < 4; ++i) {
            if (game.quiz.optionBounds[i].contains(pos)) {
                game.quiz.hoveredIndex = i;
                break;
            }
        }
    }
    else if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button != sf::Mouse::Button::Left)
            return;
        auto pos = game.window.mapPixelToCoords(button->position);
        for (int i = 0; i < 4; ++i) {
            if (game.quiz.optionBounds[i].contains(pos)) {
                handleSelection(game, i);
                return;
            }
        }
    }
    else if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        int idx = -1;
        switch (key->scancode) {
            case sf::Keyboard::Scan::A: idx = 0; break;
            case sf::Keyboard::Scan::B: idx = 1; break;
            case sf::Keyboard::Scan::C: idx = 2; break;
            case sf::Keyboard::Scan::D: idx = 3; break;
            default: break;
        }
        if (idx >= 0) {
            handleSelection(game, idx);
        }
    }
}

void drawQuizUI(Game& game, sf::RenderTarget& target) {
    if (!game.quiz.active)
        return;

    updateQuestionAudio(game);
    updateQuizSelection(game);
    if (game.state != GameState::Quiz)
        return;

    auto* question = currentQuestion(game);
    if (!question)
        return;

    auto textPos = game.textBox.getPosition();
    auto textSize = game.textBox.getSize();
    float padding = 14.f;
    float buttonHeight = 44.f;
    float buttonWidth = (textSize.x - padding * 3.f) / 2.f;

    int totalQuestions = static_cast<int>(game.quiz.questions.size());
    std::string progressLabel = "Riddle " + std::to_string(game.quiz.currentQuestion + 1) + "/" + std::to_string(std::max(1, totalQuestions));

    const sf::Font& quizFont = game.resources.quizFont;
    const sf::Font& uiFont = game.resources.uiFont;

    auto progressMetrics = measureMixedText(progressLabel, kQuizFontSize, uiFont, quizFont);
    auto promptMetrics = measureMixedText(question->prompt, kQuizFontSize, uiFont, quizFont);
    float textBlockHeight = progressMetrics.height + promptMetrics.height + padding * 0.5f;

    float popupWidth = textSize.x;
    float popupHeight = padding * 4.f + textBlockHeight + buttonHeight * 2.f + 10.f;
    float popupX = textPos.x;
    float popupY = textPos.y - popupHeight - 12.f;
    popupY = std::max(12.f, popupY);

    sf::RectangleShape bg({ popupWidth, popupHeight });
    bg.setPosition({ popupX, popupY });
    bg.setFillColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, 0.95f));
    bg.setOutlineThickness(3.f);
    bg.setOutlineColor(ColorHelper::Palette::FrameGoldLight);
    target.draw(bg);

    float contentY = popupY + padding;

    drawMixedText(target, progressLabel, kQuizFontSize, uiFont, quizFont, ColorHelper::Palette::Normal, { popupX + padding, contentY });
    contentY += progressMetrics.height + 6.f;

    drawMixedText(target, question->prompt, kQuizFontSize, uiFont, quizFont, ColorHelper::Palette::Normal, { popupX + padding, contentY });
    contentY += promptMetrics.height + padding;

    float buttonsTop = contentY;
    std::array<sf::Vector2f, 4> positions{
        sf::Vector2f{ popupX + padding, buttonsTop },
        sf::Vector2f{ popupX + padding + buttonWidth + padding, buttonsTop },
        sf::Vector2f{ popupX + padding, buttonsTop + buttonHeight + padding },
        sf::Vector2f{ popupX + padding + buttonWidth + padding, buttonsTop + buttonHeight + padding }
    };

    auto selectionPhase = game.quiz.selectionPhase;
    bool selectionActive = selectionPhase != Game::QuizData::SelectionPhase::Idle;
    bool blinkingPhase = selectionPhase == Game::QuizData::SelectionPhase::Blinking;
    bool blinkHighlight = blinkingPhase && selectionBlinkHighlight(game.quiz);

    for (int i = 0; i < 4; ++i) {
        std::string label;
        switch (i) {
            case 0: label = "A: "; break;
            case 1: label = "B: "; break;
            case 2: label = "C: "; break;
            case 3: label = "D: "; break;
        }
        label += question->options[i];

        sf::RectangleShape btn({ buttonWidth, buttonHeight });
        btn.setPosition(positions[i]);
        bool hovered = game.quiz.hoveredIndex == i;
        sf::Color baseColor = TextStyles::UI::Panel;
        float fillAlpha = hovered ? 0.95f : 0.8f;

        if (selectionActive && game.quiz.selectionIndex == i) {
            if (selectionPhase == Game::QuizData::SelectionPhase::Logging) {
                baseColor = ColorHelper::Palette::SoftOrange;
                fillAlpha = 0.95f;
            }
            else if (selectionPhase == Game::QuizData::SelectionPhase::Blinking) {
                baseColor = blinkHighlight
                    ? (game.quiz.selectionCorrect ? ColorHelper::Palette::Green : ColorHelper::Palette::SoftRed)
                    : TextStyles::UI::PanelDark;
                fillAlpha = 0.95f;
            }
            hovered = false;
        }
        else if (hovered) {
            baseColor = ColorHelper::Palette::BlueLight;
            fillAlpha = 0.95f;
        }

        btn.setFillColor(ColorHelper::applyAlphaFactor(baseColor, fillAlpha));
        btn.setOutlineThickness(2.f);
        btn.setOutlineColor(ColorHelper::Palette::FrameGoldDark);
        target.draw(btn);

        auto labelMetrics = measureMixedText(label, kQuizFontSize, uiFont, quizFont);
        float labelY = positions[i].y + (buttonHeight - labelMetrics.height) * 0.5f;
        drawMixedText(
            target,
            label,
            kQuizFontSize,
            uiFont,
            quizFont,
            ColorHelper::Palette::Normal,
            { positions[i].x + 10.f, labelY }
        );

        game.quiz.optionBounds[i] = btn.getGlobalBounds();
    }
}

void beginQuestionAudio(Game& game) {
    auto& quiz = game.quiz;
    if (quiz.suppressNextQuestionStartRestart) {
        quiz.suppressNextQuestionStartRestart = false;
        return;
    }
    stopQuestionAudio(game);
    if (quiz.questionStartSuppressed) {
        quiz.questionStartSuppressed = false;
        if (game.quizQuestionThinkingSound) {
            game.quizQuestionThinkingSound->setLooping(true);
            game.quizQuestionThinkingSound->play();
        }
        quiz.questionAudioPhase = Game::QuizData::QuestionAudioPhase::Thinking;
    } else {
        if (game.quizQuestionStartSound) {
            game.quizQuestionStartSound->setLooping(false);
            game.quizQuestionStartSound->play();
        }
        quiz.questionAudioPhase = Game::QuizData::QuestionAudioPhase::QuestionStart;
        quiz.questionStartClock.restart();
    }
}

void stopQuestionAudio(Game& game) {
    auto& quiz = game.quiz;
    if (game.quizQuestionStartSound)
        game.quizQuestionStartSound->stop();
    if (game.quizQuestionThinkingSound) {
        game.quizQuestionThinkingSound->stop();
        game.quizQuestionThinkingSound->setLooping(false);
    }
    quiz.questionAudioPhase = Game::QuizData::QuestionAudioPhase::Idle;
}

void updateQuizIntro(Game& game) {
    auto& quiz = game.quiz;
    if (!quiz.intro.active)
        return;
    if (quiz.intro.clock.getElapsedTime().asSeconds() < kQuizIntroDelay)
        return;

    quiz.intro.active = false;
    game.dialogueIndex = quiz.intro.questionIndex;
    if (quiz.intro.dialogue && quiz.intro.questionIndex < quiz.intro.dialogue->size()) {
        game.visibleText = injectSpeakerNamesForQuiz((*quiz.intro.dialogue)[quiz.intro.questionIndex].text, game);
        game.charIndex = game.visibleText.size();
    }
    startQuiz(game, quiz.intro.targetLocation, quiz.intro.questionIndex);
    quiz.quizAutoStarted = true;
    quiz.intro.dialogue = nullptr;
}

void completeQuizSuccess(Game& game) {
    stopQuestionAudio(game);
    game.quiz.quizAutoStarted = false;
    game.quiz.active = false;
    game.state = GameState::Dialogue;
    game.lastDragonName = game.quiz.dragonName;

    LocationId location = game.quiz.targetLocation;
    game.bookshelf.rewardLocation = location;
    game.bookshelf.awaitingDragonstoneReward = true;
    game.bookshelf.promptDialogueActive = true;

    game.transientDialogue.clear();
    auto dragonSpeaker = speakerFor(location);
    std::string bookshelfLine = "Take a look at this bookshelf. You might find something interesting!";
    std::string followupLine = "Only after you bring back the tome hiding the piece of the Umbra Ossea map "
        "will the Dragon Stone dialogue finally kick in.";
    game.transientDialogue.push_back({ dragonSpeaker, bookshelfLine });
    game.transientDialogue.push_back({ Speaker::StoryTeller, followupLine });

    game.currentDialogue = &game.transientDialogue;
    game.dialogueIndex = 0;
    game.visibleText.clear();
    game.charIndex = 0;
    game.typewriterClock.restart();
    game.transientReturnToMap = false;
    game.pendingTeleportToGonad = false;
    game.keyboardMapHover.reset();
    game.mouseMapHover.reset();
}

void presentDragonstoneReward(Game& game) {
    auto& state = game.bookshelf;
    if (!state.awaitingDragonstoneReward)
        return;

    state.returnAfterBookDialogue = false;

    LocationId location = state.rewardLocation;
    state.awaitingDragonstoneReward = false;
    state.promptDialogueActive = false;

    stopQuestionAudio(game);
    game.quiz.quizAutoStarted = false;
    game.quiz.active = false;
    game.state = GameState::Dialogue;
    game.lastCompletedLocation = location;
    game.locationCompleted[locIndex(location)] = true;
    game.dragonStoneCount++;
    if (game.dragonStoneCount >= 4)
        game.finalEncounterPending = true;
    game.itemController.collectDragonstone(location);
    game.transientDialogue.clear();
    game.pendingTeleportToGonad = true;

    for (const auto& line : dragonstone) {
        std::string text = line.text;
        auto replaceAll = [&](const std::string& token, const std::string& value) {
            std::size_t pos = 0;
            while ((pos = text.find(token, pos)) != std::string::npos) {
                text.replace(pos, token.size(), value);
                pos += value.size();
            }
        };
        replaceAll("{dragonelement}", dragonElementFor(location));
        replaceAll("{dragonstonecount}", std::to_string(game.dragonStoneCount));
        const std::string dragonstoneWord =
            (game.dragonStoneCount == 1 ? "Dragon Stone" : "Dragon Stones");
        replaceAll("{dragonstoneword}", dragonstoneWord);
        game.transientDialogue.push_back({
            line.speaker,
            text,
            line.triggersNameInput,
            line.triggersGenderSelection,
            line.waitForEnter
        });
    }

    game.currentDialogue = &game.transientDialogue;
    game.dialogueIndex = 0;
    game.visibleText.clear();
    game.charIndex = 0;
    game.typewriterClock.restart();
    game.transientReturnToMap = true;
    game.keyboardMapHover.reset();
    game.mouseMapHover.reset();
    state.rewardLocation = LocationId::Gonad;
}

namespace {
    void resolveFinalChoice(Game& game, int index) {
        game.finalChoice.active = false;
        game.state = GameState::Dialogue;
        game.transientDialogue.clear();

        auto appendDialogue = [&](const std::vector<DialogueLine>& src) {
            game.transientDialogue.insert(game.transientDialogue.end(), src.begin(), src.end());
        };

        switch (index) {
            case 0:
                appendDialogue(finalChoiceKill);
                break;
            case 1:
                appendDialogue(finalChoiceSpare);
                break;
            case 2:
                appendDialogue(finalChoiceAbsorb);
                break;
            default:
                break;
        }

        appendDialogue(finalThanks);
        game.setBackgroundTexture(game.resources.backgroundToryTailor);

        game.currentDialogue = &game.transientDialogue;
        game.dialogueIndex = 0;
        game.visibleText.clear();
        game.charIndex = 0;
        game.typewriterClock.restart();
        game.finalEncounterActive = false;
        game.finalEndingPending = true;
    }
}

void startFinalChoice(Game& game) {
    game.finalChoice.active = true;
    game.finalChoice.hoveredIndex = -1;
    game.finalChoice.options = { "Kill", "Spare", "Absorb" };
    game.state = GameState::FinalChoice;
}

void handleFinalChoiceEvent(Game& game, const sf::Event& event) {
    if (!game.finalChoice.active)
        return;

    if (event.is<sf::Event::MouseMoved>()) {
        auto pos = game.window.mapPixelToCoords(sf::Mouse::getPosition(game.window));
        game.finalChoice.hoveredIndex = -1;
        for (int i = 0; i < 3; ++i) {
            if (game.finalChoice.optionBounds[i].contains(pos)) {
                game.finalChoice.hoveredIndex = i;
                break;
            }
        }
    }
    else if (auto button = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (button->button != sf::Mouse::Button::Left)
            return;
        auto pos = game.window.mapPixelToCoords(button->position);
        for (int i = 0; i < 3; ++i) {
            if (game.finalChoice.optionBounds[i].contains(pos)) {
                resolveFinalChoice(game, i);
                return;
            }
        }
    }
    else if (auto key = event.getIf<sf::Event::KeyReleased>()) {
        int idx = -1;
        switch (key->scancode) {
            case sf::Keyboard::Scan::A: idx = 0; break;
            case sf::Keyboard::Scan::B: idx = 1; break;
            case sf::Keyboard::Scan::C: idx = 2; break;
            case sf::Keyboard::Scan::Num1: idx = 0; break;
            case sf::Keyboard::Scan::Num2: idx = 1; break;
            case sf::Keyboard::Scan::Num3: idx = 2; break;
            default: break;
        }
        if (idx >= 0) {
            resolveFinalChoice(game, idx);
        }
    }
}

void drawFinalChoiceUI(Game& game, sf::RenderTarget& target) {
    if (!game.finalChoice.active)
        return;

    const sf::Font& quizFont = game.resources.quizFont;
    const sf::Font& uiFont = game.resources.uiFont;

    auto textPos = game.textBox.getPosition();
    auto textSize = game.textBox.getSize();
    float padding = 14.f;
    float buttonHeight = 48.f;
    float buttonWidth = textSize.x - padding * 2.f;

    float popupWidth = textSize.x;
    float popupHeight = padding * 4.f + buttonHeight * 3.f;
    float popupX = textPos.x;
    float popupY = textPos.y - popupHeight - 12.f;
    popupY = std::max(12.f, popupY);

    sf::RectangleShape bg({ popupWidth, popupHeight });
    bg.setPosition({ popupX, popupY });
    bg.setFillColor(ColorHelper::applyAlphaFactor(TextStyles::UI::PanelDark, 0.95f));
    bg.setOutlineThickness(3.f);
    bg.setOutlineColor(ColorHelper::Palette::FrameGoldLight);
    target.draw(bg);

    float currentY = popupY + padding;
    for (int i = 0; i < 3; ++i) {
        std::string label;
        switch (i) {
            case 0: label = "1: "; break;
            case 1: label = "2: "; break;
            case 2: label = "3: "; break;
        }
        label += game.finalChoice.options[i];

        sf::RectangleShape btn({ buttonWidth, buttonHeight });
        btn.setPosition({ popupX + padding, currentY });
        bool hovered = game.finalChoice.hoveredIndex == i;
        btn.setFillColor(ColorHelper::applyAlphaFactor(hovered ? ColorHelper::Palette::BlueLight : TextStyles::UI::Panel, hovered ? 0.95f : 0.8f));
        btn.setOutlineThickness(2.f);
        btn.setOutlineColor(ColorHelper::Palette::FrameGoldDark);
        target.draw(btn);

        auto labelMetrics = measureMixedText(label, kQuizFontSize, uiFont, quizFont);
        float labelY = currentY + (buttonHeight - labelMetrics.height) * 0.5f;
        drawMixedText(
            target,
            label,
            kQuizFontSize,
            uiFont,
            quizFont,
            ColorHelper::Palette::Normal,
            { popupX + padding + 10.f, labelY }
        );

        game.finalChoice.optionBounds[i] = btn.getGlobalBounds();
        currentY += buttonHeight + padding;
    }
}
