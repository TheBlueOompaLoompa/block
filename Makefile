CPPFLAGS=$(shell sdl2-config --cflags) $(EXTRA_CPPFLAGS) -Iinclude
LDLIBS=$(shell sdl2-config --libs) -lGLEW $(EXTRA_LDLIBS)
EXTRA_LDLIBS?=-lGL

all: block

block src/main.cpp:
	g++ -o block src/*.cpp $(CPPFLAGS) $(LDLIBS)

artifact: all
	zip -r block.zip block shaders

clean:
	rm -f *.o block
.PHONY: all clean