#!/bin/bash

if g++ -DPLATFORM_PC -std=c++17 -Wall -Wextra -pedantic drivers/rtc/pc/RTCHost.cpp drivers/display/Display.cpp drivers/display/pc/DisplaySDL2.cpp drivers/display/Font.cpp drivers/display/fonts/Font5x7.cpp drivers/input/pc/InputSDL2.cpp DisplayTest.cpp -o DisplayTest $(sdl2-config --cflags --libs); then
  echo build successful - running program
  ./DisplayTest
else
  echo build failed
fi
