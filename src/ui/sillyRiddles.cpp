#include "ui/sillyRiddles.hpp"

#include <algorithm> // Random sampling helpers.
#include <array>     // Fixed-size option/entry lists.
#include <numeric>   // Builds index sequences for sampling.
#include <random>    // Shuffles entries using the provided RNG.
#include <string>    // Stores prompt/option text for the generated questions.
#include <vector>    // Collects and returns the sampled riddles.

namespace sillyRiddles {
namespace {

enum class RiddleType {
    Standard,
    AllCorrect,
    LetterSound
};

struct RiddleTemplate {
    const char* prompt;
    std::array<const char*, 4> options{};
    RiddleType type = RiddleType::Standard;
};

const std::array<RiddleTemplate, 24> kRiddles = {{
    { "What does a house wear?", { "Address", "Foundation", "Contents", "Memories & Comfort" }, RiddleType::Standard },
    { "Where would you take a sick boat? To the..?", { "dock", "harbour", "pier", "port" }, RiddleType::Standard },
    { "What is orange and sounds like a parrot?", { "A carrot", "Annoying orange", "Mandarin", "Orange-bellied Parrot" }, RiddleType::Standard },
    { "Why did the skeleton not go to the party? Because he...?", { "Had no body to go with", "Was in no shape", "Had no remains", "Couldn't drive his carcass" }, RiddleType::Standard },
    { "I have a neck but no head, and I wear a cap. What am I?", { "Bottle", "Donald Trump", "Jellyfish", "Jacket" }, RiddleType::Standard },
    { "What do you call a bear without ears?", { "B", "C", "D", "A" }, RiddleType::Standard },
    { "What do you call a fly without wings?", { "A Walk", "A Drive", "A Limp", "A Laydownandwaittodie" }, RiddleType::Standard },
    { "Which part of a road do ghosts most love to travel?", { "A dead end", "A dead start", "A dead middle", "A dead joke" }, RiddleType::Standard },
    { "What goes up but never comes down?", { "Your age", "Your rage", "Your cage", "Your page" }, RiddleType::Standard },
    { "What kind of band never plays music?", { "Rubber band", "Cantina Band", "Boyband", "Electric Callboy" }, RiddleType::Standard },
    { "What time is it when an elephant sits on a fence?", { "Time to fix the fence", "Time for some coffee", "You know what time it is", "I don't have a watch" }, RiddleType::Standard },
    { "What do you call an alligator in a vest?", { "An investigator", "Alli-Alli-gatoah!", "An Italian", "Is this your vest joke?" }, RiddleType::Standard },
    { "I want to be cremated as it is my last hope for...?", { "A smoking hot body", "A HELLFIRE TO BURN IN!", "A warm bed", "getting used to the heat?" }, RiddleType::Standard },
    { "Ladies, if he can't appreciate your fruit jokes, you need to let that...?", { "Mango", "Mandril", "Manswear", "Mandate" }, RiddleType::Standard },
    { "What was Forrest Gump's email password?", { "1forrest1!", "jenny", "chocolates", "benjamin" }, RiddleType::Standard },
    { "Why was Dumbo sad? Because he felt...?", { "irrelephant", "elephated", "telephisioned", "elephantistic" }, RiddleType::Standard },
    { "Long fairy tales have a tendency to...?", { "Dragon", "Wyvern", "Serpent", "Hydra" }, RiddleType::Standard },
    { "My sister bet I couldn't build a car out of spaghetti. You should've seen her face when I drove", { "Pasta", "Rice", "Potato", "Pizza" }, RiddleType::Standard },
    { "One Windmill asks another, \"What's your favorite kind of music?\" The other says...", { "\"I'm a big metal fan\"", "\"I don't listen to music\"", "\"I like Radio Music\"", "\"The Wind of Change\"" }, RiddleType::Standard },
    { "What did the sushi say to the bee?", { "Wasabi!", "WASABI!", "WAAASAABIII!", "WWAAAASSSAABIII!" }, RiddleType::AllCorrect },
    { "I asked a Frenchman if he played video games. He said...?", { "Wii", "Xbox", "Playstation", "Steamdeck" }, RiddleType::Standard },
    { "What do you call a girl with one leg that's shorter than the other?", { "Aileene", "Shorty", "An Uber?", "Beautiful :>" }, RiddleType::Standard },
    { "What do you call two monkeys that share an Amazon account?", { "Prime mates", "Prime example", "Prime time", "Prime numbers" }, RiddleType::Standard },
    { "What word sounds like it only has one letter?", { "", "", "", "" }, RiddleType::LetterSound }
}};

const std::array<const char*, 8> kLetterSoundCorrect = {
    "Bee",
    "Sea",
    "Age",
    "Eye",
    "Pee",
    "Queue",
    "Tea",
    "You"
};

const std::array<const char*, 24> kLetterSoundWrong = {
    "Asap",
    "btw",
    "Lol",
    "FYI",
    "brb",
    "ETA",
    "imo",
    "JK",
    "lmk",
    "amk",
    "omw",
    "hdf",
    "TBA",
    "XOXO",
    "Hdgdlfiuebaed",
    "Rofl",
    "Yolo",
    "Dafuq",
    "gg",
    "bb",
    "Noob",
    "Boon",
    "TL;DR",
    "FAQ"
};

quiz::Question makeStandardRiddle(const RiddleTemplate& entry, std::mt19937& rng) {
    quiz::Question question;
    question.prompt = entry.prompt;
    question.category = quiz::Category::Silly;
    question.explanation = "Funny logic, no calculators needed.";
    question.acceptAnyAnswer = (entry.type == RiddleType::AllCorrect);
    std::array<int, 4> order{ 0, 1, 2, 3 };
    std::shuffle(order.begin(), order.end(), rng);
    for (int i = 0; i < 4; ++i) {
        question.options[i] = entry.options[order[i]];
        if (order[i] == 0)
            question.correctIndex = i;
    }
    return question;
}

quiz::Question makeLetterSoundRiddle(const RiddleTemplate& entry, std::mt19937& rng) {
    std::array<int, 8> correctIndices{};
    std::iota(correctIndices.begin(), correctIndices.end(), 0);
    std::shuffle(correctIndices.begin(), correctIndices.end(), rng);

    std::array<int, 24> wrongIndices{};
    std::iota(wrongIndices.begin(), wrongIndices.end(), 0);
    std::shuffle(wrongIndices.begin(), wrongIndices.end(), rng);

    std::array<std::string, 4> gathered{
        std::string(kLetterSoundCorrect[correctIndices.front()]),
        std::string(kLetterSoundWrong[wrongIndices[0]]),
        std::string(kLetterSoundWrong[wrongIndices[1]]),
        std::string(kLetterSoundWrong[wrongIndices[2]])
    };

    std::array<int, 4> order{ 0, 1, 2, 3 };
    std::shuffle(order.begin(), order.end(), rng);

    quiz::Question question;
    question.prompt = entry.prompt;
    question.category = quiz::Category::Silly;
    question.explanation = "Each choice sounds like a single letter.";
    for (int i = 0; i < 4; ++i) {
        int src = order[i];
        question.options[i] = gathered[src];
        if (src == 0)
            question.correctIndex = i;
    }
    return question;
}

quiz::Question makeQuestionFromTemplate(const RiddleTemplate& entry, std::mt19937& rng) {
    if (entry.type == RiddleType::LetterSound)
        return makeLetterSoundRiddle(entry, rng);
    return makeStandardRiddle(entry, rng);
}

} // namespace

std::vector<quiz::Question> sample(std::mt19937& rng, std::size_t count) {
    std::vector<int> indices(kRiddles.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), rng);

    if (count > indices.size())
        count = indices.size();

    std::vector<quiz::Question> questions;
    questions.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
        questions.push_back(makeQuestionFromTemplate(kRiddles[indices[i]], rng));

    return questions;
}

} // namespace sillyRiddles
