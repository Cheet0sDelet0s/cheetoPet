#include <Arduino.h>
#include <RTClib.h>
#include "newsgenerator.h"

// Arrays of data
const char* newsTemplates[] = {
    "On {DATE}, {CITY} reported {EVENT}.",
    "{EVENT} shocked {CITY} on {DATE}.",
    "Residents of {CITY} were met with {EVENT} this {DATE}.",
    "People in {CITY} can't stop\ntalking about {EVENT}\nthat happened on {DATE}.",
    "Breaking News from {CITY}:\n{EVENT} occurred on {DATE}.",
    "{CITY} experiences {EVENT} on {DATE}.",
    "In {CITY}, {EVENT} took place on {DATE}.",
    "{EVENT} was the talk of {CITY} on {DATE}.",
    "On {DATE},\n{CITY} was the scene of {EVENT}.",
    "{CITY} woke up to {EVENT} on {DATE}.",
    "{EVENT} disrupted life in {CITY} on {DATE}.",
    "Citizens of {CITY} witnessed {EVENT} on {DATE}.",
    "{EVENT} made headlines in {CITY} on {DATE}.",
    "The city of {CITY} saw {EVENT} on {DATE}.",
    "{EVENT} took {CITY} by surprise on {DATE}.",
    "{CITY} was abuzz with news of {EVENT} on {DATE}.",
    "On {DATE}, {CITY} was rocked by {EVENT}.",
    "{EVENT} captured the attention of {CITY} on {DATE}.",
    "{CITY} residents recall {EVENT} from {DATE}.",
    "{EVENT} was reported in {CITY} on {DATE}.",
    "{CITY} was the location of {EVENT} on {DATE}.",
    "{EVENT} unfolded in {CITY} on {DATE}.",
    "{CITY} was the center of {EVENT} on {DATE}.",
    "{EVENT} occurred in {CITY} on {DATE}.",
    "{CITY} was the site of {EVENT} on {DATE}."
  };
  
const char* cities[] = {"New York", "London", "Tokyo", "Sydney", "Bristol", "Hobbiton", "Metropolis", "Atlantis", "El Dorado",
    "Springfield", "Smallville", "Rivendell", "Narnia", "Hogsmeade", "Wakanda", "Zootopia", "Asgard",
    "Camelot", "Emerald City", "King's Landing", "Winterfell", "Mordor", "Pandora", "Tatooine",
    "Bikini Bottom", "Duckburg", "Arendelle", "Monstropolis", "Bedrock", "Sunnydale", "Vice City",
    "Hill Valley", "Silent Hill", "Raccoon City", "Gotham City", "Quahog", "South Park", "Langley Falls",
    "Guantanomo Bay", "Chernobyl", "Area 51", "Roswell", "Shelbyville", "Ogdenville", "The Shire"
};

const char* events[] = {
    "a mysterious blackout",
    "an unexpected festival",
    "record-breaking heat",
    "a surprise parade",
    "a flock of birds blocking traffic",
    "a mad goose causing chaos",
    "a sudden downpour",
    "a flash mob dance",
    "a celebrity sighting",
    "a street performance",
    "a food truck rally",
    "a local sports victory",
    "a community art project",
    "a charity run",
    "a new cafe opening",
    "a power outage",
    "a traffic jam",
    "a lost dog found",
    "a surprise wedding",
    "a flash flood",
    "a UFO sighting",
    "a viral dance challenge",
    "a spontaneous protest",
    "a pop-up market",
    "a city-wide scavenger hunt",
    "a giant inflatable duck in the harbor",
    "a robot delivering food",
    "a drone light show",
    "a giant rubber band ball appearing overnight",
    "a spontaneous street party",
    "a parade of vintage cars",
    "a sudden snowstorm",
    "a massive bubble blowing contest",
    "a giant chess game in the park",
    "a surprise concert",
    "a flash sale at the local mall",
    "a spontaneous yoga session in the square",
    "a giant kite festival",
    "a city-wide game of hide and seek",
    "a massive pillow fight",
    "a spontaneous dance-off",
    "a giant game of hopscotch",
    "a surprise fireworks display",
    "a spontaneous poetry reading",
    "a giant game of tag",
    "a surprise ice cream truck",
    "a spontaneous drum circle"
};

int dayOfYear(DateTime dt) {
    // Days in months (non-leap year)
    const int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int doy = 0;

    // Add up days of previous months
    for (int m = 1; m < dt.month(); m++) {
        doy += daysInMonth[m-1];
    }

    // Leap year adjustment
    bool leap = ((dt.year() % 4 == 0 && dt.year() % 100 != 0) || (dt.year() % 400 == 0));
    if (leap && dt.month() > 2) {
        doy += 1;
    }

    // Add current day
    doy += dt.day();

    return doy;
}  

int seededRandom(int max, int seed) {
    srand(seed);
    return rand() % max;
}

String generateNewsHeadline(int seedModifier) {
    DateTime now = rtc.now();
    int seed = now.year() * 365 + dayOfYear(now);

    // Pick template, city, and event
    int tIndex = seededRandom(sizeof(newsTemplates) / sizeof(newsTemplates[0]), seed + seedModifier);
    int cIndex = seededRandom(sizeof(cities) / sizeof(cities[0]), seed + 1 + seedModifier);
    int eIndex = seededRandom(sizeof(events) / sizeof(events[0]), seed + 2 + seedModifier);

    // Format date
    char dateChar[32];
    snprintf(dateChar, sizeof(dateChar), "%04d-%02d-%02d", now.year(), now.month(), now.day());

    String dateStr = String(dateChar);

    // Replace tokens
    String story = newsTemplates[tIndex];

    story.replace("{DATE}", dateStr);
    story.replace("{CITY}", cities[cIndex]);
    story.replace("{EVENT}", events[eIndex]);

    return String(story);
}