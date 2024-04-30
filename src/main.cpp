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
#include <sys/stat.h>

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
#include "physics.hpp"
#include "save.hpp"

using namespace std;

GLuint program;
GLuint texture_atlas_id, program_id;
GLint uniform_mytexture;
GLuint attribute_coord3d, attribute_texcoord, attribute_normal;
GLint uniform_mvp;
GLint uniform_time;

vector<vector<vector<Chunk*>>> chunks;
vector<Entity> entities;

UIData ui;

Preferences prefs;
WorldSaveData world_save;

glm::vec3 offset = glm::vec3(0.0f);
glm::vec3 vel;
glm::vec2 look_dir;

bool init_resources(SDL_Renderer* renderer) {
	if (TTF_Init() < 0) {
			fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());
			return false;
		}

	SDL_Surface* res_texture = IMG_Load("res/textures/atlas.png");
	if (res_texture == NULL) {
		cerr << "IMG_Load: " << SDL_GetError() << endl;
		return false;
	}
	glGenTextures(1, &texture_atlas_id);
	glBindTexture(GL_TEXTURE_2D, texture_atlas_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	ui.res.atlas = &texture_atlas_id;

	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA,
		res_texture->w,
		res_texture->h,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
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
	world_save = {
		.player_pos = glm::vec3(LOAD_DISTANCE*CHUNK_SIZE/2.0f, WORLD_HEIGHT*CHUNK_SIZE, LOAD_DISTANCE*CHUNK_SIZE/2.0f),
		.look_dir = look_dir,
		.seed = world_seed
	};

	if(ui.state.create_menu) {
		world_seed = ui.state.new_world_seed;
		chk_mkdir((char*)ui.state.save_folder.c_str());
	}else {
		filesystem::path path = ui.state.save_folder;
		path.concat("/world.bin");
		if(std::filesystem::exists(path)) {
			load_data((char*)path.c_str(), &world_save, sizeof(WorldSaveData));	
		}

		world_seed = world_save.seed;
	}

	// MARK: Chunk gen
	
	for(int cx = 0; cx < chunks.size(); cx++) {
		for(int cy = 0; cy < chunks[cx].size(); cy++) {
			for(int cz = 0; cz < chunks[cx][cy].size(); cz++) {
				if(chunks[cx][cy][cz] != nullptr) {
					chunks[cx][cy][cz]->destroy();
					delete chunks[cx][cy][cz];
				}
			}
		}
	}

	chunks = V3VECARRAY(Chunk*)(LOAD_DISTANCE);
	for(int cx = 0; cx < LOAD_DISTANCE; cx++) {
		chunks[cx] = vector<vector<Chunk*>>(WORLD_HEIGHT);
		for(int cy = 0; cy < WORLD_HEIGHT; cy++) {
			chunks[cx][cy] = vector<Chunk*>(LOAD_DISTANCE);
			for(int cz = 0; cz < LOAD_DISTANCE; cz++) {
				chunks[cx][cy][cz] = new Chunk();
				chunks[cx][cy][cz]->position = glm::ivec3(cx, cy, cz);

				chunks[cx][cy][cz]->load(ui.state.save_folder.c_str());

				#define HEIGHT_OFFSET(x, offset) x + (offset)/(float)(CHUNK_SIZE * WORLD_HEIGHT)
				#define MAKE_WORLD(cx, bx) cx * CHUNK_SIZE + bx

				if(!chunks[cx][cy][cz]->changed) {
					for(int bx = 0; bx < CHUNK_SIZE; bx++) {
						for(int bz = 0; bz < CHUNK_SIZE; bz++) {
							float height = height_at_pos(MAKE_WORLD(cx, bx), MAKE_WORLD(cz, bz));
							for(int by = 0; by < CHUNK_SIZE; by++) {
								float relative_y = (float)(cy * CHUNK_SIZE + by) / (float)(CHUNK_SIZE * WORLD_HEIGHT);
								BlockType new_type = BlockType::AIR;

								if(HEIGHT_OFFSET(height, -10.0) > relative_y) {
									new_type = cave_gen(MAKE_WORLD(cx, bx), MAKE_WORLD(cy, by), MAKE_WORLD(cz, bz));
								}else if(HEIGHT_OFFSET(height, -3.0) > relative_y) {
									new_type = BlockType::STONE;
								}else if(height > relative_y) {
									new_type = BlockType::DIRT;
								} else if(HEIGHT_OFFSET(height, 1.0f) > relative_y) {
									new_type = BlockType::GRASS;
								}

								chunks[cx][cy][cz]->blocks[bx][by][bz].type = new_type;
							}
						}
					}
				}
			}
		}
	}

	for(int x = 0; x < LOAD_DISTANCE; x++) {
		for(int y = 0; y < WORLD_HEIGHT; y++) {
			for(int z = 0; z < LOAD_DISTANCE; z++) {
				if(x > 0) { chunks[x][y][z]->adjacent_chunks[3] = chunks[x - 1][y][z]; }
				if(x + 1 < LOAD_DISTANCE) { chunks[x][y][z]->adjacent_chunks[2] = chunks[x + 1][y][z]; }
				
				if(y > 0) { chunks[x][y][z]->adjacent_chunks[1] = chunks[x][y - 1][z]; }
				if(y + 1 < WORLD_HEIGHT) chunks[x][y][z]->adjacent_chunks[0] = chunks[x][y + 1][z];

				if(z > 0) { chunks[x][y][z]->adjacent_chunks[5] = chunks[x][y][z - 1]; }
				if(z + 1 < LOAD_DISTANCE) chunks[x][y][z]->adjacent_chunks[4] = chunks[x][y][z + 1];
				
				chunks[x][y][z]->update_mesh();
			}
		}
	}

	entities.clear();
	entities.push_back({
		type: EntityType::Player,
		name: (char*)"Player",
		position: world_save.player_pos,
	});

	if(ui.state.create_menu) {
		glm::vec3 hit_pos;

		bool hit_ground = raycast(&chunks, &entities[0].position, V3UP, &hit_pos, 16.0f*(float)WORLD_HEIGHT, .001f);

		if(hit_ground) {
			entities[entities.size() - 1].position = hit_pos;
		}
	}
}

// MARK: Save
void save() {
	char* save_bin_path = (char*)malloc(100);

	sprintf(save_bin_path, "%s/world.bin", ui.state.save_folder.c_str());

	world_save = {
		.player_pos = entities[0].position,
		.look_dir = look_dir,
		.seed = world_seed
	};

	save_data(save_bin_path, &world_save, sizeof(WorldSaveData));
	free(save_bin_path);

	for(int x = 0; x < LOAD_DISTANCE; x++) {
		for(int y = 0; y < WORLD_HEIGHT; y++) {
			for(int z = 0; z < WORLD_HEIGHT; z++) {
				if(chunks[x][y][z]->changed) chunks[x][y][z]->save((char*)ui.state.save_folder.c_str());
			}
		}
	}

	prefs.save();
}

glm::mat4 camera_transform = glm::mat4(1.0f);

bool firstDelta = true;
double last_time = 0;
double dt = 0;
bool m_left, m_right, m_up, m_down, m_shift, m_ctrl, m_space, m_click, m_click_lr = false;

glm::quat orientation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));

BlockType block_place_type = BlockType::DIRT;

#define CLAMP_CHUNK(x, max) min(max(0.0f, x), max)

void player_logic(Entity* player, float dt) {
	glm::vec2 move = glm::vec2(0.0f);

	if(m_up) move.y = -1;
	if(m_down) move.y = 1;
	if(m_left) move.x = -1;
	if(m_right) move.x = 1;

	// Normalize angular movement
	move = abs(move.x) + abs(move.y) > 0 ? glm::normalize(move) : move;

	// TODO: RE-ENABLE running when it doesn't get you stuck
	float y_angle = player->look_dir.x;
	player->velocity.x = (cos(y_angle) * move.x - sin(y_angle) * move.y) * WALK_SPEED * (m_ctrl ? RUN_MULTIPLIER : 1.0f);
	player->velocity.z = (sin(y_angle) * move.x + cos(y_angle) * move.y) * WALK_SPEED * (m_ctrl ? RUN_MULTIPLIER : 1.0f);

	if(m_space && player->is_grounded) {
		player->velocity.y = JUMP_FORCE;
	}

	ui.pos = player->position;
	ui.vel = player->velocity;

	player->orientation = orientation;
	offset = -player->position;

	glm::vec3 check_pos = player->position;
	check_pos.y += PLAYER_HEIGHT - .5f;

	//raycast(&chunks, &check_pos, orientation, &ui.hit_pos);

	if(m_click) { // MARK: Block place/break
		glm::vec3 hit_pos;

		if(raycast(&chunks, &check_pos, orientation, &hit_pos, 5.0f)) {
			#define CPOS(x) floor(x/CHUNK_SIZE)
			int cx = CPOS(hit_pos.x);
			int cy = CPOS(hit_pos.y);
			int cz = CPOS(hit_pos.z);

			#define BPOS(x) (int)floor(x)%16
			int bx = BPOS(hit_pos.x);
			int by = BPOS(hit_pos.y);
			int bz = BPOS(hit_pos.z);

			if(m_click_lr) {
				chunks[cx][cy][cz]
					->blocks[bx][by][bz].type = BlockType::AIR;
			}else {
				glm::vec3 block_hit = glm::vec3(fmodf(hit_pos.x, CHUNK_SIZE), fmodf(hit_pos.y, CHUNK_SIZE), fmodf(hit_pos.z, CHUNK_SIZE));

				// Centered
				glm::vec3 block_center = glm::vec3((float)bx + .5f, (float)by + .5f, (float)bz + .5f);

				glm::vec3 ray2center = block_hit - block_center;
				glm::vec3 ray_out;
				if(abs(ray2center.x) > abs(ray2center.y) && abs(ray2center.x) > abs(ray2center.z)) {
					ray_out = ray2center.x > 0.0f ? V3RIGHT : -V3RIGHT;
				}else if(abs(ray2center.y) > abs(ray2center.x) && abs(ray2center.y) > abs(ray2center.z)) {
					ray_out = ray2center.y > 0.0f ? V3UP : -V3UP;
				}else {
					ray_out = ray2center.z > 0.0f ? V3FORWARD : -V3FORWARD;
				}

				int nbx = bx + (int)ray_out.x;
				int nby = by + (int)ray_out.y;
				int nbz = bz + (int)ray_out.z;

				cx += nbx > CHUNK_SIZE - 1 ? 1 : (nbx < 0 ? -1 : 0);
				cy += nby > CHUNK_SIZE - 1 ? 1 : (nby < 0 ? -1 : 0);
				cz += nbz > CHUNK_SIZE - 1 ? 1 : (nbz < 0 ? -1 : 0);

				nbx = (nbx < 0 ? CHUNK_SIZE - 1 : nbx) % CHUNK_SIZE;
				nby = (nby < 0 ? CHUNK_SIZE - 1 : nby) % CHUNK_SIZE;
				nbz = (nbz < 0 ? CHUNK_SIZE - 1 : nbz) % CHUNK_SIZE;
				
				chunks[cx][cy][cz]
					->blocks[nbx][nby][nbz].type = block_place_type;
			}

			chunks[cx][cy][cz]->changed = true;
			
			chunks[floor(hit_pos.x/CHUNK_SIZE)][floor(hit_pos.y/CHUNK_SIZE)][floor(hit_pos.z/CHUNK_SIZE)]->update_area(chunks);
		}
		ui.hit_pos = hit_pos;

		m_click = false;
	}
}

float gt = 0;

void logic() {
	double ticks = (double)SDL_GetTicks64();
	if(!firstDelta) {
		dt = (ticks - last_time) / 1000.0;
	} else firstDelta = false;

	last_time = ticks;

	gt = SDL_GetTicks() / 1000.0f / DAY_NIGHT_TIME / 60.0f + .5f;

	ui.fps = 1000.0 / (dt * 1000.0);

	for(Entity& entity : entities) {
		if(!entity.is_owned) continue;

		switch (entity.type) {
			case EntityType::Player:
				entity.look_dir = look_dir;
				player_logic(&entity, dt);
				break;
		}

		process_entity_physics(entity, &chunks, dt);
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

// MARK: Rendering

void render_chunks(SDL_Renderer* renderer, SDL_Window* window) {
	glUseProgram(program);

	for(int x = 0; x < LOAD_DISTANCE; x++) {
		for(int y = 0; y < WORLD_HEIGHT; y++) {
			for(int z = 0; z < LOAD_DISTANCE; z++) {
				glEnableVertexAttribArray(attribute_coord3d);
				glBindBuffer(GL_ARRAY_BUFFER, chunks[x][y][z]->vbo_vertices);
				glVertexAttribPointer(
					attribute_coord3d,
					3,
					GL_FLOAT,
					GL_FALSE,
					sizeof(Vertex),
					0 // offset of the first element
				);

				glEnableVertexAttribArray(attribute_texcoord);
				glBindBuffer(GL_ARRAY_BUFFER, chunks[x][y][z]->vbo_vertices);
				glVertexAttribPointer(
					attribute_texcoord, // attribute
					2,                  // number of elements per vertex, here (x,y)
					GL_FLOAT,           // the type of each element
					GL_FALSE,           // take our values as-is
					sizeof(Vertex),                  // no extra data between each position
					(GLvoid*) (3 * sizeof(GLfloat))                   // offset of first element
				);

				glEnableVertexAttribArray(attribute_normal);
				glBindBuffer(GL_ARRAY_BUFFER, chunks[x][y][z]->nbo_normals);
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
				glBindTexture(GL_TEXTURE_2D, texture_atlas_id);
				
				/* Push each element in buffer_vertices to the vertex shader */
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunks[x][y][z]->ibo_elements);
				int size;  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
				glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
			}
		}
	}
}

void render(SDL_Renderer* renderer, SDL_Window* window) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);

	glClearColor(0.3, 0.5, 0.8, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
	if(!ui.main_menu) render_chunks(renderer, window);
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
	glDeleteTextures(1, &texture_atlas_id);
	
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
}

// MARK: Input
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
	}else if(code == prefs.input.sprint) {
		m_ctrl = val;
	}else if(code == prefs.input.jump) {
		m_space = val;
	}else if(code >= SDL_SCANCODE_1 && code <= SDL_SCANCODE_4) {
		block_place_type = (BlockType)(code - 29);
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

		if(!ui.main_menu) logic();
		
		render(renderer, window);
	}
}

int main(int argc, char* argv[]) {
	printf("Start Block\n");

	prefs.load();

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

	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	
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
	if (!init_resources(renderer))
		return EXIT_FAILURE;

	apply_prefs(window);

	chk_mkdir("worlds");

	std::string world_list;

	for(const auto& dir : filesystem::directory_iterator("worlds")) {
		world_list.append(dir.path());
		world_list.push_back('\0');
	}

	ui.save_folders = world_list.c_str();
	ui.setup_func = setup;
	ui.save_func = save;
	reset_ui(&ui);

	mainLoop(renderer, window, io);

	/* If the program exits in the usual way,
	   free resources and exit with a success */
	free_resources();
	return EXIT_SUCCESS;
}