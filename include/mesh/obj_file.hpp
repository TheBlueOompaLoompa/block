#pragma once

#include <vector>
#include <string>
#include "geometry.hpp"

struct Object {
    char* name;
    std::vector<Vertex> verticies;
    std::vector<glm::vec3> normals;
    std::vector<GLushort> indicies;
};

struct ObjFile {
    std::vector<Object> objects;
};

