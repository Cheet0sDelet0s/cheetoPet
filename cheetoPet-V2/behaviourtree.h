#ifndef BEHAVIOURTREE_H
#define BEHAVIOURTREE_H

#include <Arduino.h>

#include "pet.h"
#include "hardware.h"
#include "bitmaps.h"
#include <petLines.h>

DRAM_ATTR enum NodeStatus { SUCCESS,
                            FAILURE,
                            RUNNING };

DRAM_ATTR class Node {
public:
  virtual NodeStatus tick() = 0;
};

Node* tree;

#endif