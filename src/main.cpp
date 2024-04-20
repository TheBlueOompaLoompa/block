/*
USE W A S D to move
LOOK with your mouse
*/


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
#include "FastNoiseLite.h"

// Engine
#include "config.hpp"
#include "shader.hpp"
#include "chunk.hpp"
#include "helper.hpp"
#include "ui.hpp"
#include "entity.hpp"
//#include "worldgen.hpp"
#include "preferences.hpp"
#include "physics.hpp"

using namespace std;

GLuint program;
GLuint texture_id, program_id;
GLint uniform_mytexture;
GLuint attribute_coord3d, attribute_texcoord, attribute_normal;
GLint uniform_mvp;
GLint uniform_time;

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
	FastNoiseLite noise(0);
	noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

	#define NOISE_SCALE 2.0f

	chunks = vector<vector<vector<Chunk>>>(LOAD_DISTANCE);
	for(int x = 0; x < LOAD_DISTANCE; x++) {
		chunks[x] = vector<vector<Chunk>>(WORD_HEIGHT);
		for(int y = 0; y < WORD_HEIGHT; y++) {
			chunks[x][y] = vector<Chunk>(LOAD_DISTANCE);
			for(int z = 0; z < LOAD_DISTANCE; z++) {
				chunks[x][y][z].x = x;
				chunks[x][y][z].y = y;
				chunks[x][y][z].z = z;

				#define height_function(topper) \
					(noise.GetNoise((float)(x * CHUNK_SIZE + bx), (float)(z * CHUNK_SIZE + bz)) + 1.0f) / 2.0f / 2.0f \
					/ (noise.GetNoise((float)(x * 4.0f + bx), (float)(z  * 4.0f + bz)) + 1.0f) / 2.0f / 2.0f /2.0f \ 
					> (float)(y * CHUNK_SIZE + by topper) / WORD_HEIGHT / CHUNK_SIZE

				for(int by = 0; by < CHUNK_SIZE; by++) {
					for(int bz = 0; bz < CHUNK_SIZE; bz++) {
						for(int bx = 0; bx < CHUNK_SIZE; bx++) {
							if(height_function()) {
								chunks[x][y][z].blocks[bx][by][bz].type = BlockType::DIRT;	
							} else if(height_function(- 1.0f)) {
								chunks[x][y][z].blocks[bx][by][bz].type = BlockType::GRASS;	
							} else chunks[x][y][z].blocks[bx][by][bz].type = BlockType::AIR;
						}
					}
				}
			}
		}
	}

	for(int x = 0; x < LOAD_DISTANCE; x++) {
		for(int y = 0; y < WORD_HEIGHT; y++) {
			for(int z = 0; z < LOAD_DISTANCE; z++) {
				if(x > 0) { chunks[x][y][z].adjacent_chunks[3] = &chunks[x - 1][y][z]; }
				if(x + 1 < LOAD_DISTANCE) { chunks[x][y][z].adjacent_chunks[2] = &chunks[x + 1][y][z]; }
				
				if(y > 0) { chunks[x][y][z].adjacent_chunks[1] = &chunks[x][y - 1][z]; }
				if(y + 1 < WORD_HEIGHT) chunks[x][y][z].adjacent_chunks[0] = &chunks[x][y + 1][z];

				if(z > 0) { chunks[x][y][z].adjacent_chunks[4] = &chunks[x][y][z + 1]; }
				if(z + 1 < LOAD_DISTANCE) chunks[x][y][z].adjacent_chunks[5] = &chunks[x][y][z - 1];
				
				chunks[x][y][z].update_mesh();
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
	if (uniform_mvp == -1) {
		cerr << "Could not bind uniform " << uniform_name << endl;
		return false;
	}

	uniform_name = "time";
	uniform_time = glGetUniformLocation(program, uniform_name);
	if (uniform_time == -1) {
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
		position: glm::vec3(10.0f, 16.0f*WORD_HEIGHT, 10.0f),
	});

	glm::vec3 hit_pos;

	if(raycast(&chunks, &entities[entities.size() - 1].position, glm::quat(glm::vec3(-M_PIf, 0.0, 0.0)), &hit_pos, 16.0f*WORD_HEIGHT + 20.0f, 0.1f)) {
		entities[entities.size() - 1].position = hit_pos;
	}
}

glm::vec3 offset = glm::vec3(0.0f);
glm::vec3 vel;
glm::vec2 look_dir;

glm::mat4 camera_transform = glm::mat4(1.0f);

bool firstDelta = true;
double last_time = 0;
double dt = 0;
bool m_left, m_right, m_up, m_down, m_shift, m_space, m_click, m_click_lr = false;

glm::quat orientation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));

#define CLAMP_CHUNK(x, max) min(max(0.0f, x), max)

void player_logic(Entity* player, float dt) {
	player->velocity;

	glm::vec2 move = glm::vec2(0.0f);

	if(m_up) move.y = -1;
	if(m_down) move.y = 1;
	if(m_left) move.x = -1;
	if(m_right) move.x = 1;

	// Normalize angular movement
	move /= abs(move.x) + abs(move.y) > 1 ? sqrt(2) : 1;

	float y_angle = player->look_dir.x;
	player->velocity.x = (cos(y_angle) * move.x - sin(y_angle) * move.y) * WALK_SPEED;
	player->velocity.z = (sin(y_angle) * move.x + cos(y_angle) * move.y) * WALK_SPEED;

	bool grounded = false;
	for(int x = 0; x < 2; x++) {
		for(int y = 0; y < 2; y++) {
			glm::vec3 ray_start = player->position + glm::vec3((float)x*.90f - 0.45f, 0.0f, (float)y*.90f - 0.45f);
			if(raycast(&chunks, &ray_start, glm::quat(glm::vec3(-M_PIf, 0.0f, 0.0f)), nullptr, .01f, .001f)) {
				grounded = true;
			}
		}		
	}

	player->velocity.y = max(player->velocity.y - (GRAVITY*dt), -GRAVITY);
	if(m_space && grounded) {
		player->velocity.y = JUMP_FORCE;
	}
	//if(m_shift) player->velocity.y = -MOVE_SPEED;

	if(grounded) { player->velocity.y = max(player->velocity.y, 0.0f); }

	glm::vec3 check_pos = player->position;
	check_pos.y += 0.1f;



	bool hit_pos_z = raycast(&chunks, &check_pos, glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)), nullptr, .5f, .001f);
	bool hit_pos_x = raycast(&chunks, &check_pos, glm::quat(glm::vec3(0.0f, M_PIf/2.0f, 0.0f)), nullptr, .5f, .001f);
	bool hit_neg_z = raycast(&chunks, &check_pos, glm::quat(glm::vec3(0.0f, M_PIf, 0.0f)), nullptr, .5f, .001f);
	bool hit_neg_x = raycast(&chunks, &check_pos, glm::quat(glm::vec3(0.0f, -M_PIf/2.0f, 0.0f)), nullptr, .5f, .001f);

	//printf("+x %i +z %i -x %i -z %i\n", hit_pos_x, hit_pos_z, hit_neg_x, hit_neg_z);

	if(hit_pos_x) player->velocity.x = min(player->velocity.x, 0.0f);
	if(hit_pos_z) player->velocity.z = max(player->velocity.z, 0.0f);
	if(hit_neg_x) player->velocity.x = max(player->velocity.x, 0.0f);
	if(hit_neg_z) player->velocity.z = min(player->velocity.z, 0.0f);

	/* 
		Rotate velocity
		glm::rotate(glm::inverse(orientation), V3FORWARD)
		glm::rotate(glm::inverse(orientation), V3RIGHT)
	*/

	player->position += V3FORWARD * player->velocity.z * dt;
	player->position += V3UP * player->velocity.y * dt;
	player->position += V3RIGHT * player->velocity.x * dt;
	ui.pos = player->position;
	ui.vel = player->velocity;

	player->orientation = orientation;
	offset = -player->position;

	check_pos.y += PLAYER_HEIGHT - .6f;

	//raycast(&chunks, &check_pos, orientation, &ui.hit_pos);

	if(m_click) {
		glm::vec3 hit_pos;

		if(raycast(&chunks, &check_pos, orientation, &hit_pos, 5.0f)) {
			chunks[floor(hit_pos.x/CHUNK_SIZE)][floor(hit_pos.y/CHUNK_SIZE)][floor(hit_pos.z/CHUNK_SIZE)]
				.blocks[(int)floor(hit_pos.x)%16][(int)floor(hit_pos.y)%16][(int)floor(hit_pos.z)%16].type = m_click_lr ? BlockType::AIR : BlockType::DIRT;
			
			chunks[floor(hit_pos.x/CHUNK_SIZE)][floor(hit_pos.y/CHUNK_SIZE)][floor(hit_pos.z/CHUNK_SIZE)].update_area(chunks);
		}
		ui.hit_pos = hit_pos;

		m_click = false;
	}
}

float gt = 0;

void logic() {
	if(!firstDelta) {
		dt = ((double)SDL_GetTicks64() - last_time) / 1000.0;
	} else firstDelta = false;

	world->update(dt);
	last_time = SDL_GetTicks64();

	gt = SDL_GetTicks() / 1000.0f / DAY_NIGHT_TIME / 60.0f + .5f;

	ui.fps = dt * 1000.0;
	//ui.fps /= 2.0f;

	for(Entity& entity : entities) {
		if(!entity.is_owned) continue;
		switch (entity.type) {
			case EntityType::Player:
				entity.look_dir = look_dir;
				player_logic(&entity, dt);
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
	
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, -PLAYER_HEIGHT + .5f, 0));
	glm::mat4 view = glm::translate(glm::mat4(1.0), glm::vec3(0.0));

	glm::quat pitch = glm::angleAxis(look_dir.y, V3RIGHT);
	glm::quat yaw = glm::angleAxis(look_dir.x, V3UP);

	orientation = pitch * yaw;
	orientation = glm::normalize(orientation);
	glm::mat4 rotate = glm::mat4_cast(orientation);

	view *= rotate;
	view = glm::translate(view, offset);

	glm::mat4 projection = glm::perspective(prefs.graphics.fov * M_PIf / 180.0f, 1.0f*prefs.width/prefs.height, 0.1f, 10000.0f);

	glm::mat4 mvp = projection * view * model;

	glUseProgram(program);
	glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
	glUniform1f(uniform_time, gt);
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
					m_click = true;
					m_click_lr = ev.button.button == SDL_BUTTON_LEFT;

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

	Preferences::load(&prefs);

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

	apply_prefs(window);

	/* We can display something if everything goes OK */
	setup();
	mainLoop(renderer, window, io);

	Preferences::save(&prefs);

	/* If the program exits in the usual way,
	   free resources and exit with a success */
	free_resources();
	return EXIT_SUCCESS;
}