#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "mesh/mesh.hpp"

enum EntityType {
    Player
};

struct Entity {
    EntityType type;
    const char* name;

    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 velocity = glm::vec3(0, 0, 0);
    glm::vec2 look_dir = glm::vec2(0, 0);
    glm::quat orientation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
    
    Mesh mesh;

    bool is_owned = true;
    bool is_grounded = false;

    void render(GLuint program) {
        /*if(mesh != NULL) {
            glUseProgram(program);
            mesh.render()
        }*/
    }
};
