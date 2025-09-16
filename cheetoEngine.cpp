#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "cheetoEngine.h"

int fov = 60.0;
int starCount = 100;
int speed = 2;

// ================== 3D ENGINE CORE ==================
struct Point3D {
  float x, y, z;
};

struct Edge {
  int a, b; // vertex indices
};

// ================== CAMERA ==================
struct Camera {
  float x, y, z;
  float rotX, rotY, rotZ;
};

Camera camera = {0, 0, -100, 0, 0, 0}; // starting behind origin, looking forward

void rotateX(Point3D &p, float a) {
  float y = p.y * cos(a) - p.z * sin(a);
  float z = p.y * sin(a) + p.z * cos(a);
  p.y = y; p.z = z;
}

void rotateY(Point3D &p, float a) {
  float x = p.x * cos(a) + p.z * sin(a);
  float z = -p.x * sin(a) + p.z * cos(a);
  p.x = x; p.z = z;
}

void rotateZ(Point3D &p, float a) {
  float x = p.x * cos(a) - p.y * sin(a);
  float y = p.x * sin(a) + p.y * cos(a);
  p.x = x; p.y = y;
}


void projectPoint(Point3D p, int &x, int &y) {
  float fovRad = fov * (PI / 180.0);  // convert to radians
  float focalLength = (128 / 2) / tan(fovRad / 2); // screen_half / tan(fov/2)

  // Avoid divide by zero / behind-camera
  if (p.z <= 1) p.z = 1;

  x = (int)((p.x * focalLength) / p.z) + 128 / 2;
  y = (int)((p.y * focalLength) / p.z) + 128 / 2;
}

// Apply camera transform: world → camera space
void applyCamera(Point3D &p) {
  // Step 1: translate world into camera space
  p.x -= camera.x;
  p.y -= camera.y;
  p.z -= camera.z;

  // Step 2: rotate world opposite the camera orientation
  // order: yaw (Y), pitch (X), roll (Z)
  rotateY(p, -camera.rotY); // yaw
  rotateX(p, -camera.rotX); // pitch
  rotateZ(p, -camera.rotZ); // roll (optional)
}

void initStars(Point3D starList[], int count) {
  for (int i = 0; i < count; i++) {
    starList[i].x = random(-200, 200);
    starList[i].y = random(-200, 200);
    starList[i].z = random(-200, 200);
  }
}

class Object3D {
public:
  Point3D* vertices;
  Edge* edges;
  int vertexCount;
  int edgeCount;

  float rotX, rotY, rotZ;   // rotation
  float posX, posY, posZ;   // translation
  float scale;              // scale

  Object3D(Point3D* v, int vCount, Edge* e, int eCount) {
    vertices = v;
    vertexCount = vCount;
    edges = e;
    edgeCount = eCount;
    rotX = rotY = rotZ = 0;
    posX = posY = posZ = 0;
    scale = 1.0;
  }

  void render(Adafruit_GFX &gfx) {
    // Apply transformations and project vertices
    Point3D transformed[vertexCount];
    for (int i = 0; i < vertexCount; i++) {
      transformed[i] = vertices[i];

      // Scale
      transformed[i].x *= scale;
      transformed[i].y *= scale;
      transformed[i].z *= scale;

      // Rotate
      rotateX(transformed[i], rotX);
      rotateY(transformed[i], rotY);
      rotateZ(transformed[i], rotZ);

      // Translate
      transformed[i].x += posX;
      transformed[i].y += posY;
      transformed[i].z += posZ;

      applyCamera(transformed[i]);
    }

    // Draw edges
    for (int i = 0; i < edgeCount; i++) {
      int x0, y0, x1, y1;
      projectPoint(transformed[edges[i].a], x0, y0);
      projectPoint(transformed[edges[i].b], x1, y1);
      gfx.drawLine(x0, y0, x1, y1, SH110X_WHITE);
    }
  }
};

Point3D getForwardVector(Camera &cam) {
  Point3D fwd;
  fwd.x = sin(cam.rotY) * cos(cam.rotX);
  fwd.y = -sin(cam.rotX);
  fwd.z = cos(cam.rotY) * cos(cam.rotX);
  return fwd;
}


void moveForward(Camera &cam, float speed) {
  Point3D fwd = getForwardVector(cam);
  cam.x += fwd.x * speed;
  cam.y += fwd.y * speed;
  cam.z += fwd.z * speed;
}

void moveBackward(Camera &cam, float speed) {
  Point3D fwd = getForwardVector(cam);
  cam.x -= fwd.x * speed;
  cam.y -= fwd.y * speed;
  cam.z -= fwd.z * speed;
}

// ================== OBJECT DEFINITIONS ==================

// Cube
Point3D cubeVertices[8] = {
  {-20, -20, -20}, {20, -20, -20}, {20, 20, -20}, {-20, 20, -20},
  {-20, -20,  20}, {20, -20,  20}, {20, 20,  20}, {-20, 20,  20}
};

Edge cubeEdges[12] = {
  {0,1},{1,2},{2,3},{3,0}, // back
  {4,5},{5,6},{6,7},{7,4}, // front
  {0,4},{1,5},{2,6},{3,7}  // connectors
};

Object3D cube(cubeVertices, 8, cubeEdges, 12);

// Pyramid with a square base
Point3D pyramidVertices[5] = {
  {-20, -20, -20},  // 0 base corner
  { 20, -20, -20},  // 1 base corner
  { 20,  20, -20},  // 2 base corner
  {-20,  20, -20},  // 3 base corner
  {  0,   0,  20}   // 4 apex
};

Edge pyramidEdges[8] = {
  {0,1},{1,2},{2,3},{3,0}, // base square
  {0,4},{1,4},{2,4},{3,4}  // sides
};

// Create the pyramid object
Object3D pyramid(pyramidVertices, 5, pyramidEdges, 8);

// person object
Point3D personVertices[10] = {
  {0, 0, 0},      // 0: head center
  {0, -10, 0},    // 1: neck / top torso
  {0, -30, 0},    // 2: bottom torso / waist
  {-10, -30, 0},  // 3: left hip
  {10, -30, 0},   // 4: right hip
  {-10, -50, 0},  // 5: left foot
  {10, -50, 0},   // 6: right foot
  {-15, -10, 0},  // 7: left hand
  {15, -10, 0},   // 8: right hand
  {0, 10, 0}      // 9: top of head
};

Edge personEdges[12] = {
  {0, 9},   // head top
  {0, 1},   // neck to head
  {1, 2},   // torso
  {2, 3},   // torso to left hip
  {2, 4},   // torso to right hip
  {3, 5},   // left leg
  {4, 6},   // right leg
  {1, 7},   // left arm
  {1, 8},   // right arm
  {7, 5},   // optional left arm to left foot (like stick figure)
  {8, 6},   // optional right arm to right foot
  {3, 4}    // hip connection
};

Object3D person(personVertices, 10, personEdges, 12);


void cheetoEngine() {
  waitForSelectRelease();
  bool stopEngine = false;

  //object settings
  person.scale = 1.0;
  person.posX = 70;
  person.posY = 0;
  person.posZ = 0;

  pyramid.posZ = 40;
  
  while (!stopEngine) {
    display.clearDisplay();

    drawAdjustable(60, 20, fov, 1, 180, "fov: ", false);

    drawAdjustable(60, 50, starCount, 100, 300, "stars: ", false);

    drawAdjustable(60, 80, speed, 1, 5, "speed: ", false);

    if (drawButton(60, 110, 6, 8, "OK")) {
      stopEngine = true;
    }

    updateButtonStates();
    updateGyro();
    updateCursor();
    drawCursor();
    updateParticles();
    drawParticles();

    display.display();
  }

  stopEngine = false;
  
  Point3D stars[starCount];
  initStars(stars, starCount);

  while (!stopEngine) {
      display.clearDisplay();

      updateButtonStates();

      if (leftButtonState) {
          angleX = 0;
          angleY = 0;
          angleZ = 0;
      }

      if (middleButtonState) {
          stopEngine = true;
      }

      if (rightButtonState) {
        moveForward(camera, speed);
      }

      updateGyro();

      camera.rotX = angleY / 40.00;
      camera.rotY = angleX / 40.00 * -1;
      //camera.rotZ = angleZ / 40.00;

      if (camera.rotX > 1.5) camera.rotX = 1.5;   // ~85 degrees up
      if (camera.rotX < -1.5) camera.rotX = -1.5; // ~85 degrees down

      for (int i = 0; i < starCount; i++) {
        Point3D p = stars[i];    // copy so we don't overwrite original
        applyCamera(p);
    
        int sx, sy;
        projectPoint(p, sx, sy);
    
        // only draw if it's in front of camera & on-screen
        if (p.z > 1 && sx >= 0 && sx < 128 && sy >= 0 && sy < 128) {
            display.drawPixel(sx, sy, SH110X_WHITE);
        }
      }    

      // cube.rotX = angleY / 40.00;
      // cube.rotY = angleX / 40.00 * -1;
      // cube.rotZ = angleZ / 40.00;

      //cube.posX = 20;
      

      cube.render(display);

      // pyramid.rotX = angleY / 40.00;
      // pyramid.rotY = angleX / 40.00 * -1;
      // pyramid.rotZ = angleZ / 40.00;

      //pyramid.posX = -20;
      //pyramid.posY = 20;

      pyramid.render(display);

      person.render(display);

      display.display();

      delay(10);
  }
  waitForSelectRelease();
}