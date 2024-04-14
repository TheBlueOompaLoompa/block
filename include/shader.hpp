/**
 * Store all the file's contents in memory, useful to pass shaders
 * source code to OpenGL.  Using SDL_RWops for Android asset support.
 */
#include <GL/glew.h>
#include <string.h>
#include <filesystem>

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

// TODO: If needed add support for sub-dependencies
std::string parse_includes(const GLchar* source, const char* filename) {
	std::string sauce = std::string(source);
	auto file_path = std::filesystem::path(filename);

	// Hold current search window
	char include_start_buf[10] = "        ";

	for(int i = 0; i < sauce.length(); i++) {
		for(int x = 0; x < sizeof(include_start_buf) - 1; x++) {
			include_start_buf[x] = include_start_buf[x + 1];
		}

		include_start_buf[sizeof(include_start_buf)-2] = sauce[i];

		if(std::strcmp(include_start_buf, "\n#include") == 0) {
			int start = i - 7;
			while(sauce[i - 1] != '"') { i++; }
			
			std::string path = "";
			while(sauce[i] != '"') {
				path += sauce[i];
				i++;
			}

			file_path.replace_filename(path);
			
			const GLchar* include_source = file_read(file_path.c_str());
			if (include_source == NULL) {
				std::cerr << "Error opening " << file_path << " included in " << filename << ": " << SDL_GetError() << std::endl;
				return sauce;
			}

			sauce.replace(start, i - start + 1, include_source);
		}
	}

	return sauce;
}

GLuint create_shader(const char* filename, GLenum type) {
	const GLchar* source = file_read(filename);
	if (source == NULL) {
		std::cerr << "Error opening " << filename << ": " << SDL_GetError() << std::endl;
		return 0;
	}

	auto new_source = parse_includes(source, filename);
	free((void*)source);

	GLuint res = glCreateShader(type);

	const char* const new_new_source = new_source.c_str();
	glShaderSource(res, 1, &new_new_source, NULL);
	
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