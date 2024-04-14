/**
 * Store all the file's contents in memory, useful to pass shaders
 * source code to OpenGL.  Using SDL_RWops for Android asset support.
 */
#include <GL/glew.h>
#include "util.hpp"
#pragma once

void print_log(GLuint object) {
	GLint log_length = 0;
	if (glIsShader(object)) {
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else if (glIsProgram(object)) {
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else {
		std::cerr << "printlog: Not a shader or a program" << std::endl;
		return;
	}

	char* log = (char*)malloc(log_length);
	
	if (glIsShader(object))
		glGetShaderInfoLog(object, log_length, NULL, log);
	else if (glIsProgram(object))
		glGetProgramInfoLog(object, log_length, NULL, log);
	
	std::cerr << log;
	free(log);
}

GLuint create_shader(const char* filename, GLenum type) {
	const GLchar* source = file_read(filename);
	if (source == NULL) {
		std::cerr << "Error opening " << filename << ": " << SDL_GetError() << std::endl;
		return 0;
	}
	GLuint res = glCreateShader(type);

	glShaderSource(res, 1, &source, NULL);
	free((void*)source);
	
	printf("Compiling shader %s\n", filename);
	glCompileShader(res);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE) {
		std::cerr << filename << ":";
		print_log(res);
		glDeleteShader(res);
		return 0;
	}
	
	return res;
}

GLuint create_program(const char* vert_path, const char* frag_path) {
	GLint link_ok = GL_FALSE;
	
	GLuint vs, fs;
	GLuint program;

	if(
		(vs = create_shader(vert_path, GL_VERTEX_SHADER)) == 0 || 
		(fs = create_shader(frag_path, GL_FRAGMENT_SHADER)) == 0
	) { program = 0; }

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		std::cerr << "Error in glLinkProgram" << std::endl;
		program = 0;
	}

	return program;
}

bool bind_attrib(GLuint* loc, GLuint program, const char* attribute_name) {
	*loc = glGetAttribLocation(program, attribute_name);
	if (*loc == -1) {
		std::cerr << "Could not bind attribute " << attribute_name << std::endl;
		return false;
	}

	return true;
}