/* Using standard C++ output libraries */
#include <cstdlib>
#include <iostream>
#include <math.h>
using namespace std;

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL.h>

#include "../include/shader.hpp"
#include "../include/helper.hpp"

struct attributes {
	GLfloat coord3d[3];
	GLfloat v_color[3];
};

struct attributes triangle_attributes[] = {
    {{ 0.0,  0.8, 0.0}, {1.0, 1.0, 0.0}},
    {{-0.8, -0.8, 0.0}, {0.0, 0.0, 1.0}},
    {{ 0.8, -0.8, 0.0}, {1.0, 0.0, 0.0}}
};

GLuint program;
GLuint vbo_triangle, vbo_triangle_colors;
GLint attribute_coord3d, attribute_v_color;
GLint uniform_fade;
GLint uniform_m_transform;

bool init_resources() {
	glGenBuffers(1, &vbo_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_attributes), triangle_attributes, GL_STATIC_DRAW);

	GLfloat triangle_colors[] = {
		1.0, 1.0, 0.0,
		0.0, 0.0, 1.0,
		1.0, 0.0, 0.0,
	};
	glGenBuffers(1, &vbo_triangle_colors);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle_colors);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_colors), triangle_colors, GL_STATIC_DRAW);

	GLint compile_ok = GL_FALSE, link_ok = GL_FALSE;
	
	GLuint vs, fs;

	if((vs = create_shader("res/shaders/triangle.vs.glsl", GL_VERTEX_SHADER)) == 0) return false;
	if((fs = create_shader("res/shaders/triangle.fs.glsl", GL_FRAGMENT_SHADER)) == 0) return false;

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		cerr << "Error in glLinkProgram" << endl;
		return false;
	}
	const char* attribute_name = "coord3d";
	attribute_coord3d = glGetAttribLocation(program, attribute_name);
	if (attribute_coord3d == -1) {
		cerr << "Could not bind attribute " << attribute_name << endl;
		return false;
	}

	attribute_name = "v_color";
	attribute_v_color = glGetAttribLocation(program, attribute_name);
	if (attribute_v_color == -1) {
		cerr << "Could not bind attribute " << attribute_name << endl;
		return false;
	}

	const char* uniform_name;
	uniform_name = "fade";
	uniform_fade = glGetUniformLocation(program, uniform_name);
	if (uniform_fade == -1) {
		cerr << "Could not bind uniform " << uniform_name << endl;
		return false;
	}

	uniform_name = "m_transform";
	uniform_m_transform = glGetUniformLocation(program, uniform_name);
	if (uniform_m_transform == -1) {
		cerr << "Could not bind uniform " << uniform_name << endl;
		return false;
	}


	return true;
}


void render(SDL_Window* window) {
	/* Clear the background as white */
	glClearColor(WHITE, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);
	glEnableVertexAttribArray(attribute_coord3d);
	glEnableVertexAttribArray(attribute_v_color);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glVertexAttribPointer(
		attribute_coord3d,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct attributes),
		0 // offset of the first element
	);
	glVertexAttribPointer(
		attribute_v_color,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct attributes),
		(GLvoid*) offsetof(struct attributes, v_color)
	);

	
	/* Push each element in buffer_vertices to the vertex shader */
	glDrawArrays(GL_TRIANGLES, 0, 3);
	
	glDisableVertexAttribArray(attribute_coord3d);
	glDisableVertexAttribArray(attribute_v_color);

	/* Display the result */
	SDL_GL_SwapWindow(window);
}

void logic() {
	glUseProgram(program);
	float fade = 1.0;//sinf(SDL_GetTicks()/1000.0* M_PI)/2.0+.5;

	glUniform1f(uniform_fade, fade);

	float move = 0.0;//sinf(SDL_GetTicks() / 1000.0 * (2*3.14) / 5); // -1<->+1 every 5 seconds
	float angle = 90.0;//SDL_GetTicks() / 1000.0 * 45;  // 45° per second
	glm::vec3 axis_z(0, 0, 1);
	glm::mat4 m_transform = glm::translate(glm::mat4(1.0f), glm::vec3(move, 0.0, 0.0))
		* glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis_z);
	
	glUseProgram(program);
	glUniformMatrix4fv(uniform_m_transform, 1, GL_FALSE, glm::value_ptr(m_transform));

}

void free_resources() {
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo_triangle);
}

void mainLoop(SDL_Window* window) {
	while (true) {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				return;
		}
		render(window);
	}
}

int main(int argc, char* argv[]) {
	/* SDL-related initialising functions */
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("Block",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		640, 480,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	if (window == NULL) {
		cerr << "Error: can't create window: " << SDL_GetError() << endl;
		return EXIT_FAILURE;
	}

	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);

	if (SDL_GL_CreateContext(window) == NULL) {
		cerr << "Error: SDL_GL_CreateContext: " << SDL_GetError() << endl;
		return EXIT_FAILURE;
	}

	/* Extension wrangler initialising */
	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK) {
		cerr << "Error: glewInit: " << glewGetErrorString(glew_status) << endl;
		return EXIT_FAILURE;
	}

	if (!GLEW_VERSION_2_0) {
		cerr << "Error: your graphic card does not support OpenGL 2.0" << endl;
		return EXIT_FAILURE;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	/* When all init functions run without errors,
	   the program can initialise the resources */
	if (!init_resources())
		return EXIT_FAILURE;

	/* We can display something if everything goes OK */
	mainLoop(window);

	/* If the program exits in the usual way,
	   free resources and exit with a success */
	free_resources();
	return EXIT_SUCCESS;
}