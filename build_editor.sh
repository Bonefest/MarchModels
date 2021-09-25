#!/bin/bash

set echo on

mkdir -p bin/

cFilenames=$(find . -type f -name "*.c" -o -name "*.cpp")

assembly="editor"
compilerFlags="-g -O0 -fPIC"
includeFlags="-Isrc -Ithrdparty -Igame"
linkerFlags="-lglfw -lpthread -lXxf86vm -ldl -lxcb -lX11 -lX11-xcb -lxkbcommon-x11 -L/usr/lib/X11"
defines="-DDEBUG -DENABLE_TESTBED_GAME"

clear

echo "-------------------------------------------------------------------------------"
echo "building $assembly"
echo "-------------------------------------------------------------------------------"

echo g++ $cFilenames $compilerFlags -o .bin/$assembly $defines $includeFlags $linkerFlags
g++ $cFilenames $compilerFlags -o bin/$assembly $defines $includeFlags $linkerFlags
