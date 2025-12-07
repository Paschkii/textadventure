#pragma once
#include <vector>
#include "dialogueLine.hpp"

using Speaker = TextStyles::SpeakerId;

inline const std::vector<DialogueLine> intro = {
    { Speaker::NoNameNPC, "Welcome to this tiny fictional adventure." },
    { Speaker::NoNameNPC, "I am Tory Tailor, your guide and storyteller." },
    { Speaker::StoryTeller, "I will help you on your journey." },
    { Speaker::StoryTeller, "Before we begin, what is your name?", true },
    { Speaker::StoryTeller, "Alright, {playerName}! Let's get started!" },
    { Speaker::StoryTeller, "You will now step into the world of Glandular." },
    { Speaker::StoryTeller, "This is a world of magic, dragons, and adventure." },
    { Speaker::StoryTeller, "You are a Dragonborn, a hero destined to save the world from evil." },
    { Speaker::StoryTeller, "You will face many challenges and make important choices along the way." },
    { Speaker::StoryTeller, "But fear not, for you are not alone. You will have allies to help you on your journey." },
    { Speaker::StoryTeller, "Let's jump right in!" }
};

inline const std::vector<DialogueLine> gonad = {
    { Speaker::VillageNPC, "Hey, {playerName}. Are you awake?" },
    { Speaker::VillageNPC, "You have been asleep for a long time." },
    { Speaker::VillageNPC, "I was starting to worry about you." },
    { Speaker::VillageNPC, "You are in a small village called Gonad." },
    { Speaker::VillageNPC, "I am Noah Lott, the Elder of this village." },
    { Speaker::VillageNPC, "How I know you? {playerName}, you are a Dragonborn!" },
    { Speaker::VillageNPC, "Your skin is covered in Dragon Scales!" },
    { Speaker::VillageNPC, "And you are the only Dragonborn, that hasnt been captured by Master Bates, yet..." },
    { Speaker::VillageNPC, "So it was an easy guess for me!" },
};

inline const std::vector<DialogueLine> weapon = {
    { Speaker::VillageNPC, "Oh! And I found this weapon beside your body. Can you tell me its name?" }
};

inline const std::vector<DialogueLine> dragon = {
    { Speaker::VillageNPC, "You have to ask the dragons for their help. These are the 4 dragons:" },
    { Speaker::VillageNPC, "Rowsted Sheacane - The Fire Dragon" },
    { Speaker::VillageNPC, "This is the hottest temper among the Four, burning through every room he enters." },
    { Speaker::VillageNPC, "His tongue is thick-rolled and smoky, spitting sounds like crackling embers - every R erupts like a small wildfire." },
    { Speaker::VillageNPC, "Blunt, heavy, and scorching, his words hit with the heat of molten iron." },
    { Speaker::VillageNPC, "Flawtin Seamen - The Water Dragon" },
    { Speaker::VillageNPC, "Calm on the surface, tidal underneath - he carries the quiet weight of deep waters." },
    { Speaker::VillageNPC, "His tongue is sharp, crisp, and clean-cut, with consonants hitting like droplets on metal." },
    { Speaker::VillageNPC, "Every sentence flows steady and precise, like a cold stream running over polished stone." },
    { Speaker::VillageNPC, "Grounded Claymore - The Earth Dragon" },
    { Speaker::VillageNPC, "Steady as bedrock and stubborn as a mountain wall." },
    { Speaker::VillageNPC, "His tongue is broad, relaxed, and gravel-warm, sliding through words with a slow drawl that rumbles like shifting earth." },
    { Speaker::VillageNPC, "He speaks in solid chunks, every syllable dropping like a stone." },
    { Speaker::VillageNPC, "Gustavo Windimaess - The Air Dragon" },
    { Speaker::VillageNPC, "Lively, airy, and constantly in motion - the breeze itself can't keep up with him." },
    { Speaker::VillageNPC, "His tongue is melodic, rolling, and feather-light, dancing through vowels like swirling gusts." },
    { Speaker::VillageNPC, "Every phrase rises and falls like a warm Mediterranean wind." },
};

inline const std::vector<DialogueLine> destination = {
    { Speaker::VillageNPC, "You are all set now. It is up to you to choose your first destination." },
    { Speaker::VillageNPC, "Which dragon do you want to visit first?" },
};

inline const std::vector<DialogueLine> firedragon = {
    { Speaker::StoryTeller, "You find yourself in a hot cave. This must be the Fire Dragon's Lair." },
    { Speaker::FireDragon, "Who dares to step into my cayyve vithout perMEEESHON?!" },
    { Speaker::Player, "I am {playerName}, the Dragonborn. I have come to seek your help." },
    { Speaker::FireDragon, "I know vho you ahre. I vas just yoking! You ahre probably looking for my DRRAGON STONE." },
    { Speaker::FireDragon, "I can give you my DRRAGON STONE, but you must prooof your VORTH first." },
    { Speaker::FireDragon, "You must defeat me in glorious duel!" },
    { Speaker::Player, "A duel? But I am not ready for a duel!" },
    { Speaker::FireDragon, "I vas just yoking again! Is so booooring in here. Heh heh heh!" },
    { Speaker::FireDragon, "I vill give you my DRRAGON STONE if you can ansvehr my qvestion." },
    { Speaker::FireDragon, "Vhat is my name? (Type in your ansvehr)" }, // starts the question input
    { Speaker::FireDragon, "VRRONG! Try again." }, // Only if wrong
    { Speaker::FireDragon, "Da! You ahre korrect! My name is Rowsted Sheacane!" },
    { Speaker::FireDragon, "You ahre smart! Now take my DRRAGON STONE and leave my cayyve!" },
    { Speaker::Player, "Thank you, {fireDragon}! I will not forget this!" },
};

inline const std::vector<DialogueLine> waterdragon = {
    { Speaker::StoryTeller, "You find yourself in a wet cave. This must be the Water Dragon's Lair." },
    { Speaker::FireDragon, "Hu dares to step into mein hole vizout permission?!" },
    { Speaker::Player, "I am {playerName}, the Dragonborn. I have come to seek your help." },
    { Speaker::FireDragon, "I know vho you are. I vas only joking! You are probably looking for mein DRAGON STONE." },
    { Speaker::FireDragon, "I can give you mein DRAGON STONE, but first you must proove your VORTH." },
    { Speaker::FireDragon, "You must defeat me in a duel. Din-Norm regulated, ja!" },
    { Speaker::Player, "A duel? But I am not ready for a duel!" },
    { Speaker::FireDragon, "Sis was nur Fun, ja! It is sehr langweilig hier. Heh heh heh!" },
    { Speaker::FireDragon, "I vill give you mein DRAGON STONE if you can answer mein QVESTION." },
    { Speaker::FireDragon, "Vhat is mein name? (Type in your answer)" }, // starts the question input
    { Speaker::FireDragon, "VRONG! Nochmal." }, // Only if wrong
    { Speaker::FireDragon, "Jawoll! You are correct! Mein name is Flawtin Seamen!" },
    { Speaker::FireDragon, "You are very smart! Now take mein DRAGON STONE and leave mein hole!" },
    { Speaker::Player, "Thank you, {waterDragon}! I will not forget this!" },
};

inline const std::vector<DialogueLine> earthdragon = {
    { Speaker::StoryTeller, "You find yourself in a muddy cave. This must be the Earth Dragon's Lair." },
    { Speaker::FireDragon, "Whooo the heck steps into MAH CAVE without even askin’?!" },
    { Speaker::Player, "I am {playerName}, the Dragonborn. I have come to seek your help." },
    { Speaker::FireDragon, "Aw yeah, I know who ya are! I’m just messin’ with ya! Yer probably lookin’ for my DRAGON STONE." },
    { Speaker::FireDragon, "I can hand it over, sure — but ya gotta prooove yer WORTH first." },
    { Speaker::FireDragon, "Yer gonna hafta defeat me in a duel, partner." },
    { Speaker::Player, "A duel? But I am not ready for a duel!" },
    { Speaker::FireDragon, "Ha! I’m just foolin’ with ya again! It gets real dang borin’ in here. Heh heh heh!" },
    { Speaker::FireDragon, "I’ll give ya my DRAGON STONE if ya answer mah QUESTION right." },
    { Speaker::FireDragon, "What’s mah name? (Type yer answer)" }, // starts the question input
    { Speaker::FireDragon, "Nope! That’s wronger than a snake wearin’ socks! Try again!" }, // Only if wrong
    { Speaker::FireDragon, "YEEHAW! That’s right! Mah name is Grounded Claymore!" },
    { Speaker::FireDragon, "Yer a smart lil’ tumbleweed! Now grab the DRAGON STONE and git outta mah cave!" },
    { Speaker::Player, "Thank you, {earthDragon}! I will not forget this!" },
};

inline const std::vector<DialogueLine> airdragon = {
    { Speaker::StoryTeller, "You find yourself in a windy cave. This must be the Air Dragon's Lair." },
    { Speaker::FireDragon, "Who-a dares to step into-a MY CAVE without-a PERMISSIONE?!" },
    { Speaker::Player, "I am {playerName}, the Dragonborn. I have come to seek your help." },
    { Speaker::FireDragon, "Aaaah, I know-a who you are! I was-a just joking! You come for my DRAGON STONE, sì sì!" },
    { Speaker::FireDragon, "I can-a give you my DRAGON STONE, but first you must-a prooove your Valore!" },
    { Speaker::FireDragon, "You must defeat-a me in a duel — molto dramatico!" },
    { Speaker::Player, "A duel? But I am not ready for a duel!" },
    { Speaker::FireDragon, "I was-a only joking again! It is SO BORING in here! Heh heh heh-a!" },
    { Speaker::FireDragon, "I will-a give you my DRAGON STONE if you can answer mio QUESTIONE!" },
    { Speaker::FireDragon, "What-a is my name? (Type-a your answer)" }, // starts the question input
    { Speaker::FireDragon, "NOOO! Sbagliata! Try-a again!" }, // Only if wrong
    { Speaker::FireDragon, "BRAVISSIMO! You are correct-a! My name is Gustavo Windimaess!" },
    { Speaker::FireDragon, "You are molto smart-a! Now take-a my DRAGON STONE and leave-a my cave!" },
    { Speaker::Player, "Thank you, {airDragon}! I will not forget this!" },
};

inline const std::vector<DialogueLine> dragonstone = {
    { Speaker::StoryTeller, "You have received the Dragon Stone of the {dragonelement}!" },
    { Speaker::StoryTeller, "You now have {dragonstonecount} Dragon Stone(s)!" },
    { Speaker::StoryTeller, "Your Dragon Scales grow even stronger!" },
    { Speaker::StoryTeller, "Your weapon has been upgraded! It can now use the power of the {dragonelement}." },
    { Speaker::StoryTeller, "You will now be teleported back to Gonad." },
};
