CC = g++
INCLUDE_DIRS = -Isrc -Ithrdparty -Igame -I/usr/include/lua5.3
DEFINES = -DDEBUG -DENABLE_EDITOR_GAME

CFLAGS = -g -O0 -fPIC -std=c++1z ${DEFINES} $(INCLUDE_DIRS)# -MMD -include $(OBJ_FILES:.o=.d)
LDFLAGS =-lXxf86vm -lxkbcommon-x11 -L/usr/lib/X11
LDLIBS =-lglfw -lpthread -ldl -xcb -lX11 -lX11-xcb -llua5.3

CPP_SRC_FILES = $(shell find . -type f -name "*.cpp")
C_SRC_FILES = $(shell find . -type f -name "*.c")
OBJ_FILES = $(patsubst %.cpp,%.o,${CPP_SRC_FILES}) $(patsubst %.c,%.o,${C_SRC_FILES})



$(info $(INCLUDE_DIRS))

editor: $(OBJ_FILES)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


#.PHONE: clean

#clean:
#	find . -name "*.png" | xargs rm
