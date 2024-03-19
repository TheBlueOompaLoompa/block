/**
 * Store all the file's contents in memory, useful to pass shaders
 * source code to OpenGL.  Using SDL_RWops for Android asset support.
 */
#include <GL/glew.h>
#pragma once

class Shader {

public:
	GLuint program_id;
	char* path;

	// Fragment shader and vertex shader must have the same name, but end with .f.glsl and .v.glsl respectively
	Shader(const char* name);
	~Shader();

	bool load();

	bool success = true;
};