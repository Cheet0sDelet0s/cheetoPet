#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "cheetoEngine.h"

// Cube vertices
Point3D cubeVertices[8] = {
    {-30, -30, -30}, {30, -30, -30}, {30, 30, -30}, {-30, 30, -30},
    {-30, -30, 30},  {30, -30, 30},  {30, 30, 30},  {-30, 30, 30}
  };
  
  // Cube edges (pairs of vertex indices)
  int cubeEdges[12][2] = {
    {0,1},{1,2},{2,3},{3,0}, // back face
    {4,5},{5,6},{6,7},{7,4}, // front face
    {0,4},{1,5},{2,6},{3,7}  // connecting edges
  };
  
  // Rotation angles are based on gyro
  
  // Projection function
  void projectPoint(Point3D p, int &x, int &y) {
    float fov = 60.0;
    float distance = 100.0;
    x = (int)((p.x * fov) / (p.z + distance)) + 128 / 2;
    y = (int)((p.y * fov) / (p.z + distance)) + 128 / 2;
  }
  
  // Rotation functions
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


void cheetoEngine() {
    bool stopEngine = false;
    waitForSelectRelease();
    while (!stopEngine) {
        display.clearDisplay();

        updateButtonStates();
        if (leftButtonState) {
            angleX, angleY, angleZ = 0;
        }
        if (rightButtonState) {
            stopEngine = true;
        }

        updateGyro();

        // Copy vertices for rotation
        Point3D transformed[8];
        for (int i = 0; i < 8; i++) {
            transformed[i] = cubeVertices[i];
            rotateX(transformed[i], angleX / 20.00);
            rotateY(transformed[i], angleY / 20.00);
            rotateZ(transformed[i], angleZ / 20.00);
        }

        // Draw edges
        for (int i = 0; i < 12; i++) {
            int x0, y0, x1, y1;
            projectPoint(transformed[cubeEdges[i][0]], x0, y0);
            projectPoint(transformed[cubeEdges[i][1]], x1, y1);
            display.drawLine(x0, y0, x1, y1, SH110X_WHITE);
        }

        display.display();

        delay(30);
    }
    waitForSelectRelease();
}