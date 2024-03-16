#pragma once

#include <glm/glm.hpp>

enum BlockType
{
    AIR,
    DIRT
};

struct Block {
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;

    BlockType type = BlockType::AIR;
};