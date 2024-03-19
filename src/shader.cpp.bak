
#include <iostream>
#include "include/shader.hpp"
#include "util.hpp"
#include "shader.hpp"

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

Shader::Shader (const char* name) {
    path = (char*)name;
}

Shader::~Shader()
{
    glDeleteProgram(program_id);
}

bool Shader::load() {
    GLuint vs, fs;

    std::string path_string(path);

    if(( vs = create_shader((path_string + std::string(".v.glsl")).c_str(), GL_VERTEX_SHADER)) == 0) return false;
    if(( fs = create_shader((path_string + std::string(".f.glsl")).c_str(), GL_FRAGMENT_SHADER)) == 0) return false;

    GLint link_ok = GL_FALSE;

    program_id = glCreateProgram();
	glAttachShader(program_id, vs);
	glAttachShader(program_id, fs);
	glLinkProgram(program_id);
	glGetProgramiv(program_id, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		printf("Error in glLinkProgram\n");
		return false;
	}

    return true;
}