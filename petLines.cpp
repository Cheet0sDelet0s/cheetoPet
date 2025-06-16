#include "petLines.h"
#include <Arduino.h>

DRAM_ATTR String idleLines[] = {
  "whassup",
  "am good",
  "lmao",
  "uhhh",
  "beep beep im a sheep",
  "wheeyy",
  "heheheha",
  "what to say",
  "hello",
  "a red spy is in the base!",
  "protect the briefcase",
  "wahoo",
  "are you in class",
  "sudo rm -rf /",
  "gold gold gold",
  "gonna play GooseStrike 2",
  "fatty fatty",
  "x formerly twitter",
  "did you get those reports?",
  "i needed those reports by friday",
  "git pull origin main",
  "git commit -m 'wheyy'",
  "git push --force",
  "did you hear about that guy?",
  "heard he got out of jail",
  "heard he got hit by a truck",
  "heard he got fired",
  "heard he got electrocuted",
  "see if your device needs charging"
};

const size_t idleLinesCount = sizeof(idleLines) / sizeof(idleLines[0]);

DRAM_ATTR String beingCarriedLines[] = {
  "put me down!",
  "bro stop",
  "i hate this",
  "stop it",
  "omg why"
};

const size_t beingCarriedLinesCount = sizeof(beingCarriedLines) / sizeof(beingCarriedLines[0]);

DRAM_ATTR String hungryLines[] = {
  "stomach is a lil empty",
  "anything in the fridge?",
  "*rumble*",
  "food...",
  "hubgry :((",
  "feed?",
  "BRO FEED ME",
  "IM ACTUALLY GONNA DIE",
  "SERIOUSLY I WILL DIE",
  "GIVE ME FOOD.",
  "you suck man give me food",
  "FOOD. IN. MY. MOUTH."
};

const size_t hungryLinesCount = sizeof(hungryLines) / sizeof(hungryLines[0]);

DRAM_ATTR String boredLines[] = {
  "so bored",
  "play with me!",
  "give me attention",
  "please pong",
  "nothing is fun",
  "fun time?",
  "can we train for pong tournament",
  "pooooongggg"
};

const size_t boredLinesCount = sizeof(boredLines) / sizeof(boredLines[0]);

DRAM_ATTR String tiredLines[] = {
  "so tired",
  "sleep...",
  "bed pls",
  "what time is it? bedtime",
  "PLEASE SLEEP",
  "*yawn*",
  "please i havent slept in 5 days",
  "i can die from exhaustion! just saying...",
  "i will develop insomnia"
};

const size_t tiredLinesCount = sizeof(tiredLines) / sizeof(tiredLines[0]);

DRAM_ATTR String shakenLines[] = {
  "bro stop shaking me",
  "stop shaking i will die",
  "6 more g's and im dead",
  "is there a volcano wtf",
  "are you in a car?",
  "ouch!",
  "my stuff is breaking!",
  "i can die from shaking!",
  "i will get bruised"
};

const size_t shakenLinesCount = sizeof(shakenLines) / sizeof(shakenLines[0]);

const char* nouns[] = {
  "cat", "dog", "robot", "car", "tree", "bird", "house", "computer",
  "book", "river", "mountain", "child", "city", "flower", "ocean", "star"
};

const char* adjectives[] = {
  "fast", "red", "lazy", "funny", "bright", "silent", "tall", "ancient",
  "wild", "happy", "bitter", "cold", "gentle", "sharp", "brave", "calm"
};

const char* verbs[] = {
  "runs", "jumps", "drives", "flies", "sings", "laughs", "shines", "whispers",
  "dances", "climbs", "swims", "dreams", "wanders", "hides", "glows", "builds"
};


const char* adverbs[] = {
  "quickly", "silently", "gracefully", "happily", "sadly", "loudly", "bravely",
  "carefully", "eagerly", "boldly", "slowly", "fiercely", "softly", "wildly",
  "brightly", "gently"
};

const int nounCount = sizeof(nouns) / sizeof(nouns[0]);
const int adjCount = sizeof(adjectives) / sizeof(adjectives[0]);
const int verbCount = sizeof(verbs) / sizeof(verbs[0]);
const int adverbCount = sizeof(adverbs) / sizeof(adverbs[0]);

const char* templates[] = {
  "the <adj> <noun> <verb> <adv>.",
  "a <noun> that <adv> <verb> is very <adj>.",
  "the <noun> <adv> <verb>.",
  "you know theres a <adj> <noun> over there",
  "idk, have you tried the <adj> <noun> that <verb> <adv>?",
  "<adj> and <adj>, the <noun> <verb> <adj>",
  "why does the <noun> <verb> so <adv>?",
  "sometimes, the <adj> <noun> <verb> in the night",
  "did you hear about the guy that <verb> through the <noun>?"
};

const int templateCount = sizeof(templates) / sizeof(templates[0]);

const char* randomNoun() {
  return nouns[random(nounCount)];
}

const char* randomAdj() {
  return adjectives[random(adjCount)];
}

const char* randomVerb() {
  return verbs[random(verbCount)];
}

const char* randomAdverb() {
  return verbs[random(adverbCount)];
}

String generateSentence() {
  String sentence = templates[random(templateCount)];

  // Replace tokens
  sentence.replace("<noun>", randomNoun());
  sentence.replace("<adj>", randomAdj());
  sentence.replace("<verb>", randomVerb());
  sentence.replace("<adv>", randomAdverb());

  return sentence;
}
