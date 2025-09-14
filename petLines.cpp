#include "petLines.h"
#include <Arduino.h>

DRAM_ATTR String idleLines[] = {
  "whassup",
  "lmao",
  "uhhh",
  "beep beep im a sheep",
  "wheeyy",
  "heheheha",
  "what to say",
  "hello",
  "a red spy is in the base!",
  "protect the briefcase",
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
  "see if your device needs charging",
  "what is my purpose?",
  "this vexes me",
  "it is i, {PETNAME}",
  "hello! im {PETNAME}",
  "{PETNAME} is a pretty cool name amrite",
};

const size_t idleLinesCount = sizeof(idleLines) / sizeof(idleLines[0]);

String happyLines[] = {
  "i am very happy",
  "this is great",
  "i love my life",
  "so much fun",
  "yayyy",
  "wooohooo",
  "wahoo",
  "im the happiest pet",
  "life is good",
  "couldnt be better",
  "this is the best day ever",
  "im so glad to be alive",
  "i love you so much",
  "you are the best owner ever",
  "thank you for taking care of me",
  "you are my favorite human",
  "am good"
};

const size_t happyLinesCount = sizeof(happyLines) / sizeof(happyLines[0]);

String sadLines[] = {
  "i am very sad",
  "this is bad",
  "i hate my life",
  "so much pain",
  "booohoo",
  "waaaah",
  "im the saddest pet",
  "life is bad",
  "couldnt be worse",
  "this is the worst day ever",
  "im so sad to be alive",
  "i hate you so much",
  "you are the worst owner ever",
  "why do you take care of me",
  "you are my least favorite human",
  "am bad"
};

const size_t sadLinesCount = sizeof(sadLines) / sizeof(sadLines[0]);

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
  "FOOD. IN. MY. MOUTH.",
  "give me the damn food"
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
  "pooooongggg",
  "shooty?",
  "flappy bur?",
  "bubblebox?",
  "im bored to death",
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

DRAM_ATTR String fireplaceLines[] = {
  "toasty",
  "i love marshmellow",
  "consuming",
  "the chair cannot cook marshmellows",
  "popcorn",
  "would you like a marshmellow?"
};

const size_t fireplaceLinesCount = sizeof(fireplaceLines) / sizeof(fireplaceLines[0]);

DRAM_ATTR String pianoLines[] = {
  "la de da de da",
  "i am the next beethoven",
  "bing bong bing",
  "welcome to my concert",
  "practicing arpegios or whatever",
  "can we get some drums?",
  "practicing scales or whatever"
};

const size_t pianoLinesCount = sizeof(pianoLines) / sizeof(pianoLines[0]);

const char* nouns[] = {
  "cat", "dog", "robot", "car", "tree", "bird", "house", "computer",
  "book", "river", "mountain", "child", "city", "flower", "ocean", "star",
  "music", "man", "woman", "goose"
};

const char* adjectives[] = {
  "fast", "red", "lazy", "funny", "bright", "silent", "tall", "ancient",
  "wild", "happy", "bitter", "cold", "gentle", "sharp", "brave", "calm", "vexed"
};

const char* verbs[] = {
  "run", "jump", "drive", "fly", "sing", "laugh", "shine", "whisper",
  "dance", "climb", "swim", "dream", "wander", "hide", "glow", "build",
  "fart", "vex"
};


const char* adverbs[] = {
  "quickly", "silently", "gracefully", "happily", "sadly", "loudly", "bravely",
  "carefully", "eagerly", "boldly", "slowly", "fiercely", "softly", "wildly",
  "brightly", "gently", "musically", "vexedly"
};

const int nounCount = sizeof(nouns) / sizeof(nouns[0]);
const int adjCount = sizeof(adjectives) / sizeof(adjectives[0]);
const int verbCount = sizeof(verbs) / sizeof(verbs[0]);
const int adverbCount = sizeof(adverbs) / sizeof(adverbs[0]);

const char* templates[] = {
  "the <adj> <noun> <verb>s <adv>.",
  "a <noun> that <adv> <verb>s is very <adj>.",
  "the <noun> <adv> <verb>s.",
  "you know theres a <adj> <noun> over there",
  "idk, have you tried the <adj> <noun> that <verb>s <adv>?",
  "<adj> and <adj>, the <noun> <verb>s <adv>",
  "why does the <noun> <verb> so <adv>?",
  "sometimes, the <adj> <noun>s <verb> in the night",
  "did you hear about the <noun> that <verb>s through the <noun>?",
  "<adj> <noun> <noun>",
  "the <adj> <noun> need not <verb> <adv> to become <adj>",
  "yoo did you see the <noun>?",
  "you sound so <adj> right now",
  "im <verb>ing right now"
};

const int templateCount = sizeof(templates) / sizeof(templates[0]);

const char* prefixes[] = {"Al", "Be", "Car", "Da", "El", "Fi", "Go", "Ha", "In", "Jo", "Ka", "Li", "Mo", "Ne", "Or", "Pa", "Qu", "Re", "Si", "Ta", "Ul", "Vi", "Wi", "Xe", "Ya", "Zo"};
const char* suffixes[] = {"ton", "ria", "vin", "nor", "das", "lith", "mus", "nus", "phy", "rus", "son", "tis", "ver", "wyn", "xen", "yus", "zen"};

const int prefixCount = sizeof(prefixes) / sizeof(prefixes[0]);
const int suffixCount = sizeof(suffixes) / sizeof(suffixes[0]);


bool findNameIndices(const String &name, int &prefixIndex, int &suffixIndex) {
  for (int i = 0; i < prefixCount; i++) {
    const char* prefix = prefixes[i];
    int prefixLen = strlen(prefix);

    // Does the name start with this prefix?
    if (name.startsWith(prefix)) {
      for (int j = 0; j < suffixCount; j++) {
        const char* suffix = suffixes[j];
        int suffixLen = strlen(suffix);

        // Does the name end with this suffix?
        if (name.endsWith(suffix)) {
          prefixIndex = i;
          suffixIndex = j;
          return true; // found both
        }
      }
    }
  }
  return false; // no match found
}

String decodeName(int prefixIndex, int suffixIndex) {
  if (prefixIndex < 0 || prefixIndex >= prefixCount || suffixIndex < 0 || suffixIndex >= suffixCount) {
    return ""; // invalid indices
  }
  return String(prefixes[prefixIndex]) + String(suffixes[suffixIndex]);
}

String randomName() {
  return String(prefixes[random(prefixCount)]) + String(suffixes[random(suffixCount)]);
}

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
  return adverbs[random(adverbCount)];
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
