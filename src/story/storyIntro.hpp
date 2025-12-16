#pragma once
// === C++ Libraries ===
#include <array>                    // Stores quiz speech entries in fixed-size arrays.
#include <string>                   // Holds narration strings for dialogue lines.
#include <utility>                  // Provides std::pair used by color token helpers.
#include <vector>                   // Aggregates dialogue sequences and color keywords.
// === Header Files ===
#include "dialogueLine.hpp"         // Supplies DialogueLine definitions for the story arrays.
#include "helper/colorHelper.hpp"   // Supplies palette colors referenced by color keywords.
#include "story/textStyles.hpp"     // Provides speaker identifiers used by the story helpers.

namespace StoryIntro {
// Declares keywords that should glow in the intro text and their tinted colors.
struct ColorKeyword {
    std::vector<std::string> words;
    sf::Color color;
};

// Colorized keyword groups used in the intro/quiz text draw helper.
inline const std::vector<ColorKeyword> kColorHighlights = {
    { { "Dragon Stone", "Dragon Stones", "Dragon Stones!", "Dragon stone", "Dragon stones" }, ColorHelper::Palette::Dim },
    { { "DRAGON STONE", "DRAGON STONES", "DRRAGON STONE", "DRRAGON STONES" }, ColorHelper::Palette::Dim },
    { { "Dragonborn", "Dragon Scales", "smoky", "iron", "metal", "stone", "Stone" }, ColorHelper::Palette::Dim },
    { { "Master Bates", "Bates", "Ominous Castle", "castle", "shady figure", "Seminiferous" }, ColorHelper::Palette::DarkPurple },
    { { "Noah", "Noah Lott", "Tory Tailor", "Wanda Rinn", "Will Figsid-Wright", "NoNameWanderer" }, ColorHelper::Palette::PurpleBlue },
    { { "Fire Dragon", "hottest", "burning", "crackling embers", "wildfire", "heat", "wall of flames", "Flames", "Blyathyroid" }, ColorHelper::Palette::FireDragon },
    { { "Water Dragon", "surface", "deep waters", "droplets", "flows", "riverbed", "currents", "cold stream", "Lacrimere" }, ColorHelper::Palette::WaterDragon },
    { { "Earth Dragon", "bedrock", "mountain wall", "rumbles", "shifting earth", "puddle of mud", "Cladrenal" }, ColorHelper::Palette::EarthDragon },
    { { "Air Dragon", "airy", "breeze", "swirling gusts", "wind", "winds", "Aerobronchi" }, ColorHelper::Palette::AirDragon },
    { { "Gonad", "Glandular", "Holmabir", "Kattkavar", "Stiggedin", "destination", "weapon", "Who wants tyo be", "Hu vants to be a", "Who wants-a to be a", "Who wants to be a" }, ColorHelper::Palette::TitleAccent },
};

// Returns a flattened list of tokens/colors for quicker lookup while typing.
inline const std::vector<std::pair<std::string, sf::Color>>& colorTokens() {
    static const std::vector<std::pair<std::string, sf::Color>> tokens = [] {
        std::vector<std::pair<std::string, sf::Color>> flattened;
        std::size_t total = 0;
        for (const auto& entry : kColorHighlights)
            total += entry.words.size();
        flattened.reserve(total);
        for (const auto& entry : kColorHighlights) {
            for (const auto& word : entry.words)
                flattened.emplace_back(word, entry.color);
        }
        return flattened;
    }();
    return tokens;
}

// Represents scripted quiz prompts keyed by dragon speaker identity.
struct QuizSpeech {
    TextStyles::SpeakerId speaker;
    std::string mathPrompt;
    std::string correctResponse;
};

// Hard-coded speech lines used when quiz questions are presented by each dragon.
inline const std::array<QuizSpeech, 4> kQuizSpeeches = {
    QuizSpeech{
        TextStyles::SpeakerId::FireDragon,
        "Vhat is da solyushun to zis math riddle?",
        "Da! Flames roar for this correct answer!"
    },
    QuizSpeech{
        TextStyles::SpeakerId::WaterDragon,
        "Vat is ze solution to zis math riddle?",
        "Jawoll! Ze currents applaud your logic!"
    },
    QuizSpeech{
        TextStyles::SpeakerId::EarthDragon,
        "What's the solution to this math riddle?",
        "YEEHAW! Yer reasoning is solid as bedrock!"
    },
    QuizSpeech{
        TextStyles::SpeakerId::AirDragon,
        "What-a is the solutione to dis math riddle?",
        "BRAVISSIMO! De winds cheer fer your answer!"
    }
};

// Looks up the quiz speech entry belonging to a specific dragon.
inline const QuizSpeech* quizSpeechFor(TextStyles::SpeakerId speaker) {
    for (const auto& entry : kQuizSpeeches) {
        if (entry.speaker == speaker)
            return &entry;
    }
    return nullptr;
}

// Returns the math prompt string that matches the dragon speaker.
inline const std::string& quizMathPrompt(TextStyles::SpeakerId speaker) {
    static const std::string kDefault = "What is the solution to this math riddle?";
    if (auto* speech = quizSpeechFor(speaker))
        return speech->mathPrompt;
    return kDefault;
}

// Returns the custom celebration text for the specified dragon speaker.
inline std::string quizCorrectResponse(TextStyles::SpeakerId speaker) {
    if (auto* speech = quizSpeechFor(speaker))
        return speech->correctResponse;
    if (speaker == TextStyles::SpeakerId::AirDragon) {
        return "Corretto!";
    }
    if (speaker == TextStyles::SpeakerId::EarthDragon) {
        return "Correct!";
    }
    if (speaker == TextStyles::SpeakerId::FireDragon) {
        return "Korrektnyj!";
    }
    if (speaker == TextStyles::SpeakerId::WaterDragon) {
        return "Korrekt!";
    }
    return {};
}
} // namespace StoryIntro

using Speaker = TextStyles::SpeakerId;

// Introductory dialogue seen when the game first loads.
inline const std::vector<DialogueLine> intro = {
    { Speaker::NoNameNPC, "Welcome to this tiny fictional adventure." },
    { Speaker::NoNameNPC, "I am Tory Tailor, your guide and storyteller." },
    { Speaker::StoryTeller, "I will help you on your journey." },
    { Speaker::StoryTeller, "Which one of those do you prefer?", false, true, false },
    { Speaker::StoryTeller, "Alright, let's get started!" },
    { Speaker::StoryTeller, "You will now step into the world of Glandular." },
    { Speaker::StoryTeller, "This is a world of magic, dragons, and adventure." },
    { Speaker::StoryTeller, "You are a Dragonborn, a hero destined to save the world from evil." },
    { Speaker::StoryTeller, "You will face many challenges and make important choices along the way." },
    { Speaker::StoryTeller, "But fear not, for you are not alone. You will have allies to help you on your journey." },
    { Speaker::StoryTeller, "Let's jump right in!" }
};

// Dialogue from Perigonal Fields - Wanda Rinn introducing the player’s awakening.
inline const std::vector<DialogueLine> gonad = {
    { Speaker::NoNameNPC, "Hey...! Are you alright? Wake up...." },
    { Speaker::NoNameNPC, "Let me help you up. You look horrible!" },
    { Speaker::VillageWanderer, "I am Wanda Rinn. What is your name?", true },
    { Speaker::VillageWanderer, "Here, drink this. This will heal you in no time!" },
    { Speaker::VillageWanderer, "Do you remember what happened to you?" },
    { Speaker::Player, "I was fighting against the evil Sorcerer Master Bates. He kidnapped my {dragonSibling}, {dragonSiblingName}." },
    { Speaker::Player, "He managed to knock {dragonSiblingName} out. I got so enraged, that I lost control of my powers..." },
    { Speaker::Player, "I don't really remember what happened afterwards. But {dragonSiblingName}...! I have to save {her/him}..." },
    { Speaker::VillageWanderer, "You are in no condition to fight! It also seems, that your weapons have been broken, look..!" },
    // Display broken weapons after this DialogueLine
    { Speaker::VillageWanderer, "In the village nearby, Perigonal Fields, lives a talented Blacksmith, Will Figsid-Wright, said to be one of the best in whole Glandular! " },
    { Speaker::VillageWanderer, "Maybe he can help you restore your weapons. The Village Elder, Noah Lott, might also be of help as he has a lot of wisdom." },
    { Speaker::Player, "Thank you, Wanda Rinn... Who knows what would have happened to me if you had not found me." },
    { Speaker::VillageWanderer, "Don't worry about that. Let's head for Perigonal Fields. I'll help carrying your belongings." },
    { Speaker::StoryTeller, "Wanda Rinn will help you along your journey. If you ever need her help, just open the Menu to the right" },
    // Show tutorial for the Menu. Basic elements first. Map will be explained later, as well as dragons, quests, riddles
};

// Brief dialogue prompting the player to name the found weapon.
inline const std::vector<DialogueLine> weapon = {
    { Speaker::VillageElder, "Oh! And I found this weapon beside your body. Can you tell me its name?" }
};

// Extended village narrator text describing each dragon’s personality.
inline const std::vector<DialogueLine> dragon = {
    { Speaker::VillageElder, "You have to ask the dragons for their help. These are the 4 dragons:" },
    { Speaker::VillageElder, "Rowsted Sheacane - The Fire Dragon" },
    { Speaker::VillageElder, "This is the hottest temper among the Four, burning through every room he enters." },
    { Speaker::VillageElder, "His tongue is thick-rolled and smoky, spitting sounds like crackling embers - every R erupts like a small wildfire." },
    { Speaker::VillageElder, "Blunt, heavy, and scorching, his words hit with the heat of molten iron." },
    { Speaker::VillageElder, "Flawtin Seamen - The Water Dragon" },
    { Speaker::VillageElder, "Calm on the surface, tidal underneath - he carries the quiet weight of deep waters." },
    { Speaker::VillageElder, "His tongue is sharp, crisp, and clean-cut, with consonants hitting like droplets on metal." },
    { Speaker::VillageElder, "Every sentence flows steady and precise, like a cold stream running over polished stone." },
    { Speaker::VillageElder, "Grounded Claymore - The Earth Dragon" },
    { Speaker::VillageElder, "Steady as bedrock and stubborn as a mountain wall." },
    { Speaker::VillageElder, "His tongue is broad, relaxed, and gravel-warm, sliding through words with a slow drawl that rumbles like shifting earth." },
    { Speaker::VillageElder, "He speaks in solid chunks, every syllable dropping like a stone." },
    { Speaker::VillageElder, "Gustavo Windimaess - The Air Dragon" },
    { Speaker::VillageElder, "Lively, airy, and constantly in motion - the breeze itself can't keep up with him." },
    { Speaker::VillageElder, "His tongue is melodic, rolling, and feather-light, dancing through vowels like swirling gusts." },
    { Speaker::VillageElder, "Every phrase rises and falls like a warm Mediterranean wind." },
};

// Dialogue asking which dragon location to visit first.
inline const std::vector<DialogueLine> destination = {
    { Speaker::VillageElder, "You are all set now. It is up to you to choose your first destination." },
    { Speaker::VillageElder, "Which dragon do you want to visit first?" },
};

// Greeting lines shown when the player returns to Perigonal Fields after a dungeon run.
inline const std::vector<DialogueLine> gonadWelcomeBack = {
    { Speaker::VillageElder, "Welcome back, {playerName}! You made it out of {lastLocation}!" },
    { Speaker::VillageElder, "I see you've retrieved the Dragon Stone from {lastDragonName}." },
    { Speaker::VillageElder, "Catch your breath, then head toward your next destination!" },
};

// Story-heavy dialogue triggered before the final battle in Perigonal Fields.
inline const std::vector<DialogueLine> finalEncounter = {
    { Speaker::StoryTeller, "Upon reaching ground, you freeze for a minute." },
    { Speaker::StoryTeller, "Perigonal Fields is gone. And right where it used to be stands a dark, ominous Castle." },
    { Speaker::StoryTeller, "A shady figure steps out of the castle greeting you with an evil smirk." },
    { Speaker::MasterBates, "So you have finally come, {playerName}! I see you have retrieved all the Dragon Stones!" },
    { Speaker::MasterBates, "But you are too late! I have already absorbed the power of your Dragonborn siblings!" },
    { Speaker::MasterBates, "Now I will defeat you and absorb your power and the power of the Dragon Stones, too!" },
    { Speaker::Player, "I will not let you do that, Master Bates! I will defeat you and free my siblings!" },
    { Speaker::MasterBates, "You are a fool, {playerName}! You will never defeat me!" },
    { Speaker::Player, "Dragon Stones, lend me your power!" },
    { Speaker::StoryTeller, "The Dragon Stones glow and the power of the Fire Dragon, Air Dragon, Water Dragon and Earth Dragon is absorbed into your weapon!" },
    { Speaker::Player, "Eat this, Master Bates! This is for my siblings!" },
    { Speaker::StoryTeller, "You attack Master Bates with the power of the Dragon Stones. Master Bates is defeated!" },
    { Speaker::NoahBates, "Noooooo! How can this be?! I came so far!" },
    { Speaker::Player, "Noah Lott?! You are Master Bates?! You are the one who kidnapped my siblings?!" },
    { Speaker::NoahBates, "Please, {playerName}, don't kill me! We can reign this world together!" },
    { Speaker::Player, "No! You have to pay for what you have done!" },
};

// Closing thank-you lines played after beating the game.
inline const std::vector<DialogueLine> finalThanks = {
    { Speaker::StoryTeller, "Thank you {playerName} for playing my SFML text adventure game!" },
    { Speaker::StoryTeller, "I hope you enjoyed it!" },
};

// Dialogue branch shown when the player chooses to kill Noah Bates.
inline const std::vector<DialogueLine> finalChoiceKill = {
    { Speaker::Player, "The world is a better place without you!" },
    { Speaker::StoryTeller, "You are using {weapon} to kill Noah Bates!" },
    { Speaker::StoryTeller, "You have saved the world!" },
};

// Dialogue branch seen when the player spares Noah Bates.
inline const std::vector<DialogueLine> finalChoiceSpare = {
    { Speaker::Player, "You will die soon anyways! I wont dirty my hands with your blood!" },
    { Speaker::StoryTeller, "You have let Noah Bates survive... Your {weapon} is glowing!" },
    { Speaker::StoryTeller, "Your siblings have been freed!" },
};

// Dialogue branch for the corrupted absorb ending.
inline const std::vector<DialogueLine> finalChoiceAbsorb = {
    { Speaker::Player, "I will absorb your power and thus the power of my siblings!" },
    { Speaker::StoryTeller, "You are using {weapon} to absorb Noah Bates!" },
    { Speaker::StoryTeller, "You have been corrupted by the evil power!" },
    { Speaker::StoryTeller, "You have become the new Master Bates!" },
    { Speaker::StoryTeller, "You are {playerName} Bates!" },
};

inline const std::vector<DialogueLine> firedragon = {
    { Speaker::StoryTeller, "You step back from a wall of flames as you reach your destination." },
    { Speaker::StoryTeller, "This must be Blyathyroid, where the Fire Dragon resides." },
    { Speaker::FireDragon, "Who dares to step into my cayyve vithout perMEEESHON?!" },
    { Speaker::Player, "I am {playerName}, the Dragonborn. I have come to seek your help." },
    { Speaker::FireDragon, "I know vho you ahre. I vas just yoking! You ahre probably looking for my DRRAGON STONE." },
    { Speaker::FireDragon, "I can give you my DRRAGON STONE, but you must prooof your VORTH first." },
    { Speaker::FireDragon, "You must defeat me in glorious duel!" },
    { Speaker::Player, "A duel? But I am not ready for a duel!" },
    { Speaker::FireDragon, "I vas just yoking again! Is so booooring in here. Heh heh heh!" },
    { Speaker::FireDragon, "I will give you my DRRAGON STONE if you can win in this Riddle Game." },
    { Speaker::FireDragon, "Wyelcome byeck to dis shyow: Who wants tyo be Dragonborn? Dis is contestant {playerName}, he searches for DRRAGON STONES!" },
    { Speaker::FireDragon, "Vhat is my name?" }, // starts the question input
    { Speaker::FireDragon, "VRRONG! Try again." }, // Only if wrong
    { Speaker::FireDragon, "Da! You solved it! My name is Rowsted Sheacane!" },
    { Speaker::FireDragon, "Aaaannd we have a winnnneeeer!" },
    { Speaker::FireDragon, "You ahre smart! Now take my DRRAGON STONE and leave my cayyve!" },
    { Speaker::Player, "Thank you, {fireDragon}! I will not forget this!" },
};

inline const std::vector<DialogueLine> waterdragon = {
    { Speaker::StoryTeller, "You fall into a riverbed as you reach your destination." },
    { Speaker::StoryTeller, "This must be Lacrimere, where the Water Dragon resides." },
    { Speaker::WaterDragon, "Hu dares to step into mein hole vizout permission?!" },
    { Speaker::Player, "I am {playerName}, the Dragonborn. I have come to seek your help." },
    { Speaker::WaterDragon, "I know hu you are. I vas only joking! You are probably looking for mein DRAGON STONE." },
    { Speaker::WaterDragon, "I can give you mein DRAGON STONE, but first you must proove your VORTH." },
    { Speaker::WaterDragon, "You must defeat me in a duel. Din-Norm regulated, ja!" },
    { Speaker::Player, "A duel? But I am not ready for a duel!" },
    { Speaker::WaterDragon, "Sis was nur Fun, ja! It is sehr langweilig hier. Heh heh heh!" },
    { Speaker::WaterDragon, "I vill give you mein Dragon Stone if you can win in zis Riddle Game." },
    { Speaker::WaterDragon, "And hier, ja, Willkommen back to: Hu vants to be a Dragonborn? Auer todays Teilnehmer ist {playerName}, ja, hu is searching for ze Dragon Stones!" },
    { Speaker::WaterDragon, "Vhat is mein name?" }, // starts the question input
    { Speaker::WaterDragon, "VRONG! Nochmal." }, // Only if wrong
    { Speaker::WaterDragon, "Jawoll! You are korrekt! Mein name ist Flawtin Seamen!" },
    { Speaker::WaterDragon, "Aaaannd ve have a vinnnneeeer!" },
    { Speaker::WaterDragon, "You are very smart! Now take mein DRAGON STONE and leave mein hole!" },
    { Speaker::Player, "Thank you, {waterDragon}! I will not forget this!" },
};

inline const std::vector<DialogueLine> earthdragon = {
    { Speaker::StoryTeller, "You step into a puddle of mud as you reach your destination." },
    { Speaker::StoryTeller, "This must be Cladrenal, where the Earth Dragon resides." },
    { Speaker::EarthDragon, "Whooo the heck steps into MAH CAVE without even askin'?!" },
    { Speaker::Player, "I am {playerName}, the Dragonborn. I have come to seek your help." },
    { Speaker::EarthDragon, "Aw yeah, I know who ya are! I'm just messin' with ya! Yer probably lookin' for my DRAGON STONE." },
    { Speaker::EarthDragon, "I can hand it over, sure - but ya gotta prooove yer WORTH first." },
    { Speaker::EarthDragon, "Yer gonna hafta defeat me in a duel, partner." },
    { Speaker::Player, "A duel? But I am not ready for a duel!" },
    { Speaker::EarthDragon, "Ha! I'm just foolin' with ya again! It gets real dang borin' in here. Heh heh heh!" },
    { Speaker::EarthDragon, "I will give you my Dragon Stone if you can win in this Riddle Game." },
    { Speaker::EarthDragon, "And hereby I warmly welcome you back to: Who wants to be a Dragonborn? Our todays contestant is {playerName}, who is searching for the Dragon Stones!" },
    { Speaker::EarthDragon, "What's mah name?" }, // starts the question input
    { Speaker::EarthDragon, "Nope! That's wronger than a snake wearin' socks! Try again!" }, // Only if wrong
    { Speaker::EarthDragon, "YEEHAW! That's right! Mah name is Grounded Claymore!" },
    { Speaker::EarthDragon, "Aaaannd we have a winnnneeeer!" },
    { Speaker::EarthDragon, "Yer a smart lil' tumbleweed! Now grab the DRAGON STONE and git outta mah cave!" },
    { Speaker::Player, "Thank you, {earthDragon}! I will not forget this!" },
};

inline const std::vector<DialogueLine> airdragon = {
    { Speaker::StoryTeller, "A heavy breeze hits your face as you reach your destination." },
    { Speaker::StoryTeller, "This must be Aerobronchi, where the Air Dragon resides." },
    { Speaker::AirDragon, "Who-a dares to step into-a MY CAVE without-a PERMISSIONE?!" },
    { Speaker::Player, "I am {playerName}, the Dragonborn. I have come to seek your help." },
    { Speaker::AirDragon, "Aaaah, I know-a who you are! I was-a just joking! You come for my DRAGON STONE, si si!" },
    { Speaker::AirDragon, "I can-a give you my DRAGON STONE, but first you must-a prooove your Valore!" },
    { Speaker::AirDragon, "You must defeat-a me in a duel - molto dramatico!" },
    { Speaker::Player, "A duel? But I am not ready for a duel!" },
    { Speaker::AirDragon, "I was-a only joking again! It is SO BORING in here! Heh heh heh-a!" },
    { Speaker::AirDragon, "I will give you my Dragon Stone if you can win in this Riddle Game." },
    { Speaker::AirDragon, "And molto welcome-a you back-a to: Who wants-a to be a Dragonborn? Our contestant-a today is-a {playerName}, who is searching-a for-a the Dragon Stones!" },
    { Speaker::AirDragon, "What-a is my name?" }, // starts the question input
    { Speaker::AirDragon, "NOOO! Sbagliata! Try-a again!" }, // Only if wrong
    { Speaker::AirDragon, "BRAVISSIMO! You are correct-a! My name is Gustavo Windimaess!" },
    { Speaker::AirDragon, "Aaaannd we have a winnnneeeer!" },
    { Speaker::AirDragon, "You are molto smart-a! Now take-a my DRAGON STONE and leave-a my cave!" },
    { Speaker::Player, "Thank you, {airDragon}! I will not forget this!" },
};

inline const std::vector<DialogueLine> dragonstone = {
    { Speaker::StoryTeller, "You have received the Dragon Stone of the {dragonelement} Dragon!" },
    { Speaker::StoryTeller, "You now have {dragonstonecount} {dragonstoneword}!" },
    { Speaker::StoryTeller, "Your Dragon Scales grow even stronger!" },
    { Speaker::StoryTeller, "Your weapon has been upgraded! It can now use the power of the {dragonelement} Dragon." },
    { Speaker::StoryTeller, "You will now be teleported back to Perigonal Fields." },
};
