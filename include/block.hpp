#pragma once

#include <glm/glm.hpp>

enum BlockType
{
    AIR,
    DIRT,
    GRASS,
    STONE,
    IRON_ORE
};

struct Block {
    BlockType type = BlockType::AIR;
};