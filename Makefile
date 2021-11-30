# For details see:
#
# 1. https://stackoverflow.com/questions/2908057/can-i-compile-all-cpp-files-in-src-to-os-in-obj-then-link-to-binary-in
# 2. https://stackoverflow.com/questions/1950926/create-directories-using-make-file
# 3. https://web.stanford.edu/class/archive/cs/cs107/cs107.1174/guide_make.html

# Defines a compiler to use
CC = g++

# Defines libraries to include
INCLUDE_DIRS = -Isrc -Ithrdparty -Igame -I/usr/include/lua5.3 -Itests

# Defines C-macros defines
DEFINES = -DDEBUG -DENABLE_EDITOR_GAME #-DTEST_COMPILE_PATH

# Compiler flags
CFLAGS = -g -O0 -fPIC -std=c++1z ${DEFINES} $(INCLUDE_DIRS)

# Linker flags
LDFLAGS =-lXxf86vm -lxkbcommon-x11 -L/usr/lib/X11

# Link librarires
LDLIBS =-lglfw -ldl -xcb -lX11 -lX11-xcb -llua5.3 -lgtest -lgmock -lpthread 

# Similarly to the old ./build_editor.sh script - find all .c/.cpp files and save them into corresponding variables
CPP_SRC_FILES = $(shell find . -type f -name "*.cpp")
C_SRC_FILES = $(shell find . -type f -name "*.c")

# Pat(tern)subst(itute): find a file ending with .cpp, replace .cpp by .o and preappend the rest
# (e.g src/main.cpp --> % is equal to src/main, replace .cpp by .o, result is src/main.o)
OBJ_FILES = $(patsubst %.cpp,%.o,${CPP_SRC_FILES}) $(patsubst %.c,%.o,${C_SRC_FILES})

# Replace ./ part of the object files by ./obj/ so that we output all object files into /obj/ folder
OBJ_FILES := $(patsubst ./%,./obj/%,${OBJ_FILES})

# For debugging purpose only: $(info info $(OBJ_FILES))

# Main target, it depends on all obj files, meaning that to make an editor, it need to have objs up-to-date
bin/editor: $(OBJ_FILES)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# (Called 'pattern rule') Declare that obj file at location ./obj/%.o is depending on corresponding ./%.cpp file
./obj/%.o: ./%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Same for .c files
./obj/%.o: ./%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# PHONY means not a real target, it doesn't build anything
.PHONY: clean

clean:
	$(shell rm -rf obj)
    # old way of removing obj files: find . -name "*.o" | xargs rm
