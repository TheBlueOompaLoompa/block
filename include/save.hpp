#pragma once
#include <glm/glm.hpp>

struct WorldSaveData {
	glm::vec3 player_pos;
	glm::vec2 look_dir;
	int seed;
};