// bitmaps for cheetoPet

#ifndef BITMAPS_H
#define BITMAPS_H

#include <Arduino.h>

extern const unsigned char pet_gooseStill[] PROGMEM;
 
extern const unsigned char pet_gooseStillBig[] PROGMEM;

extern const unsigned char pet_gooseStillBigMask[] PROGMEM;

extern const unsigned char ui_couch1[] PROGMEM;

extern const unsigned char ui_table[] PROGMEM;

extern const unsigned char ui_fireplace[] PROGMEM;

extern const unsigned char ui_bigTable[] PROGMEM;

extern const unsigned char ui_window[] PROGMEM;
 
extern const unsigned char ui_menu[] PROGMEM;

extern const unsigned char ui_pencil[] PROGMEM;

extern const unsigned char ui_settings[] PROGMEM;

extern const unsigned char ui_shop[] PROGMEM;

extern const unsigned char ui_back[] PROGMEM;

extern const unsigned char ui_inventory[] PROGMEM;

extern const unsigned char ui_cursor[] PROGMEM;

extern const unsigned char ui_cursorMask[] PROGMEM;

extern const unsigned char item_apple[] PROGMEM;

extern const unsigned char pet_gooseSleep[] PROGMEM;

extern const unsigned char item_banana[] PROGMEM;

extern const unsigned char item_rug[] PROGMEM;

extern const unsigned char item_controller[] PROGMEM;

extern const unsigned char pet_gooseWalk[] PROGMEM;

extern const unsigned char pet_gooseWalkMask[] PROGMEM;

extern const unsigned char pet_gooseWalk2[] PROGMEM;

extern const unsigned char pet_gooseWalk2Mask[] PROGMEM;

extern const unsigned char item_gravestone[] PROGMEM;

extern const unsigned char pet_gooseSit[] PROGMEM;

extern const unsigned char pet_gooseSitMask[] PROGMEM;

extern const unsigned char pet_gooseMarshmellow[] PROGMEM;

extern const unsigned char pet_gooseMarshmellowMask[] PROGMEM;

extern const unsigned char item_tree [] PROGMEM;

extern const unsigned char item_bush [] PROGMEM;

extern const unsigned char item_fence [] PROGMEM;

extern const unsigned char item_sun [] PROGMEM;

extern const unsigned char item_grass [] PROGMEM;

extern const unsigned char item_grass2 [] PROGMEM;

extern const unsigned char item_house [] PROGMEM;

extern const unsigned char item_piano [] PROGMEM;

extern const unsigned char item_poop [] PROGMEM;

extern const unsigned char misc_vex [] PROGMEM;

extern const unsigned char misc_doctor [] PROGMEM;

extern const unsigned char item_trashcan [] PROGMEM;

extern const unsigned char item_mailbox [] PROGMEM;

extern const unsigned char pet_hedgehog [] PROGMEM;

extern const unsigned char pet_hedgehogMask [] PROGMEM;

extern const unsigned char pet_hedgehogWalk1 [] PROGMEM;

extern const unsigned char pet_hedgehogWalk1Mask [] PROGMEM;

extern const unsigned char pet_hedgehogWalk2 [] PROGMEM;

extern const unsigned char pet_hedgehogWalk2Mask [] PROGMEM;

extern const unsigned char pet_hedgehogMarshmellow [] PROGMEM;

extern const unsigned char pet_hedgehogMarshmellowMask [] PROGMEM;

extern const unsigned char pet_hedgehogSit [] PROGMEM;

extern const unsigned char pet_hedgehogSitMask [] PROGMEM;

extern const unsigned char pet_birdStill [] PROGMEM;

extern const unsigned char pet_birdStillMask [] PROGMEM;

extern const unsigned char pet_birdWalk1 [] PROGMEM;

extern const unsigned char pet_birdWalk1Mask [] PROGMEM;

extern const unsigned char pet_birdWalk2 [] PROGMEM;

extern const unsigned char pet_birdWalk2Mask [] PROGMEM;

extern const unsigned char pet_birdSit [] PROGMEM;

extern const unsigned char pet_birdSitMask [] PROGMEM;

extern const unsigned char pet_birdMarshmellow [] PROGMEM;

extern const unsigned char pet_birdMarshmellowMask [] PROGMEM;

struct BitmapInfo {
  const uint8_t* data;
  uint16_t width;
  uint16_t height;
};

extern const BitmapInfo bitmaps[];

extern String displayNames[];

extern int bitmapCount;

#endif