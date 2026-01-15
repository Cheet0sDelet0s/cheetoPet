#include "pet.h"

int saveFileVersion = 3;

const char* prefixes[] = {"Al", "Be", "Car", "Da", "El", "Fi", "Go", "Ha", "In", "Jo", "Ka", "Li", "Mo", "Ne", "Or", "Pa", "Qu", "Re", "Si", "Ta", "Ul", "Vi", "Wi", "Xe", "Ya", "Zo"};
const char* suffixes[] = {"ton", "ria", "vin", "nor", "das", "lith", "mus", "nus", "phy", "rus", "son", "tis", "ver", "wyn", "xen", "yus", "zen"};

const int prefixCount = sizeof(prefixes) / sizeof(prefixes[0]);
const int suffixCount = sizeof(suffixes) / sizeof(suffixes[0]);

const Pet pets[] = {
  {"goose", 1, 21, 23, 26, 28},
  {"hedgehog", 43, 45, 47, 51, 49},
  {"bird", 53, 55, 57, 59, 61}
};

int petTypes = sizeof(pets) / sizeof(pets[0]);

// AREA VARIABLES
std::vector<ItemList> homePlot = {
  {7, 98, 4}
};

std::vector<ItemList> outsidePlot = {
    {34, 40, 50},
    {35, 60, 70},
    {33, 96, 5},
    {36, 46, 6},
    {42, 29, 25}
};

//item inventory
DRAM_ATTR int inventory[8] = {};
DRAM_ATTR int inventoryItems = 0;

DRAM_ATTR int itemBeingPlaced = -1;
DRAM_ATTR bool startHandlingPlacing = false;

//food inventory
DRAM_ATTR int foodInventory[8] = { 18 };
DRAM_ATTR int foodInventoryItems = 1;
DRAM_ATTR int placedFood[10] = {};
DRAM_ATTR int amountFoodPlaced = 0;
DRAM_ATTR int placedFoodX[10] = {};
DRAM_ATTR int placedFoodY[10] = {};
DRAM_ATTR bool handleFoodPlacing = false;

DRAM_ATTR std::vector<ItemList>* currentAreaPtr = nullptr;

//game library
DRAM_ATTR int gameLibrary[8] = { 0 };
DRAM_ATTR int gameLibraryCount = 1;
//                             0        1           2             3           4
const String gameNames[5] = {"pong", "shooty", "flappy bur", "bubblebox", "3d test"};

void petMessage(String message) {
  message.replace("{PETNAME}", petName);
  currentPetMessage = message;
  messageDisplayTime = 0;
  int msgLength = currentPetMessage.length();
  messageMaxTime = constrain((msgLength * 15) / 2, 1, 3000);

  for (int i = 0; i < msgLength; i++) {
    int letterNumber = currentPetMessage[i] - 'a';
    int frequency = 400 + letterNumber * 4;
    queueTone(frequency, 50);
  }

  // Spawn particles around the pet's mouth
  float baseAngle = petDir == 1 ? 0 : PI; // Base angle depends on pet direction
  float angleStep = 15 * DEG_TO_RAD; // 15 degrees in radians
  float speed = 1.5; // Particle speed

  for (int i = -1; i <= 1; i++) { // Generate 3 particles, spaced 15 degrees apart
    float angle = baseAngle + i * angleStep; // Adjust angle for each particle
    float vx = cos(angle) * speed; // Velocity in x direction
    float vy = sin(angle) * speed; // Velocity in y direction
    createParticle(3, petX + 8 * petDir, petY + 4, vx, vy, random(16, 24)); // Spawn particle near pet's mouth
  }
}

void startMovingPet(int x, int y, int speed) {
  if (petSitTimer < 1) {
    petMoveX = x;
    petMoveY = y;
    petMoveSpeed = speed;
    petMoveAnim = 0;
    movePet = true;
  }
}

void updatePetMovement() {
  int xDiff = petMoveX - petX;
  xDiff = xDiff / abs(xDiff);

  petDir = xDiff;

  int yDiff = petMoveY - petY;
  yDiff = yDiff / abs(yDiff);
  
  petX += xDiff * petMoveSpeed;
  petY += yDiff * petMoveSpeed;

  if (xDiff == 1) {
    if (petX > petMoveX) {
      petX = petMoveX;
    }
  } else {
    if (petX < petMoveX) {
      petX = petMoveX;
    }
  }

  if (yDiff == 1) {
    if (petY > petMoveY) {
      petY = petMoveY;
    }
  } else {
    if (petY < petMoveY) {
      petY = petMoveY;
    }
  }

  // if (random(1, 10) == 1) {
  //   createParticle(1, petX, petY, xDiff * -1, yDiff * -1, 20);
  // }
  
  if (petX == petMoveX && petY == petMoveY) {
    movePet = false;
  }
  petMoveAnim++;
  if (petMoveAnim > 4) {
    petMoveAnim = 0;
  }
}

void killPet(String deathReason = "") {
    spiralFill(SH110X_WHITE);
    delay(500);
    display.clearDisplay();
    display.display();
    delay(1000);
    const BitmapInfo& bmp = bitmaps[25];
    display.drawBitmap(49, 94, bmp.data, bmp.width, bmp.height, SH110X_WHITE);
    display.display();
    delay(1000);
    display.setCursor(0,0);
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(2);
    display.println("your pet\ndied");
    display.display();
    delay(500);
    display.setTextSize(1);
    display.println("after it ");
    display.display();
    delay(500);
    display.println(deathReason);
    display.display();
    delay(500);
    display.println("we are sorry\nfor your loss.");
    display.display();
    delay(500);
    display.println("press A to\nrestart.");
    display.display();
    updateButtonStates();
    while(!rightButtonState) {updateButtonStates();}
    esp_restart();
}

void updatePet() {
  unsigned long currentMillis = millis(); // this is for non-blocking delta time, you can't use delay() most of the time as it blocks the whole program

  if (currentMillis - previousMillis >= interval) { // if the interval has happened, move the pet
    previousMillis = currentMillis;     //interval passed
    
    if (movePet) {
      updatePetMovement(); //moves pet towards its desired position
    }
  }

  tree->tick();  //behaviour tree update



}