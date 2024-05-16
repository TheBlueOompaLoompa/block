#pragma once
#include "worldgen.hpp"
#include <glm/glm.hpp>

struct World {
	glm::vec3 player_pos;
	glm::vec2 look_dir;
    WorldGeneration generation;
};

struct NewSaveData {
	int version = 0;
	glm::vec3 player_pos;
	glm::ivec2 root_chunk_offset;
	glm::vec2 look_dir;
	int seed;
};
