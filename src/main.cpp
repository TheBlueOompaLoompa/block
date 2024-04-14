// Std
#include <cstdlib>
#include <iostream>
#include <vector>
#include <math.h>

// Backend
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <reactphysics3d/reactphysics3d.h>

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

// Engine
#include "config.hpp"
#include "shader.hpp"
#include "chunk.hpp"
#include "helper.hpp"
#include "ui.hpp"
#include "entity.hpp"
#include "worldgen.hpp"
#include "preferences.hpp"

using namespace std;

GLuint program;
GLuint texture_id, program_id;
GLint uniform_mytexture;
GLuint attribute_coord3d, attribute_texcoord, attribute_normal;
GLint uniform_fade;
GLint uniform_mvp;

vector<vector<vector<Chunk>>> chunks;
vector<Entity> entities;

rp3d::PhysicsCommon physicsCommon;
rp3d::PhysicsWorld* world = physicsCommon.createPhysicsWorld();

UIData ui;

TTF_Font* Sans;

GLuint height_program = 0;

Preferences prefs;

bool init_resources() {
	if (TTF_Init() < 0) {
			fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());
			return false;
		}

	Sans = TTF_OpenFont("./res/fonts/NotoSans-Regular.ttf", 12);

	if(Sans == NULL) {
		fprintf(stderr, "Failed to load font: %s\n", SDL_GetError());
		return false;
	}


	// WORLD GEN

	height_program = create_program("res/shaders/compute/compute.v.glsl", "res/shaders/compute/height.glsl");
	//GLuint height_map = gen_height_map(&height_program, CHUNK_SIZE, 0, 0, LOAD_DISTANCE, LOAD_DISTANCE);

	chunks = vector<vector<vector<Chunk>>>(LOAD_DISTANCE);
	for(int x = 0; x < LOAD_DISTANCE; x++) {
		chunks[x] = vector<vector<Chunk>>(WORD_HEIGHT);
		for(int y = 0; y < WORD_HEIGHT; y++) {
			chunks[x][y] = vector<Chunk>(LOAD_DISTANCE);
			for(int z = 0; z < LOAD_DISTANCE; z++) {
				chunks[x][y][z].x = x;
				chunks[x][y][z].y = y;
				chunks[x][y][z].z = z;

				for(int by = 0; by < CHUNK_SIZE; by++) {
					for(int bz = 0; bz < CHUNK_SIZE; bz++) {
						for(int bx = 0; bx < CHUNK_SIZE; bx++) {
							if(round(sin(bz*M_1_PI) * 2.0f) + 2 >= by) {
								chunks[x][y][z].blocks[bx][by][bz].type = BlockType::DIRT;
								if(round(sin(bz*M_1_PI) * 2.0f) + 2 == by) chunks[x][y][z].blocks[bx][by][bz].type = BlockType::GRASS;
							}else chunks[x][y][z].blocks[bx][by][bz].type = BlockType::AIR;
						}
					}
				}
			}
		}
	}

	for(int x = 0; x < LOAD_DISTANCE; x++) {
		for(int y = 0; y < WORD_HEIGHT; y++) {
			for(int z = 0; z < LOAD_DISTANCE; z++) {
				if(z > 0) chunks[x][y][z].adjacent_chunks[5] = &chunks[x][y][z - 1];
				if(z < CHUNK_SIZE - 1) chunks[x][y][z].adjacent_chunks[4] = &chunks[x][y][z + 1];
				if(x > 0) chunks[x][y][z].adjacent_chunks[2] = &chunks[x - 1][y][z];
				for(int a = 0; a < x; a++) {
					chunks[x][y][z].blocks[a][8][8].type = BlockType::GRASS;
				}
				/*if(y > 0) chunks[x][y][z].adjacent_chunks[1] = &chunks[x][y - 1][z];
				if(z > 0) chunks[x][y][z].adjacent_chunks[5] = &chunks[x][y][z - 1];

				if(x < CHUNK_SIZE - 1) chunks[x][y][z].adjacent_chunks[3] = &chunks[x + 1][y][z];
				if(y < CHUNK_SIZE - 1) chunks[x][y][z].adjacent_chunks[0] = &chunks[x][y + 1][z];
				*/
				
				chunks[x][y][z].updateMesh(&physicsCommon, world);
			}
		}
	}

	SDL_Surface* res_texture = IMG_Load("res/textures/atlas.png");
	if (res_texture == NULL) {
		cerr << "IMG_Load: " << SDL_GetError() << endl;
		return false;
	}
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

	program = create_program("res/shaders/cube.v.glsl", "res/shaders/cube.f.glsl");
	if(program == 0) { return false; }
	
	if(
		!bind_attrib(&attribute_coord3d, program, "coord3d") ||
		!bind_attrib(&attribute_texcoord, program, "texcoord") ||
		!bind_attrib(&attribute_normal, program, "normal")
	) { return false; }

	const char* uniform_name;
	uniform_name = "mvp";
	uniform_mvp = glGetUniformLocation(program, uniform_name);
	if (uniform_fade == -1) {
		cerr << "Could not bind uniform " << uniform_name << endl;
		return false;
	}

	return true;
}

void apply_prefs(SDL_Window* window) {
	SDL_SetWindowFullscreen(window, prefs.fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
}

void setup() {
	entities.push_back({
		type: EntityType::Player,
		name: (char*)"Player",
		position: glm::vec3(0.0f, 16.0f*WORD_HEIGHT, 0.0f),
	});
}

glm::vec3 offset = glm::vec3(0.0f);
glm::vec3 vel;
glm::vec2 look_dir;

glm::mat4 camera_transform = glm::mat4(1.0f);

float last_time = 0;
float dt = 0;
bool m_left, m_right, m_up, m_down, m_shift, m_space = false;

glm::quat orientation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));

void player_logic(Entity* player) {
	player->velocity = glm::vec3(0.0f);
	if(m_up) player->velocity.z = MOVE_SPEED;
	if(m_down) player->velocity.z = -MOVE_SPEED;
	if(m_left) player->velocity.x = MOVE_SPEED;
	if(m_right) player->velocity.x = -MOVE_SPEED;
	if(m_space) player->velocity.y = -MOVE_SPEED;
	if(m_shift) player->velocity.y = MOVE_SPEED;
	ui.vel = player->velocity;

	player->position -= glm::rotate(glm::inverse(orientation), V3FORWARD) * player->velocity.z * dt;
	player->position -= V3UP * player->velocity.y * dt;
	player->position -= glm::rotate(glm::inverse(orientation), V3RIGHT) * player->velocity.x * dt;
	ui.pos = player->position;

	player->orientation = orientation;
	offset = -player->position;
}

void logic() {
	dt = SDL_GetTicks() - last_time;
	world->update(dt / 1000.0f);
	last_time = SDL_GetTicks();

	ui.fps += 1.0f/(dt/1000.0f);
	ui.fps /= 2.0f;

	for(Entity& entity : entities) {
		if(!entity.is_owned) continue;
		switch (entity.type) {
			case EntityType::Player:
				player_logic(&entity);
				break;
		}
	}

	ui.look_dir = look_dir;

	// const Transform& transform = body->getTransform();
    // const Vector3& position = transform.getPosition();
	// const Quaternion& orientation = transform.getOrientation();

	float angle = SDL_GetTicks() / 1000.0 * 45;  // 45° per second
	glm::vec3 axis_y(0, 1, 0);
	//glm::mat4 anim = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis_y);
	
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, -2, 0));
	glm::mat4 view = glm::translate(glm::mat4(1.0), glm::vec3(0.0));

	glm::quat pitch = glm::angleAxis(look_dir.y, V3RIGHT);
	glm::quat yaw = glm::angleAxis(look_dir.x, V3UP);

	orientation = pitch * yaw;
	orientation = glm::normalize(orientation);
	glm::mat4 rotate = glm::mat4_cast(orientation);

	view *= rotate;
	view = glm::translate(view, offset);

	glm::mat4 projection = glm::perspective(45.0f, 1.0f*prefs.width/prefs.height, 0.1f, 300.0f);

	glm::mat4 mvp = projection * view * model;

	glUseProgram(program);
	glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

}

void render(SDL_Renderer* renderer, SDL_Window* window) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);

	/* Clear the background as white */
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	for(int x = 0; x < LOAD_DISTANCE; x++) {
		for(int y = 0; y < WORD_HEIGHT; y++) {
			for(int z = 0; z < LOAD_DISTANCE; z++) {
				glEnableVertexAttribArray(attribute_coord3d);
				glBindBuffer(GL_ARRAY_BUFFER, chunks[x][y][z].vbo_vertices);
				glVertexAttribPointer(
					attribute_coord3d,
					3,
					GL_FLOAT,
					GL_FALSE,
					sizeof(Vertex),
					0 // offset of the first element
				);

				glEnableVertexAttribArray(attribute_texcoord);
				glBindBuffer(GL_ARRAY_BUFFER, chunks[x][y][z].vbo_vertices);
				glVertexAttribPointer(
					attribute_texcoord, // attribute
					2,                  // number of elements per vertex, here (x,y)
					GL_FLOAT,           // the type of each element
					GL_FALSE,           // take our values as-is
					sizeof(Vertex),                  // no extra data between each position
					(GLvoid*) (3 * sizeof(GLfloat))                   // offset of first element
				);

				glEnableVertexAttribArray(attribute_normal);
				glBindBuffer(GL_ARRAY_BUFFER, chunks[x][y][z].nbo_normals);
				glVertexAttribPointer(
					attribute_normal,
					3,
					GL_FLOAT,
					GL_FALSE,
					sizeof(glm::vec3),
					(GLvoid*) 0
				);


				glActiveTexture(GL_TEXTURE0);
				glUniform1i(uniform_mytexture, /*GL_TEXTURE*/0);
				glBindTexture(GL_TEXTURE_2D, texture_id);
				
				/* Push each element in buffer_vertices to the vertex shader */
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunks[x][y][z].ibo_elements);
				int size;  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
				glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
			}
		}
	}

	if(render_ui(&ui, &prefs)) {
		apply_prefs(window);
	}

	/* Display the result */
	SDL_GL_SwapWindow(window);
}

void onResize(int width, int height) {
	prefs.width = width;
	prefs.height = height;
	
	glViewport(0, 0, prefs.width, prefs.height);
}


void free_resources() {
	glDeleteProgram(program);
	glDeleteTextures(1, &texture_id);
	
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
}

void check_input(SDL_Scancode code, bool val) {
	if(code == prefs.input.forward) {
		m_up = val;
	}else if(code == prefs.input.left) {
		m_left = val;
	}else if(code == prefs.input.backward) {
		m_down = val;
	}else if(code == prefs.input.right) {
		m_right = val;
	}else if(code == prefs.input.crouch) {
		m_shift = val;
	}else if(code == prefs.input.jump) {
		m_space = val;
	}else {
		switch(code) {
			case SDL_SCANCODE_F3:
				if(val) { ui.f3 = !ui.f3; }
				break;
			default: break;
		}
	}
}

void look(int x, int y) {
	look_dir.x += (float)x * LOOK_SPEED;
	// Prevent massive look numbers
	look_dir.x = (abs(look_dir.x) > 2 * M_PI) ? abs(look_dir.x) - 2 * M_PI : look_dir.x;
	look_dir.y = max(min((float)y * LOOK_SPEED + look_dir.y, M_PI_2f), -M_PI_2f);
}

void mainLoop(SDL_Renderer* renderer, SDL_Window* window, ImGuiIO& io) {
	while (true) {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			ImGui_ImplSDL2_ProcessEvent(&ev);
			switch(ev.type) {
				case SDL_QUIT:
					return;
				case SDL_WINDOWEVENT:
					if(ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
						onResize(ev.window.data1, ev.window.data2);
					break;
				case SDL_KEYDOWN:
					if(ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
						ui.esc = !ui.esc;
						SDL_SetWindowMouseGrab(window, ui.esc ? SDL_FALSE : SDL_TRUE);
						SDL_SetRelativeMouseMode(ui.esc ? SDL_FALSE : SDL_TRUE);
					}
					if(io.WantCaptureKeyboard) break;
					check_input(ev.key.keysym.scancode, true);
					break;
				case SDL_KEYUP:
					if(io.WantCaptureKeyboard) break;
					check_input(ev.key.keysym.scancode, false);
					break;
				case SDL_MOUSEBUTTONDOWN:
					if(io.WantCaptureMouse) break;
					SDL_SetWindowMouseGrab(window, SDL_TRUE);
					SDL_SetRelativeMouseMode(SDL_TRUE);
					SDL_SetCursor(NULL);
					break;
				case SDL_MOUSEMOTION:
					if(io.WantCaptureMouse) break;
					look(ev.motion.xrel, ev.motion.yrel);
					break;
				default:
					break;
			}
		}

		if(ui.quit) { return; }

		logic();
		render(renderer, window);
	}
}

int main(int argc, char* argv[]) {
	printf("Start Block\n");

	/* SDL-related initialising functions */
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window;
	SDL_Renderer* renderer;

	SDL_CreateWindowAndRenderer(
		prefs.width, prefs.height,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI,
		&window, &renderer);
	SDL_SetWindowTitle(window, GAME_TITLE);

	if (window == NULL)
	{
		cerr << "Error: can't create window: " << SDL_GetError() << endl;
		return EXIT_FAILURE;
	}

	SDL_SetHint(SDL_HINT_APP_NAME, GAME_TITLE);

	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "0");
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);

	auto context = SDL_GL_CreateContext(window);
	if (context == NULL)
	{
		cerr << "Error: SDL_GL_CreateContext: " << SDL_GetError() << endl;
		return EXIT_FAILURE;
	}

	glewExperimental = true;

	/* Extension wrangler initialising */
	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK)
	{
		cerr << "Error: glewInit: " << glewGetErrorString(glew_status) << endl;
		return EXIT_FAILURE;
	}

	if (!GLEW_VERSION_3_1)
	{
		cerr << "Error: your graphic card does not support OpenGL 3.1" << endl;
		return EXIT_FAILURE;
	}

	//glGenFramebuffers(1, 0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 330 core");


	/* When all init functions run without errors,
	   the program can initialise the resources */
	if (!init_resources())
		return EXIT_FAILURE;

	/* We can display something if everything goes OK */
	setup();
	mainLoop(renderer, window, io);

	/* If the program exits in the usual way,
	   free resources and exit with a success */
	free_resources();
	return EXIT_SUCCESS;
}