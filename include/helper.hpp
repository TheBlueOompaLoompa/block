#pragma once
#include <glm/glm.hpp>

// Consts
#define BLACK 0.0, 0.0, 0.0
#define WHITE 1.0, 1.0, 1.0
#define V3UP glm::vec3(0.0f, 1.0f, 0.0f)
#define V3RIGHT glm::vec3(1.0f, 0.0f, 0.0f)
#define V3FORWARD glm::vec3(0.0f, 0.0f, 1.0f)

// Vector helpers
#define V3VECARRAY(type) std::vector<std::vector<std::vector<type>>>
#define V3DIST(v1,v2) sqrtf(powf((v1.x - v2.x), 2) + powf((v1.y - v2.y), 2) + powf((v1.z - v2.z), 2))
#define V3FMT(vec) vec.x, vec.y, vec.z
#define V2FMT(vec) vec.x, vec.y