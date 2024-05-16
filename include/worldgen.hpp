#pragma once
#include "FastNoiseLite.h"
#include "block.hpp"
#include "config.hpp"

#define RESCALE(x) (std::max(std::min(x, 1.0f), -1.0f) + 1.0f) / 2.0f

struct WorldGeneration {
    int seed;

    /*
    *  XZ are block coordinates. You must convert from local chunk space to world space.
    *  Returns a height value from `0.0` to `1.0`.
    */
    float height_at_pos(float x, float z) {
        FastNoiseLite noise(seed);
        noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

        float mountain_height = RESCALE(noise.GetNoise(x, z)) / 2.0f;

        noise.mSeed += 1;
        float flat_height = RESCALE(noise.GetNoise(x / 5.0f, z / 5.0f)) / 4.0f + .5f;

        noise.mSeed += 1;

        float filter = std::max(RESCALE(noise.GetNoise(x / 20.0f, z / 20.0f)*4.0f) - 0.2f, 0.0f);

        float height = mountain_height * filter + flat_height * (1.0f - filter);
        
        return height;
    }

    // Returns true if air
    BlockType cave_gen(float x, float y, float z) {
        FastNoiseLite noise(seed);
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

    BlockType generate_block(float cx, float cy, float cz, float bx, float by, float bz) {
        #define MAKE_WORLD(cx, bx) cx * CHUNK_SIZE + bx
        #define HEIGHT_OFFSET(x, offset) x + (offset)/(float)(CHUNK_SIZE * WORLD_HEIGHT)

        float x = MAKE_WORLD(cx, bx);
        float y = MAKE_WORLD(cy, by);
        float z = MAKE_WORLD(cz, bz);

        float height = height_at_pos(x, z);

        float relative_y = (float)(cy * CHUNK_SIZE + by) / (float)(CHUNK_SIZE * WORLD_HEIGHT);
        BlockType new_type = BlockType::AIR;

        if(HEIGHT_OFFSET(height, -10.0) > relative_y) {
            new_type = cave_gen(x, y, z);
        }else if(HEIGHT_OFFSET(height, -3.0) > relative_y) {
            new_type = BlockType::STONE;
        }else if(height > relative_y) {
            new_type = BlockType::DIRT;
        } else if(HEIGHT_OFFSET(height, 1.0f) > relative_y) {
            new_type = BlockType::GRASS;
        }

        return new_type;
    }
};

