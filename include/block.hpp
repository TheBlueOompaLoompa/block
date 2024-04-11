#pragma once

#include <glm/glm.hpp>

enum BlockType
{
    AIR,
    DIRT,
    GRASS
};

struct Block {
    BlockType type = BlockType::AIR;
};