#include <behaviourtree.h>

//BEHAVIOUR TREE STUFF (PRETTY SIGMA)

DRAM_ATTR class Selector : public Node {
  std::vector<Node*> children;
public:
  Selector(std::initializer_list<Node*> nodes)
    : children(nodes) {}
  NodeStatus tick() override {
    for (auto child : children) {
      NodeStatus status = child->tick();
      if (status != FAILURE) return status;
    }
    return FAILURE;
  }
};

DRAM_ATTR class Sequence : public Node {
  std::vector<Node*> children;
public:
  Sequence(std::initializer_list<Node*> nodes)
    : children(nodes) {}
  NodeStatus tick() override {
    for (auto child : children) {
      NodeStatus status = child->tick();
      if (status != SUCCESS) return status;
    }
    return SUCCESS;
  }
};


//LEAF NODES
class IsHungry : public Node {
public:
  NodeStatus tick() override {
    return (petHunger < 30) ? SUCCESS : FAILURE;
  }
};

class IsBored : public Node {
public:
  NodeStatus tick() override {
    return (petFun < 40) ? SUCCESS : FAILURE;
  }
};

class IsTired : public Node {
public:
  NodeStatus tick() override {
    return (petSleep < 30) ? SUCCESS : FAILURE;
  }
};

class IsBeingShaken : public Node {
public:
  NodeStatus tick() override {
    return (totalG > 2) ? SUCCESS : FAILURE;
  }
};

class AskForFood : public Node {
public:
  NodeStatus tick() override {
    if (messageDisplayTime < messageMaxTime || random(0, 50) != 1) {
      return RUNNING;
    }
    int messageRandomiser = random(0, hungryLinesCount);
    petMessage(hungryLines[messageRandomiser]);
    return SUCCESS;   
  } 
};

class AskForPlay : public Node {
public:
  NodeStatus tick() override {
    if (messageDisplayTime < messageMaxTime || random(0, 50) != 1) {
      return RUNNING;
    }
    int messageRandomiser = random(0, boredLinesCount);
    petMessage(boredLines[messageRandomiser]);
    return SUCCESS;
  } 
};

class AskForSleep : public Node {
public:
  NodeStatus tick() override {
    if (messageDisplayTime < messageMaxTime || random(0, 50) != 1) {
      return RUNNING;
    }
    int messageRandomiser = random(0, tiredLinesCount);
    petMessage(tiredLines[messageRandomiser]);
    return SUCCESS;
  } 
};

class Idle : public Node {
public:
  NodeStatus tick() override {
    //0.8% chance to yap
    if (messageDisplayTime >= messageMaxTime) {
      if (random(0, 200) == 1) {
        if (random(0, 2) == 1) {  // either say a mood related line or a random line
          if (petMood <= 20) {
            int messageRandomiser = random(0, sadLinesCount);
            petMessage(sadLines[messageRandomiser]);
            return SUCCESS;
          } else if (petMood >= 70) {
            int messageRandomiser = random(0, happyLinesCount);
            petMessage(happyLines[messageRandomiser]);
            return SUCCESS;
          }
        } else {
          int messageRandomiser = random(0, idleLinesCount);
          petMessage(idleLines[messageRandomiser]);
        }
      } else if (random(0, 200) == 2) {
        petMessage(generateSentence());
      } 
    }
    if (!movePet) {
      if (random(0, 100) == 1) {
        startMovingPet(random(0, 105), random(35, 100), 1);
      }
    }
    return SUCCESS;
  }
};

class WantToUsePiano : public Node {
public:
  NodeStatus tick() override {
    if (petStatus == 3 || (petStatus == 0 && (petSitTimer < 5) && random(0, 800) == 1)) {
      petStatus = 3;
      return SUCCESS;
    }
    return FAILURE;
  }
};

class UsePiano : public Node {
public:
  NodeStatus tick() override {
    if (checkItemIsPlaced(37)) {
      updateAreaPointers();
      int index = findIndexByType(*currentAreaPtr, 37);
      int itemX = (*currentAreaPtr)[index].x + 4;
      int itemY = (*currentAreaPtr)[index].y + 10;
      if (!movePet) {
        startMovingPet(itemX, itemY, 1);
      }
      //Serial.println("moving pet to piano");
      if (petX == itemX && petY == itemY) {
        //Serial.println("pet has reached piano");
        
        playRandomSong();

        sitPet(2, 1);

        petMessage(pianoLines[random(0, pianoLinesCount)]);
        petStatus = 0;
        return SUCCESS;
      } 
      return RUNNING;
    }
    petStatus = 0;
    return FAILURE;
  }
};

class WantToUseFireplace : public Node {
public:
  NodeStatus tick() override {
    if (petStatus == 2 || (petStatus == 0 && (petSitTimer < 5) && random(0, 800) == 1)) {
      petStatus = 2;
      return SUCCESS;
    }
    return FAILURE;
  }
};

class UseFireplace : public Node {
public:
  NodeStatus tick() override {
    if (checkItemIsPlaced(5)) {
      updateAreaPointers();
      BitmapInfo petBmp = bitmaps[pets[userPet].marshmellowID];
      int index = findIndexByType(*currentAreaPtr, 5);
      int itemX = (*currentAreaPtr)[index].x - 15;
      int itemY = 43 - petBmp.height;
      if (!movePet) {
        startMovingPet(itemX, itemY, 2);
      }
      //Serial.println("moving pet to fireplace");
      if (petX == itemX && petY == itemY) {
        //Serial.println("pet has reached fireplace");
        sitPet(200, 1);
        petHunger += random(0, 2);
        petMessage(fireplaceLines[random(0, fireplaceLinesCount)]);
        petStatus = 0;
        return SUCCESS;
      } 
      return RUNNING;
    }
    petStatus = 0;
    return FAILURE;
  }
};

class WantToSitOnCouch : public Node {
public:
  NodeStatus tick() override {
    if (petStatus == 1 || (petStatus == 0 && (petSitTimer < 5) && random(0, 800) == 1)) {
      //Serial.println("wanting to sit on couch");
      petStatus = 1;
      return SUCCESS;
    }
    //Serial.println("not wanting to sit on couch");
    return FAILURE;
  }
};

class IsCouchAvailable : public Node {
public:
  NodeStatus tick() override {
    updateAreaPointers();
    if (findIndexByType(*currentAreaPtr, 3) != -1) {
      return SUCCESS; 
    } else {
      petStatus = 0;
      return FAILURE;
    }
  }
};

class SitOnCouch : public Node {
public:
  NodeStatus tick() override {
    if (petStatus == 1) {
      updateAreaPointers();
      int index = findIndexByType(*currentAreaPtr, 3);
      if (index != -1) {
        BitmapInfo petBmp = bitmaps[pets[userPet].sitID];
        int itemX = (*currentAreaPtr)[index].x  +  (28 - petBmp.width) / 2;
        int itemY = (*currentAreaPtr)[index].y  +  (30 - petBmp.height) / 2;
        if (!movePet) {
          startMovingPet(itemX, itemY, 2);
        }
        //Serial.println("moving pet to couch");
        if (petX == itemX && petY == itemY) {
          //Serial.println("pet has reached couch");
          sitPet(200, 0);
          petStatus = 0;
          return SUCCESS;
        } else {
          return RUNNING;
        }
      }  
    }
  }
};

class IsFoodAvailable : public Node {
public:
  NodeStatus tick() override {

    if (amountFoodPlaced > 0) {
      return SUCCESS;
    } else {
      return FAILURE;
    }
  }
};

class BeingCarried : public Node {
public:
  NodeStatus tick() override {

    if (movingPet) {
      return SUCCESS;
    } else {
      return FAILURE;
    }
  }
};

class ComplainAboutBeingCarried : public Node {
public:
  NodeStatus tick() override {
    if (messageDisplayTime >= messageMaxTime) {
      int messageRandomiser = random(0, beingCarriedLinesCount);
      petMessage(beingCarriedLines[messageRandomiser]);
    }

    return SUCCESS;
  }
};

class ComplainAboutBeingShaken : public Node {
public:
  NodeStatus tick() override {

    if (messageDisplayTime >= messageMaxTime) {
      int messageRandomiser = random(0, shakenLinesCount);
      petMessage(shakenLines[messageRandomiser]);
    }

    return SUCCESS;
  }
};

class EatFood : public Node {
public:
  NodeStatus tick() override {

    int lastFoodIndex = amountFoodPlaced - 1;    
    int lastFoodX = placedFoodX[lastFoodIndex];
    int lastFoodY = placedFoodY[lastFoodIndex] - 8;

    if (lastFoodX == petX && lastFoodY == petY) {
      removeFromList(placedFood, lastFoodIndex, lastFoodIndex);
      removeFromList(placedFoodX, lastFoodIndex, lastFoodIndex);
      removeFromList(placedFoodY, lastFoodIndex, lastFoodIndex);
      amountFoodPlaced--;
      petHunger += 30;
      petPoop += 30;
      movePet = false;
      petMessage("yum!");
      return SUCCESS;
    } else {
      if (!movePet) {
        startMovingPet(lastFoodX, lastFoodY, 2);
      }
      return RUNNING;
    }
  }
};

class ShouldPoop : public Node {
public:
  NodeStatus tick() override {
    if (petPoop > 99) {
      return SUCCESS;
    } else {
      return FAILURE;
    }
  }
};

class Poop : public Node {
public:
  NodeStatus tick() override {
    updateAreaPointers();

    ItemList newItem = {
      38, petX + 5, petY + 5
    };

    for (int i = 0; i < 8; i++) {
      float angle = i * (PI / 4); // Divide circle into 8 parts
      float vx = cos(angle); // Velocity in x direction
      float vy = sin(angle); // Velocity in y direction
      createParticle(2, petX + 5, petY + 5, vx, vy, random(80, 120)); // Spawn particle
    }

    currentAreaPtr->push_back({newItem});

    petPoop = 0;
    
    return SUCCESS;
  }
};

class ShouldDie : public Node {
public:
  NodeStatus tick() override {

    if (petHunger < 1 || petSleep < 1) {
      return SUCCESS;
    } else {
      return FAILURE;
    }
  }
};

class Die : public Node {
public:
  NodeStatus tick() override {
    if (petHunger < 1) {
      killPet("starved to death");
    } else {
      killPet("collapsed due to\nexhaustion");
    }
  }
};

/* THIS IS THE SHIT. RIGHT HERE. THIS IS WHERE ITS AT. OH MY GOD LOOK AT IT
*ahem* behaviour tree setup. put all actions in order, the higher they are in the list the higher their priority.
each node will pass to the next one FAILURE, RUNNING or SUCCESS.
if its failure, it runs the next sequence below it.
if its running, it doesnt run the next node or next sequence.
if its success, it runs the next node in the current sequence.
for example, dying is a higher priority than asking for food.
*/
DRAM_ATTR Node* tree = new Selector({ new Sequence({ new ShouldDie(), new Die() }),
                                      new Sequence({ new ShouldPoop(), new Poop() }),
                                      new Sequence({ new IsBeingShaken(), new ComplainAboutBeingShaken() }),
                                      new Sequence({ new BeingCarried(), new ComplainAboutBeingCarried() }),
                                      new Sequence({ new IsFoodAvailable(), new EatFood() }),
                                      new Sequence({ new IsHungry(), new AskForFood() }),
                                      new Sequence({ new IsTired(), new AskForSleep() }),
                                      new Sequence({ new IsBored(), new AskForPlay() }),
                                      new Sequence({ new WantToUseFireplace(), new UseFireplace() }) ,
                                      new Sequence({ new WantToSitOnCouch(), new IsCouchAvailable(), new SitOnCouch() }),
                                      new Sequence({ new WantToUsePiano(), new UsePiano() }),
                                      new Idle()
                                      });