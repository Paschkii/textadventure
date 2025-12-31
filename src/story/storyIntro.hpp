#pragma once
// === C++ Libraries ===
#include <array>                    // Stores quiz speech entries in fixed-size arrays.
#include <string>                   // Holds narration strings for dialogue lines.
#include <utility>                  // Provides std::pair used by color token helpers.
#include <vector>                   // Aggregates dialogue sequences and color keywords.
// === Header Files ===
#include "dialogueLine.hpp"         // Supplies DialogueLine definitions for the story arrays.
#include "helper/colorHelper.hpp"   // Supplies palette colors referenced by color keywords.

// Dialogue token cheatsheet:
//   {player}/{playerName}       – the current player name entered during the intro.
//   {fireDragon}/{waterDragon}/… – the colored dragon speaker names used inside quizzes.
//   {lastDragonName}            – last dragon name captured while routing through map dialogue.
//   {lastLocation}              – the name of the most recently completed location.
//   {dragonbornName}            – the name of the Dragonborn the player didn't pick (Asha Scale or Ember Scale).
//   {dragonbornSubject}         – uses he/she for the leftover Dragonborn.
//   {dragonbornObject}          – uses him/her for the leftover Dragonborn.
//   {dragonbornPossessive}      – uses his/her for the leftover Dragonborn.
//   {dragonbornOwnPossesive}    - uses her/his for the leftover Dragonborn (used if speaking of itself)
//   {dragonbornPossessivePronoun}- uses his/hers for the leftover Dragonborn.
//   {dragonbornSibling}         – uses brother/sister based on that Dragonborn's gender.
//
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
    { { "Dragonborn", "Dragonborns", "Drragonborn", "Dragon Scales", "Dragonscales", "smoky", "iron", "metal", "steel", "stone", "Stone" }, ColorHelper::Palette::Dim },
    { { "Asha Scale", "Ember Scale", "Asha", "Ember", "Umbra Ossea", "brother", "sister", "his", "her" }, ColorHelper::Palette::SoftRed },
    { { "Master Bates", "Bates", "Ominous Castle", "castle", "shady figure", "Seminiferous" }, ColorHelper::Palette::DarkPurple },
    { { "Noah", "Noah Lott", "Village Elder" "Tory Tailor", "Wanda Rinn", "Wanda", "Will Figsid", "Will", "Blacksmith", "smithcraft" }, ColorHelper::Palette::PurpleBlue },
    { { "Fire Dragon", "hottest", "hot", "burning", "crackling embers", "wildfire", "heat", "wall of flames", "Flames", "Fire", "Blyathyroid" }, ColorHelper::Palette::FireDragon },
    { { "Water Dragon", "surface", "deep waters", "droplets", "flows", "riverbed", "currents", "cold stream", "Lacrimere" }, ColorHelper::Palette::WaterDragon },
    { { "Earth Dragon", "bedrock", "mountain wall", "rumbles", "shifting earth", "puddle of mud", "Cladrenal" }, ColorHelper::Palette::EarthDragon },
    { { "Air Dragon", "airy", "breeze", "swirling gusts", "wind", "winds", "Aerobronchi" }, ColorHelper::Palette::AirDragon },
    { { "Gonad", "Glandular", "Perigonal", "Holmabir", "Kattkavar", "Stiggedin", "destination", "village", "weapon", "weapons", "histories", "cores", "core" "spatial magic", "Who vants tyo be", "Who vants to be", "Who wants-a to be a", "Who wants to be a" }, ColorHelper::Palette::TitleAccent },
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

namespace {
    inline std::vector<std::pair<std::string, sf::Color>> dynamicColorTokens;
}

inline std::vector<std::pair<std::string, sf::Color>> colorTokensWithDynamic() {
    auto tokens = colorTokens();
    tokens.insert(tokens.end(), dynamicColorTokens.begin(), dynamicColorTokens.end());
    return tokens;
}

inline void refreshDynamicDragonbornTokens(
    std::string playerName,
    std::string dragonbornName,
    std::string dragonbornSubject,
    std::string dragonbornObject,
    std::string dragonbornPossessive,
    std::string dragonbornPossessivePronoun,
    std::string dragonbornOwnPossessive,
    std::string dragonbornSibling,
    std::string dragonbornSiblingName
) {
    dynamicColorTokens.clear();
    auto add = [&](const std::string& value) {
        if (!value.empty())
            dynamicColorTokens.emplace_back(value, ColorHelper::Palette::SoftRed);
    };

    add(playerName);
    add(dragonbornName);
    add(dragonbornSubject);
    add(dragonbornObject);
    add(dragonbornPossessive);
    add(dragonbornPossessivePronoun);
    add(dragonbornOwnPossessive);
    add(dragonbornSibling);
    add(dragonbornSiblingName);
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
        "Vhat is da solyushun to zis riddle?",
        "Da! Flames roar for this correct answer!"
    },
    QuizSpeech{
        TextStyles::SpeakerId::WaterDragon,
        "Vat is ze solution to zis riddle?",
        "Jawoll! Ze currents applaud your logic!"
    },
    QuizSpeech{
        TextStyles::SpeakerId::EarthDragon,
        "What's the solution to this riddle?",
        "YEEHAW! Yer reasoning is solid as bedrock!"
    },
    QuizSpeech{
        TextStyles::SpeakerId::AirDragon,
        "What-a is the solutione to dis riddle?",
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
    static const std::string kDefault = "What is the solution to this riddle?";
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
// This is were the story starts and the player meets its first companion: Wanda Rinn
inline const std::vector<DialogueLine> perigonal = {
    { Speaker::NoNameWanderer, "Hey...! Are you alright? Wake up...." },
    { Speaker::NoNameWanderer, "Easy now - don't move. You're bleeding." },
    { Speaker::NoNameWanderer, "My name is Wanda Rinn. What is your name?", true }, // Name Insertion
    { Speaker::Player, "Ugghhhh.... My whole body hurts..." },
    { Speaker::VillageWanderer, "Here, drink. This will stitch you up in no time." }, // Potion Handling Needed here, fill players health back up to 100%
    { Speaker::VillageWanderer, "What happened to you {playerName}?" },
    { Speaker::Player, "This place....? Where am I...?" },
    { Speaker::VillageWanderer, "You are in the Fields of Petrigonal in Glandular." },
    { Speaker::Player, "Glandular? We used to speak that name carefully... Like a place you don't step into - only talk about." },
    { Speaker::VillageWanderer, "We...? What do you mean by we?" },
    { Speaker::Player, "The residents of Umbra Ossea - where I come from." },
    { Speaker::VillageWanderer, "Umbra Ossea...? You say that as if it were a real place." },
    { Speaker::Player, "It is. Or at least... it was." },
    { Speaker::VillageWanderer, "People around here use that name differently. As a... tale. Something you don't expect to be true." },
    { Speaker::Player, "That's how we spoke of Glandular. A world beyond the veil. A place you never return from." },
    { Speaker::VillageWanderer, "...So both of us grew up listening to lies." },
    { Speaker::Player, "Or half-truths." },
    { Speaker::VillageWanderer, "Maybe... Or both worlds were just afraid of each other." },
    { Speaker::VillageWanderer, "And you? How did you cross that veil?" },
    { Speaker::Player, "I didn't mean to. An evil sorcerer by the name Master Bates found a way in." },
    { Speaker::VillageWanderer, "Master Bates?! That name has heavy weight here." },
    { Speaker::VillageWanderer, "He tried to overrule the dragons regions for a long time and has brought nothing but chaos." },
    { Speaker::Player, "Sounds familar. He attacked us, while I was training with my {dragonbornSibling}, {dragonbornSiblingName}." },
    { Speaker::Player, "He clearly didn't mean to overrule us - but steal from us and erase us..." },
    { Speaker::VillageWanderer, "What happened to your {dragonbornSibling}, ...{dragonbornSiblingName}?" },
    { Speaker::Player, "{dragonbornSiblingName} was hit first. I didn't think - I just charged in." },
    { Speaker::Player, "The next thing I remember is you waking me up... {dragonbornSiblingName} must have used {dragonbornPossessive} Spatial Magic." },
    { Speaker::Player, "A blind jump - Glandular was never meant to be the destination... I guess." },
    { Speaker::VillageWanderer, "Or maybe it was. If you're telling the truth... then we're all in danger." },
    { Speaker::Player, "Here, I'll prove it **Pulls back sleeve, scales glinting** These aren't scars. They are..." },
    { Speaker::VillageWanderer, "Dragon Scales! Only true Dragonborns have them! So the legends tend to have some weight in them." },
    { Speaker::Player, "I need to go back and save {dragonbornSiblingName}. Where are my weapons?", false, false, true },
    { Speaker::VillageWanderer, "They are here. But they've been scattered. I will take care of them." },
    { Speaker::VillageWanderer, "There is a village nearby named Gonad. A mighty Blacksmith resides there. His name is Will Figsid." },
    { Speaker::VillageWanderer, "He is well known across Glandular for his smithcraft. I'm sure he can help!", false, false, true, DialogueLineAction::StartsQuest, std::optional<std::string>{"Forged Destiny"} },
    { Speaker::VillageWanderer, "Get up {playerName}. Gonad isn't far - and if Master Bates is involved, we don't have much time." },
    { Speaker::StoryTeller, "Wanda Rinn is a very helpful companion. She will handle your inventory and can give you hints and explanations along your travel." },
    { Speaker::StoryTeller, "You can open your inventory through this menu button." },
    { Speaker::StoryTeller, "This is all for now. Once a new Menu Shortcut unlocks, I will explain it to you." },
};
// First part of the Gonad Dialogue. This is where the player meets Noah Lott
inline const std::vector<DialogueLine> gonad_part_one = {
    { Speaker::VillageWanderer, "And here we are - Gonad. Peaceful, steady and alive." },
    { Speaker::VillageWanderer, "Not many places in Glandular are like this." },
    { Speaker::Player, "It feels... safe. Like nothing's chasing me here." },
    { Speaker::VillageWanderer, "I know what you mean. People work, trade, eat together - and sleep with two shut eyes." },
    { Speaker::VillageWanderer, "We should see Noah Lott first. He's the Elder of this Village." },
    { Speaker::VillageWanderer, "He knows Glandular better than most. And he still owes me a favor." },
    { Speaker::Player, "A good one?" },
    { Speaker::VillageWanderer, "A reliable one." },
    { Speaker::Player, "So this is a real village. Not just a stop on the road." },
    { Speaker::VillageWanderer, "Gonad doesn't survive by accident. You'll see." },
    { Speaker::VillageElder, "Wanda Rinn! I had a feeling you'd return - you always do." },
    { Speaker::VillageWanderer, "Some places pull harder than others." },
    { Speaker::VillageElder, "And who's your escort?" },
    { Speaker::Player, "My name is {playerName}. I woke up near the village. I don't remember how I got there." },
    { Speaker::VillageWanderer, "Found him with his gear in pieces. All of it." },
    { Speaker::VillageElder, "Hmmmm... You won't get far in Glandular like that." },
    { Speaker::VillageWanderer, "True. Maybe we can resolve the favor you still owe me with a discounted visit at Figsid's Forge?" },
    { Speaker::VillageElder, "And straight to business - That's the Wanda I know! hahaha!" },
    { Speaker::VillageElder, "I've already informed him of your coming - It is good to know a lot hahaha!" },
    { Speaker::VillageElder, "If anyone can help you, it's him. I'll make sure he gives you his full attention." },
    { Speaker::Player, "This is much more than I hoped for. Thank you!" },
    { Speaker::VillageElder, "You will find his forge at the end of this road. Wanda knows the way." },
    { Speaker::VillageWanderer, "Until next favor Noah! Always a pleasure making deals with you." },
    { Speaker::VillageElder, "Hopefully next time you'll have a favor to return!" },
    { Speaker::VillageWanderer, "We will see! Come on, {playerName} - the forge is just ahead!" },
    { Speaker::Player, "Yes, let's go!" },
};
// Will Figsid's Forge. Here, the players weapon will be repaired - but only one!
inline const std::vector<DialogueLine> blacksmith = {
    { Speaker::Blacksmith, "Welcome to my forge. Noah already told me you'd come. Tell me your reason." },
    { Speaker::Player, "He said you might be able to help with my weapons." },
    { Speaker::Blacksmith, "Maybe. Show me what you brought." },
    { Speaker::Player, "...It's not pretty. All my weapons are broken." },
    { Speaker::Blacksmith, "Let me see.......... Hmmmmm.......I see..........." },
    { Speaker::Blacksmith, "Okay......... Hmmmmm....... Alright........." },
    { Speaker::Blacksmith, "Uhhhh......... Hmmmmm....... Interesting........." },
    { Speaker::Blacksmith, ".....These weapons are done." },
    { Speaker::Player, "Done as in... repaired?" },
    { Speaker::Blacksmith, "Done as in... finished. Their shapes are ruined. But the cores haven't gone silent yet." },
    { Speaker::Player, "So there's no saving them...?" },
    { Speaker::Blacksmith, "Not as they are. Trying would only give you something that breaks when you need it most." },
    { Speaker::Player, "Then what can you do?" },
    { Speaker::Blacksmith, "Three weapons. Three histories. Three cores. And enough metal to create something new." },
    { Speaker::Blacksmith, "I can take what remains and forge only ONE new weapon - but stronger than any of these ever were." },
    { Speaker::Player, "...So I will have to choose." },
    { Speaker::Blacksmith, "Yes. I can shape the steel into the kind of weapon you want to carry forward." },
    { Speaker::VillageWanderer, "This is one of those moments where you have to let your loved one go, isn't it?" },
    { Speaker::Player, "Yeah, it feels like it." },
    { Speaker::Blacksmith, "The forge is hot. Which weapon shall I create?" }, // New WeaponSelection UI, take broken weapons out of players inventory
    { Speaker::Blacksmith, "This will take some time. Feel free to get some rest. I will wake you when I'm done." }, // When Weapon is Selected, after this Line 5 second black screen
    { Speaker::Blacksmith, "Wake up, I'm done. This is your new {weaponName}.", false, false, true, DialogueLineAction::CompletesQuest, std::nullopt, std::optional<std::string>{"Forged Destiny"} }, // Give new weapon to Player, display it in a popup
    { Speaker::Player, "This... is better than I expected." },
    { Speaker::Blacksmith, "Expectation is what brought you here in the first place." },
    { Speaker::Blacksmith, "What you presented to me was broken. What you're holding now is flawless." },
    { Speaker::Player, "It feels... very different." },
    { Speaker::Blacksmith, "It should. Although this weapon is stronger, you yourself haven't changed during your beauty sleep." },
    { Speaker::Blacksmith, "You should go back to Noah Lott and ask him about the Elemental Dragons." },
    { Speaker::Blacksmith, "They might know a way to strengthen your mind, your body an your soul." },
    { Speaker::VillageWanderer, "Seems like you got a tough path ahead of you if you want to save your {dragonbornSibling}." },
    { Speaker::Player, "Then I shall walk it." },
    { Speaker::VillageWanderer, "Good answer! We should head back to Noah Lott now." }, // Fade into gonad_part_two
};
// Secon part of the Gonad Dialogue. This is where the player receives a map
inline const std::vector<DialogueLine> gonad_part_two = {
    { Speaker::VillageElder, "You're back. I see Will kept his word.", false, false, true, DialogueLineAction::StartsQuest, std::optional<std::string>{"Map of Many Lands"}, std::nullopt },
    { Speaker::Player, "He did. This weapon is... something else." },
    { Speaker::VillageElder, "Good. Then listen carefully." },
    { Speaker::VillageElder, "Glandular holds many lands - but four regions stand apart, shaped by the Elemental Dragons." },
    { Speaker::VillageElder, "They keep the balance. They also test those who walk their paths." },
    { Speaker::VillageElder, "Take this." }, // Give Map of Glandular here
    { Speaker::VillageElder, "Not all roads wish to be found. This map shows the ones that do." }, // Open Map at the end of this line
    { Speaker::StoryTeller, "And just like that, the world opens." },
    { Speaker::StoryTeller, "But before you rush off - let's make sure you know what you're carrying." },
    { Speaker::StoryTeller, "To the south lies Aerobronchi.", false, false, true, DialogueLineAction::StartsQuest, std::optional<std::string>{"Trial of Soul"} },
    { Speaker::StoryTeller, "Land of wind and open skies." },
    { Speaker::StoryTeller, "Its dragon is Gustavo Windimaess." }, // Fade in Air Dragon Asset
    { Speaker::StoryTeller, "Quick-tongued. Proud. He enjoys watching people lose their breath." }, // Unhighlight Aerobronchi, Fade Out Air Dragon Asset
    { Speaker::StoryTeller, "To the east stands Blyathyroid.", false, false, true, DialogueLineAction::StartsQuest, std::optional<std::string>{"Trial of Resolve"} }, // Highlight Blyathyroid
    { Speaker::StoryTeller, "Fire, stone, and pressure." }, 
    { Speaker::StoryTeller, "Rowsted Sheacane rules there." }, // Fade in Fire Dragon Asset
    { Speaker::StoryTeller, "He respects strength - and nothing else." }, // Unhighlight Blyathyroid, fade out Fire Dragon Asset
    { Speaker::StoryTeller, "To the west lies Cladrenal.", false, false, true, DialogueLineAction::StartsQuest, std::optional<std::string>{"Trial of Body"} }, // Highlight Cladrenal
    { Speaker::StoryTeller, "Solid ground. Heavy silence." },
    { Speaker::StoryTeller, "Grounded Claymore watches over it." }, // Fade in Earth Dragon Asset
    { Speaker::StoryTeller, "Straightforward. Fair. But unmoving once decided." }, // Unhighlight Cladrenal, Fade out Earth Dragon Asset
    { Speaker::StoryTeller, "And to the north... Lacrimere.", false, false, true, DialogueLineAction::StartsQuest, std::optional<std::string>{"Trial of Mind"} }, // Highlight Lacrimere
    { Speaker::StoryTeller, "Cold waters and deeper emotions." },
    { Speaker::StoryTeller, "Flawtin Seamen dwells there." }, // Fade in Water Dragon Asset
    { Speaker::StoryTeller, "He listens closely - sometimes too closely." }, // Unhighlight Lacrimere, Fade out Water Dragon Asset
    { Speaker::Player, "And they will all test me?" },
    { Speaker::VillageElder, "Yes. The trials are the same in nature. But the dragons are not." },
    { Speaker::VillageElder, "Each will judge you through their own temper." },
    { Speaker::VillageElder, "Soul, resolve, body or mind." },
    { Speaker::Player, "And if I fail?" },
    { Speaker::VillageElder, "You won't. Not because you shouldn't, but because you can't." },
    { Speaker::VillageElder, "Your weapon is ready. The paths are open. It's up to you to decide your fate." },
    { Speaker::VillageElder, "And remember - the dragons do not change you. They only reveal what has yet to awaken." },
    { Speaker::Player, "A map... and a chance to save {dragonbornSiblingName}." },
    { Speaker::VillageWanderer, "We are all set now. Let's open the map and choose our first destination.", false, false, true, DialogueLineAction::OpensMapFromMenu | DialogueLineAction::CompletesQuest, std::nullopt, std::optional<std::string>{"Map of Many Lands"} },
};

// Third Gonad dialogue that bridges the Dragon Stones with the final confrontation.
inline const std::vector<DialogueLine> gonad_part_three = {
    { Speaker::VillageElder, "The dragons refined your Soul, Mind, Body and Resolve. I can feel the lessons they've carved into you." },
    { Speaker::VillageElder, "Have you finally found a way back to Umbra Ossea?" },
    { Speaker::Player, "The Umbra Ossea map is finally clear to me.", false, false, true, DialogueLineAction::OpensMapFromMenu },
    { Speaker::StoryTeller, "The chart ignites with light as you present it to Noah." },
    { Speaker::StoryTeller, "You should go now, your homeland is waiting for you!", false, false, true, DialogueLineAction::StartsSeminiferousTeleport },
};


// Extended village narrator text describing each dragon’s personality.
inline const std::vector<DialogueLine> dragon = {
#if 0
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
#endif
};

inline const std::vector<DialogueLine> gonad = {};
inline const std::vector<DialogueLine> weapon = {};

// Greeting lines shown when the player returns to Gonad after a dungeon run.
inline const std::vector<DialogueLine> gonadWelcomeBack = {
    { Speaker::VillageElder, "Welcome back, {playerName}! You made it out of {lastLocation}!" },
    { Speaker::VillageElder, "I see you've retrieved the Dragon Stone from {lastDragonName}." },
    { Speaker::VillageElder, "Catch your breath, then head toward your next destination!", false, false, true, DialogueLineAction::OpensMapFromMenu },
};

// Dialogue triggered once you survive the teleport to Seminiferous.
inline const std::vector<DialogueLine> seminiferous_part_one = {
    { Speaker::StoryTeller, "Upon reaching ground, you freeze for a minute.", false, false, true, DialogueLineAction::StartsQuest, std::optional<std::string>{"Face of the Master"}, std::nullopt },
    { Speaker::StoryTeller, "This is definitely Umbra Ossea. But right where the temple used to be stands a dark, ominous Castle." },
    { Speaker::StoryTeller, "A shady figure steps out of the castle greeting you with an evil smirk." },
    { Speaker::MasterBates, "So you have survived, {playerName}! Ready to die for real this time?" },
    { Speaker::Player, "Set my {dragonbornSibling} free, Master Bates! And leave Umbra Ossea to the people it belongs to!" },
    { Speaker::MasterBates, "Hahaha, did you really think I'd listen to a spoiled little brat with no powers?!" },
    { Speaker::Player, "Then I got no choice. I will fight for the sake of Umbra Ossea and {dragonbornSiblingName}!" },
    { Speaker::MasterBates, "Come at me, brooooooooooooooo...!", false, false, true, DialogueLineAction::StartsBattle },
};

inline const std::vector<DialogueLine> seminiferous_part_two = {
    { Speaker::MasterBates, "Ughhh....... damn it..... If you and your little friends wouldn't have come.....!!!", false, false, true, DialogueLineAction::CompletesQuest, std::nullopt, std::optional<std::string>{"Face of the Master"} },
    { Speaker::Player, "Your time is over! Time to pay the price for your wrong-doing!" },
    { Speaker::MasterBates, "No... this power... I was so close...!" },
    { Speaker::Player, "Master Bates. Your reign ends here." },
    { Speaker::StoryTeller, "The castle trembles as Master Bates falls silent." },
    { Speaker::VillageElder, "You did it, {playerName}. The darkness over Umbra Ossea is broken." },
    { Speaker::VillageWanderer, "I knew you could bring everyone back. Well done." },
    { Speaker::Blacksmith, "You handled that weapon real good!" },
    { Speaker::StoryTeller, "Together, the companions lift you up to cheer for your Glorious Victory!" },
    { Speaker::Player, "{dragonbornSiblingName}... you're back. I won't let you go again." },
    { Speaker::DragonbornSibling, "Thank you, {dragonbornSibling} for saving me and our home!" },
    { Speaker::StoryTeller, "{dragonbornSiblingName} smiles and takes your hand as the journey home begins." },
};

// Closing thank-you lines played after beating the game.
inline const std::vector<DialogueLine> finalThanks = {
    { Speaker::StoryTeller, "Thank you {playerName} for playing my SFML text adventure game!" },
    { Speaker::StoryTeller, "I hope you enjoyed it!" },
};

// Dialogue branch shown when the player chooses to kill Noah Bates.
inline const std::vector<DialogueLine> finalChoiceKill = {
    { Speaker::Player, "The world is a better place without you!" },
    { Speaker::StoryTeller, "You are using {weapon} to kill Master Bates!" },
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
    { Speaker::FireDragon, "Hmh. I smell fear. And soft rresolve. Dragonborn... still very soft, da." },
    { Speaker::Player, "You... know who I am?" },
    { Speaker::FireDragon, "Da. I know vho you ahre, little Drragonborn. I smell your fear and your doubt." },
    { Speaker::Player, "I wanted to protect my {dragonbornSibling} and my home. But I failed..." },
    { Speaker::Player, "In the end, I lost everything... What do I lack?" },
    { Speaker::FireDragon, "You think you lost everything, but you still carry anger, fear, and hope. You ahre unfinished, da." },
    { Speaker::Player, "It feels like the fire inside me eats me from within." },
    { Speaker::FireDragon, "Da. Because it has nowhere to go. Your scales too thin, your claws not sharp. Heat leaks everywhere." },
    { Speaker::FireDragon, "Vhen pain comes, you pull back. Vhen guilt comes, you scratch once... and then stop." },
    { Speaker::Player, "So what should I do... fight harder?" },
    { Speaker::FireDragon, "Not just harrrder. Longer. Cleaner." },
    { Speaker::FireDragon, "Rresolve is not vild flame. Rresolve is controlled fire that refuses to go out." },
    { Speaker::FireDragon, "It stays vhen body shakes, vhen mind screams to run, vhen heart says 'enough'." },
    { Speaker::Player, "And if I fail again?" },
    { Speaker::FireDragon, "You vill. Many times, da." },
    { Speaker::FireDragon, "But each time you stand again, you hammerrr your rresolve." },
    { Speaker::FireDragon, "Each time you stay in heat, your scales grow thicker and your claws more sure." },
    { Speaker::Player, "Do you really think I can reach them?" },
    { Speaker::FireDragon, "Da. I vould not vaste breath on dead emberrr." },
    { Speaker::FireDragon, "I see spark that refuses to die. If you learn to hold, that spark vill become blaze." },
    { Speaker::FireDragon, "Now enough vords. Fire does not promise. Fire tests. So........" },
    { Speaker::FireDragon, "Wyelcome byeck to dis shyow: Who vants tyo be Drragonborn? Dis is contestant {playerName}, he searches for strong rresolve!", false, false, true, DialogueLineAction::StartsQuiz },
    { Speaker::FireDragon, "Vhat is my name?" }, // starts the question input
    { Speaker::FireDragon, "VRRONG! Try again." }, // Only if wrong
    { Speaker::FireDragon, "Da! You solved it! My name is Rrowsted Sheacane!" },
    { Speaker::FireDragon, "Aaaannd we have a winnnneeeer!", false, false, true, DialogueLineAction::FinalCheer }, // End of Quiz
    { Speaker::FireDragon, "Remember dis heat vhen your rresolve tries to crack again, da." },
    { Speaker::FireDragon, "Even ze finest steel must pass thrrrough ze hottest fire." },
    { Speaker::Player, "Thank you, {fireDragon}! I will keep going." },
};


inline const std::vector<DialogueLine> waterdragon = {
    { Speaker::StoryTeller, "Cold water splashes up as you slip on wet stone." },
    { Speaker::StoryTeller, "This must be Lacrimere, where the Water Dragon resides." },
    { Speaker::WaterDragon, "Hm. Your head is loud. Zoughts everywhere. Dragonborn, ja. But mind... very unfocused." },
    { Speaker::Player, "You... know who I am?" },
    { Speaker::WaterDragon, "Ja. I know vho you are, little Dragonborn. Your fear is not ze problem. Your zinking is." },
    { Speaker::Player, "I wanted to protect my {dragonbornSibling} and my home." },
    { Speaker::Player, "I keep seeing that moment. Over and over. What is wrong with me...?" },
    { Speaker::WaterDragon, "You are not broken. And you are not wrong. You are drowning in your own mind, ja." },
    { Speaker::Player, "Drowning...?" },
    { Speaker::WaterDragon, "You replay every choice until ze vater turns to fog." },
    { Speaker::WaterDragon, "You blame yourself for every vave you could not stop." },
    { Speaker::Player, "Then what do I need?" },
    { Speaker::WaterDragon, "You need clear mind. Not less feeling. Less noise." },
    { Speaker::WaterDragon, "Mind is like vater. It reflects ze trus vhen it is still." },
    { Speaker::WaterDragon, "But you stir it all day... and zen believe ze ripples." },
    { Speaker::Player, "Can that change?" },
    { Speaker::WaterDragon, "Ja. You learn to let zoughts pass, instead of chasing each one." },
    { Speaker::WaterDragon, "You choose vhat to hold, and vhat to let flow avay." },
    { Speaker::Player, "And if I start to drown in it again?" },
    { Speaker::WaterDragon, "Zen you stop. You breathe. You focus on one thing zat matters." },
    { Speaker::WaterDragon, "You do not have to think of all losses at once. Just ze next step to reach zem." },
    { Speaker::Player, "Do you think I can reach them?" },
    { Speaker::WaterDragon, "Ja. If you learn to see clearly." },
    { Speaker::WaterDragon, "Vith clear mind, you vill see paths zat panic hides from you." },
    { Speaker::WaterDragon, "So. Enough theory, ja? Zoughts are easy. Focus is not." },
    { Speaker::WaterDragon, "Velcome back to zis show: Who vants to be Dragonborn? Zis is contestant {playerName}, searching for clear mind!", false, false, true, DialogueLineAction::StartsQuiz },
    { Speaker::WaterDragon, "Vhat is my name?" }, // starts the question input
    { Speaker::WaterDragon, "Vrond. Focus. Try again." }, // Only if wrong
    { Speaker::WaterDragon, "Jawoll. You are correct. My name is Flawtin Seamen." },
    { Speaker::WaterDragon, "Aaand ve have a vinnner!", false, false, true, DialogueLineAction::FinalCheer }, // End of Quiz
    { Speaker::WaterDragon, "Remember zis vater vhen your mind turns to fog again, ja." },
    { Speaker::WaterDragon, "Don't push ze river - it flows by itself." },
    { Speaker::Player, "Thank you, {waterDragon}. I will try to see clearly." },
};


inline const std::vector<DialogueLine> earthdragon = {
    { Speaker::StoryTeller, "You step into cold mud. It holds your boots for a second before letting go." },
    { Speaker::StoryTeller, "This must be Cladrenal, where the Earth Dragon resides." },
    { Speaker::EarthDragon, "Heh. Careful there. Ground bites back if you don't respect it." },
    { Speaker::Player, "You know who I am...?" },
    { Speaker::EarthDragon, "Sure do. Dragonborn." },
    { Speaker::EarthDragon, "You walk light. Too light. Like you ain't sure you belong here." },
    { Speaker::Player, "I wanted to protect my {dragonbornSibling} and my home." },
    { Speaker::Player, "But I could not hold. Everything slipped away." },
    { Speaker::EarthDragon, "Yeah. I can see that." },
    { Speaker::EarthDragon, "Your body moved. But nothin' in you set itself down." },
    { Speaker::Player, "What do I lack?" },
    { Speaker::EarthDragon, "Weight." },
    { Speaker::EarthDragon, "Not size. Not muscle. The kind that says: I stay." },
    { Speaker::Player, "Stay... even when it hurts?" },
    { Speaker::EarthDragon, "Especially then." },
    { Speaker::EarthDragon, "A tree don't pull up roots just 'cause wind gets loud." },
    { Speaker::Player, "Sometimes I feel like I just fall apart." },
    { Speaker::EarthDragon, "Then you learn to fall like stone. Down. Still. Then stand." },
    { Speaker::EarthDragon, "Body first. Breath. Ground. Then everythin' else follows." },
    { Speaker::Player, "Can that change for me?" },
    { Speaker::EarthDragon, "Sure can. But not by runnin'." },
    { Speaker::EarthDragon, "You slow down 'n you plant your feet. You finish what your hands start." },
    { Speaker::Player, "And if I lose again?" },
    { Speaker::EarthDragon, "Then you get back up slower... but steadier." },
    { Speaker::EarthDragon, "Every time you rise, the ground beneath you remembers." },
    { Speaker::Player, "Do you think I can stand strong enough to protect them?" },
    { Speaker::EarthDragon, "Yeah. If you learn to stay put." },
    { Speaker::EarthDragon, "I see a frame that ain't filled yet. But it can hold plenty." },
    { Speaker::EarthDragon, "Alright. Enough talk." },
    { Speaker::EarthDragon, "Earth don't brag. Earth tests." },
    { Speaker::EarthDragon, "Welcome back to the show: Who wants to be a Dragonborn? Today's contestant is {playerName}, lookin' for solid ground!", false, false, true, DialogueLineAction::StartsQuiz },
    { Speaker::EarthDragon, "What's my name?" }, // starts the question input
    { Speaker::EarthDragon, "Nope. Try again. Put some weight in that answer." }, // Only if wrong
    { Speaker::EarthDragon, "Yeehaw. That's right. My name is Grounded Claymore." },
    { Speaker::EarthDragon, "Aaaannd we got ourselves a winner!", false, false, true, DialogueLineAction::FinalCheer }, // End of Quiz
    { Speaker::EarthDragon, "Remember this ground when your knees want to quit." },
    { Speaker::EarthDragon, "Walk as if you are kissin' the earth with your feet." },
    { Speaker::Player, "Thank you, {earthDragon}. I will try to stand firm." },
};


inline const std::vector<DialogueLine> airdragon = {
    { Speaker::StoryTeller, "A warm current lifts dust from the ground as you arrive." },
    { Speaker::StoryTeller, "This must be Aerobronchi, where the Air Dragon resides." },
    { Speaker::AirDragon, "Ah... a traveler carried by many winds." },
    { Speaker::AirDragon, "And yet, you do not know which one is yours, si?" },
    { Speaker::Player, "You know who I am?" },
    { Speaker::AirDragon, "Si. Dragonborn." },
    { Speaker::AirDragon, "But your soul... it drifts. It wants ten directions at once." },
    { Speaker::Player, "I wanted to protect my {dragonbornSibling} and my home." },
    { Speaker::Player, "Now I just feel lost. Pulled everywhere at the same time." },
    { Speaker::AirDragon, "Dis happens when heart and path do not speak together." },
    { Speaker::AirDragon, "You run where fear blows you, not where you choose to go." },
    { Speaker::Player, "Then what do I lack?" },
    { Speaker::AirDragon, "Anchor. Not chains. A center." },
    { Speaker::AirDragon, "Soul is not about floating higher. It is about knowing where you return." },
    { Speaker::Player, "Sometimes I do not even know who I am anymore." },
    { Speaker::AirDragon, "Good. Question is doorway." },
    { Speaker::AirDragon, "When you forget, you listen. To breath. To memory. To what still matters." },
    { Speaker::Player, "Will I ever feel whole again?" },
    { Speaker::AirDragon, "Si. When your choices line up with your truth." },
    { Speaker::AirDragon, "Not with fear. Not with guilt. With truth." },
    { Speaker::Player, "And if I lose myself again?" },
    { Speaker::AirDragon, "Then you stop. You turn. You find de wind that is yours... and you follow only dat one." },
    { Speaker::Player, "Do you think I can save them?" },
    { Speaker::AirDragon, "I think your soul already knows de way." },
    { Speaker::AirDragon, "You only need to stop chasing every other path." },
    { Speaker::AirDragon, "Bene. Enough talk." },
    { Speaker::AirDragon, "Air does not argue. Air reveals." },
    { Speaker::AirDragon, "Welcome back to de show: Who wants to be a Dragonborn? Our contestant today is {playerName}, searching for true soul!", false, false, true, DialogueLineAction::StartsQuiz },
    { Speaker::AirDragon, "What is my name?" }, // starts the question input
    { Speaker::AirDragon, "No no. Wrong wind. Try again." }, // Only if wrong
    { Speaker::AirDragon, "Bravissimo. You are correct. My name is Gustavo Windimaess." },
    { Speaker::AirDragon, "Aaand we have a winner!", false, false, true, DialogueLineAction::FinalCheer }, // End of Quiz
    { Speaker::AirDragon, "Your soul is calm now. Like a feather in de air." },
    { Speaker::AirDragon, "Only de wind knows where it will carry our dandelion souls." },
    { Speaker::Player, "Thank you, {airDragon}. I will try to listen to it." },
};


inline const std::vector<DialogueLine> dragonstone = {
    { Speaker::StoryTeller, "You have received many useful things. Noah Lott really spoke the truth!" },
    { Speaker::StoryTeller, "You will now be teleported back to Gonad." },
};
