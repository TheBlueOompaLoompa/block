#pragma once
#include <glm/glm.hpp>

struct WorldSaveData {
	glm::vec3 player_pos;
	glm::vec2 look_dir;
	int seed;
};

struct NewSaveData {
	int version = 0;
	glm::vec3 player_pos;
	glm::ivec2 root_chunk_offset;
	glm::vec2 look_dir;
	int seed;
};