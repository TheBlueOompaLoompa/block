CPPFLAGS=$(shell sdl2-config --cflags) $(EXTRA_CPPFLAGS) -Iinclude
LDLIBS=$(shell sdl2-config --libs) -lGLEW $(EXTRA_LDLIBS)
EXTRA_LDLIBS?=-lGL
block src/main.cpp:
	g++ -o block src/*.cpp $(CPPFLAGS) $(LDLIBS)
clean:
	rm -f *.o block
.PHONY: all clean