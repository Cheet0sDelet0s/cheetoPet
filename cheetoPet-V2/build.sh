#!/bin/bash

if g++ -DPLATFORM_PC -std=c++20 -Wall -Wextra -pedantic $(find . -type f -name "*.cpp") -o cheetoPet-V2 $(sdl2-config --cflags --libs); then
  echo build successful - running program
  ./cheetoPet-V2
else
  echo build failed
fi
