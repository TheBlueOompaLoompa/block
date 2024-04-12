#pragma once
#include <glm/glm.hpp>

#define BLACK 0.0, 0.0, 0.0
#define WHITE 1.0, 1.0, 1.0
#define V3UP glm::vec3(0.0, 1.0, 0.0)
#define V3RIGHT glm::vec3(1.0, 0.0, 0.0)
#define V3FORWARD glm::vec3(0.0, 0.0, 1.0)

#define V3FMT(vec) vec.x, vec.y, vec.z
#define V2FMT(vec) vec.x, vec.y