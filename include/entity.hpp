#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "components/component.hpp"

enum EntityType {
    Player
};

struct Entity {
    EntityType type;
    const char* name;

    std::vector<EntityComponent> components;

    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 velocity = glm::vec3(0, 0, 0);
    glm::vec2 look_dir = glm::vec2(0, 0);
    glm::quat orientation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));

    bool is_owned = true;
    bool is_grounded = false;
};