#pragma once

#include <glm/glm.hpp>

enum BlockType
{
    AIR,
    DIRT
};

class Block
{
public:
    glm::vec3 pos = glm::vec3(0.0);
    BlockType type;

    Block(BlockType p_type, glm::vec3 p_pos);
    ~Block();
};