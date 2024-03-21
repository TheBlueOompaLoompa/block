// Std
#include <cstdlib>
#include <iostream>
#include <vector>
#include <math.h>

// Backend
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <reactphysics3d/reactphysics3d.h>

// Engine
#include "../include/config.hpp"
#include "../include/shader.hpp"
#include "../include/chunk.hpp"
#include "../include/helper.hpp"

using namespace std;

GLuint program;
GLuint texture_id, program_id;
GLint uniform_mytexture;
GLuint attribute_coord3d, attribute_texcoord;
GLint uniform_fade;
GLint uniform_mvp;

vector<Chunk> chunks;

Chunk chunk;

using namespace reactphysics3d;

PhysicsCommon physicsCommon;
PhysicsWorld* world = physicsCommon.createPhysicsWorld();

int screen_width = 1280;
int screen_height = 720;

bool init_resources()
{
	for(int j = 0; j < 2; j++) {
		chunks.push_back(Chunk());
		if(j > 0) {
			chunks[j - 1].adjacent_chunks[2] = &chunks[j];
			chunks[j].adjacent_chunks[3] = &chunks[j - 1];
		}
		for(int y = 0; y < CHUNK_SIZE; y++) {
			for(int z = 0; z < CHUNK_SIZE; z++) {
				for(int x = 0; x < CHUNK_SIZE; x++) {
					if((y == 0) || (x == 4 && y != 0)) {
						chunks[j].blocks[x][y][z].type = BlockType::DIRT;
					}else chunks[j].blocks[x][y][z].type = BlockType::AIR;

					//printf("Type: %i\n", chunk.blocks[x][y][z].type);
				}
			}
		}

		chunks[j].x = j;
	}

	chunks[0].blocks[0][9][0].type = BlockType::DIRT;
	chunks[0].blocks[0][8][0].type = BlockType::DIRT;

	for(auto& chunk : chunks) {
		chunk.updateMesh(&physicsCommon, world);
		printf("Chunk X: %i Y: %i Z: %i Faces: %i\n", chunk.x, chunk.y, chunk.z, chunk.indices.size()/6);
	}

	SDL_Surface* res_texture = IMG_Load("res/textures/texture.png");
	if (res_texture == NULL) {
		cerr << "IMG_Load: " << SDL_GetError() << endl;
		return false;
	}
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, // target
		0, // level, 0 = base, no minimap,
		GL_RGBA, // internalformat
		res_texture->w, // width
		res_texture->h, // height
		0, // border, always 0 in OpenGL ES
		GL_RGBA, // format
		GL_UNSIGNED_BYTE, // type
		res_texture->pixels);
	SDL_FreeSurface(res_texture);



	GLint link_ok = GL_FALSE;
	
	GLuint vs, fs;

	if((vs = create_shader("res/shaders/cube.v.glsl", GL_VERTEX_SHADER)) == 0) return false;
	if((fs = create_shader("res/shaders/cube.f.glsl", GL_FRAGMENT_SHADER)) == 0) return false;

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		cerr << "Error in glLinkProgram" << endl;
		return false;
	}
	const char* attribute_name = "coord3d";
	attribute_coord3d = glGetAttribLocation(program, attribute_name);
	if (attribute_coord3d == -1)
	{
		cerr << "Could not bind attribute " << attribute_name << endl;
		return false;
	}

	/*attribute_name = "texcoord";
	attribute_texcoord = glGetAttribLocation(program, attribute_name);
	if (attribute_texcoord == -1) {
		cerr << "Could not bind attribute " << attribute_name << endl;
		return false;
	}*/

	const char* uniform_name;
	uniform_name = "mvp";
	uniform_mvp = glGetUniformLocation(program, uniform_name);
	if (uniform_fade == -1)
	{
		cerr << "Could not bind uniform " << uniform_name << endl;
		return false;
	}

	return true;
}

Vector3 position(0, 20, -10);
Quaternion orientation = Quaternion::identity(); 
Transform c_transform(position, orientation); 
RigidBody* body = world->createRigidBody(c_transform); 

glm::vec3 offset = glm::vec3(0, 5, 0);
glm::vec3 vel;
glm::vec2 lookDir;

glm::mat4 cameraTransform = glm::mat4(1.0f);

float last_time = 0;
float dt = 0;
bool m_left, m_right, m_up, m_down = false;

#define ROT_SPEED 1000.0f
float CAM_DIST = 15.0f;

void logic()
{
	dt = SDL_GetTicks() - last_time;
	world->update(dt / 1000.0f);
	last_time = SDL_GetTicks();

	vel = glm::vec3(0, 0, 0);

	if(m_up) {
		vel.z = MOVE_SPEED;
	}
	if(m_down) {
		vel.z = -MOVE_SPEED;
	}
	if(m_left) {
		vel.x = MOVE_SPEED;
	}
	if(m_right) {
		vel.x = -MOVE_SPEED;
	}

	offset.y += vel.x * dt;
	CAM_DIST -= vel.z * dt;


	offset.x = cos(SDL_GetTicks() / ROT_SPEED) * CAM_DIST;
	offset.z = sin(SDL_GetTicks() / ROT_SPEED) * CAM_DIST;

	const Transform& transform = body->getTransform();
    const Vector3& position = transform.getPosition();
	const Quaternion& orientation = transform.getOrientation();

	float angle = SDL_GetTicks() / 1000.0 * 45;  // 45° per second
	glm::vec3 axis_y(0, 1, 0);
	//glm::mat4 anim = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis_y);
	
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-8, -16, -8));
	glm::mat4 view = glm::translate(glm::mat4(1.0f), offset);
	view *= glm::rotate(view, glm::radians(lookDir.y), glm::vec3(1.0, 0.0, 0.0));
	view = glm::lookAt(offset + glm::vec3(0, -16, 0), glm::vec3(0, -10, 0), glm::vec3(0, 1, 0));
	glm::mat4 projection = glm::perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 300.0f);

	glm::mat4 mvp = projection * view * model;

	glUseProgram(program);
	glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

}

void render(SDL_Window* window)
{
	/* Clear the background as white */
	glClearColor(WHITE, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	for(auto& chunk : chunks) {
		//printf("Chunk: %i %i %li\n", chunk.x, chunk.y, chunk.indices.size());
		glEnableVertexAttribArray(attribute_coord3d);
		glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo_vertices);
		glVertexAttribPointer(
			attribute_coord3d,
			3,
			GL_FLOAT,
			GL_FALSE,
			5 * sizeof(GLfloat),
			0 // offset of the first element
		);

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(uniform_mytexture, /*GL_TEXTURE*/0);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		/*glEnableVertexAttribArray(attribute_texcoord);
		glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo_texcoords);
		glVertexAttribPointer(
			attribute_texcoord, // attribute
			2,                  // number of elements per vertex, here (x,y)
			GL_FLOAT,           // the type of each element
			GL_FALSE,           // take our values as-is
			5 * sizeof(GLfloat),                  // no extra data between each position
			(GLvoid*) (3 * sizeof(GLfloat))                   // offset of first element
		);*/

		
		/* Push each element in buffer_vertices to the vertex shader */
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk.ibo_elements);
		int size;  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
		glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
	}

	/* Display the result */
	SDL_GL_SwapWindow(window);
}

void onResize(int width, int height)
{
	screen_width = width;
	screen_height = height;
	glViewport(0, 0, screen_width, screen_height);
}


void free_resources()
{
	glDeleteProgram(program);
	glDeleteTextures(1, &texture_id);
	chunk.destroy();
}

void check_input(SDL_Scancode code, bool val) {
	switch(code) {
		case SDL_SCANCODE_W:
			m_up = val;
			break;
		case SDL_SCANCODE_A:
			m_left = val;
			break;
		case SDL_SCANCODE_S:
			m_down = val;
			break;
		case SDL_SCANCODE_D:
			m_right = val;
			break;
		default: break;
	}

	if(!val) {
		printf("%f %f\n", offset.x, offset.z);
	}
}

void look(int x, int y) {
	lookDir += glm::vec2(x * LOOK_SPEED, y * LOOK_SPEED);
}

void mainLoop(SDL_Window* window)
{
	while (true) {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch(ev.type) {
				case SDL_QUIT:
					return;
				case SDL_WINDOWEVENT:
					if(ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
						onResize(ev.window.data1, ev.window.data2);
					break;
				case SDL_KEYDOWN:
					check_input(ev.key.keysym.scancode, true);
					if(ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
						SDL_SetWindowMouseGrab(window, SDL_FALSE);
						SDL_SetRelativeMouseMode(SDL_FALSE);
					}
					break;
				case SDL_KEYUP:
					check_input(ev.key.keysym.scancode, false);
					break;
				case SDL_MOUSEBUTTONDOWN:
					SDL_SetWindowMouseGrab(window, SDL_TRUE);
					SDL_SetRelativeMouseMode(SDL_TRUE);
					SDL_SetCursor(NULL);
					break;
				case SDL_MOUSEMOTION:
					look(ev.motion.xrel, ev.motion.yrel);
					break;
				default:
					break;
			}
		}
		logic();
		render(window);
	}
}

int main(int argc, char* argv[])
{
	printf("Start Block\n");

	/* SDL-related initialising functions */
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("Block",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_width, screen_height,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	if (window == NULL)
	{
		cerr << "Error: can't create window: " << SDL_GetError() << endl;
		return EXIT_FAILURE;
	}

	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	//SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);

	if (SDL_GL_CreateContext(window) == NULL)
	{
		cerr << "Error: SDL_GL_CreateContext: " << SDL_GetError() << endl;
		return EXIT_FAILURE;
	}

	/* Extension wrangler initialising */
	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK)
	{
		cerr << "Error: glewInit: " << glewGetErrorString(glew_status) << endl;
		return EXIT_FAILURE;
	}

	if (!GLEW_VERSION_3_3)
	{
		cerr << "Error: your graphic card does not support OpenGL 3.3" << endl;
		return EXIT_FAILURE;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
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