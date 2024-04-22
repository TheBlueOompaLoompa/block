#pragma once
#include "FastNoiseLite.h"
#include "block.hpp"

#define RESCALE(x) (std::max(std::min(x, 1.0f), -1.0f) + 1.0f) / 2.0f

/*
 *  XZ are block coordinates. You must convert from local chunk space to world space.
 *  Returns a height value from `0.0` to `1.0`.
*/
float height_at_pos(float x, float z) {
    FastNoiseLite noise(0);
	noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    float mountain_height = RESCALE(noise.GetNoise(x, z)) / 2.0f;

    noise.mSeed += 1;
    float flat_height = RESCALE(noise.GetNoise(x / 5.0f, z / 5.0f)) / 4.0f + .5f;

    noise.mSeed += 1;

    float filter = std::max(RESCALE(noise.GetNoise(x / 20.0f, z / 20.0f)*4.0f) - 0.2f, 0.0f);

    float height = mountain_height * filter + flat_height * (1.0f - filter);

    /*
    #define height_function(topper) \
					(noise.GetNoise((float)(x * CHUNK_SIZE + bx), (float)(z * CHUNK_SIZE + bz)) + 1.0f) / 2.0f / 2.0f \
					/ (noise.GetNoise((float)(x * 4.0f + bx), (float)(z  * 4.0f + bz)) + 1.0f) / 2.0f / 2.0f /2.0f \ 
					> (float)(y * CHUNK_SIZE + by topper) / WORD_HEIGHT / CHUNK_SIZE
    */
    return height;
}

// Returns true if air
BlockType cave_gen(float x, float y, float z) {
    FastNoiseLite noise(0);
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    float scale = 10.0f;

    BlockType block = BlockType::AIR;

    bool is_air = noise.GetNoise(x * scale, y * scale, z * scale) > 0.5f;

    if(!is_air) {
        noise.mSeed += 1;
        block = BlockType::STONE;
        bool is_ore = noise.GetNoise(x * scale, y * scale, z * scale) > 0.7f;
        if(is_ore) block = BlockType::IRON_ORE;
    }

    return block;
}