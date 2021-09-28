#!/bin/bash

set echo on

mkdir -p bin/

cFilenames=$(find . -type f -name "*.c" -o -name "*.cpp")

assembly="editor"
compilerFlags="-g -std=c++1z -O0 -fPIC"

# TODO: Add lua5.3 to the thrdparty folder
includeFlags="-Isrc -Ithrdparty -Igame -I/usr/include/lua5.3"
linkerFlags="-lglfw -lpthread -lXxf86vm -ldl -lxcb -lX11 -lX11-xcb -lxkbcommon-x11 -L/usr/lib/X11 -llua5.3"
defines="-DDEBUG -DENABLE_TESTBED_GAME"

clear

echo "-------------------------------------------------------------------------------"
echo "building $assembly"
echo "-------------------------------------------------------------------------------"

echo g++ $cFilenames $compilerFlags -o .bin/$assembly $defines $includeFlags $linkerFlags
g++ $cFilenames $compilerFlags -o bin/$assembly $defines $includeFlags $linkerFlags
