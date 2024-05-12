#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <string.h>
#include <iostream>
#include <map>
#include <filesystem>

#include "util.hpp"

class Shader {
public:
	// path/to/shadername (.v/f.glsl)
	Shader(const char* path);

	void use();
	void use(GLuint vbo_vertices, GLuint normal);

	bool bind_attrib(const char* attribute_name);
	GLuint get_attrib(const char* attribute_name);
	GLuint get_program_id();
	void destroy();
private:
	GLuint m_program;

	std::map<std::string, GLuint> m_va_arrays;

	// COPY: Function is from the same tutorial on loading shader files
	void m_print_log(GLuint object);
	// TODO: If needed add support for sub-dependencies
	std::string m_parse_includes(const GLchar* source, const char* filename);

	GLuint m_create_shader(const char* filename, GLenum type);
	GLuint m_create_program(const char* vert_path, const char* frag_path);
};

#endif
