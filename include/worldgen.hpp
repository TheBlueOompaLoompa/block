#pragma once
#include "FastNoiseLite.h"

/*
 *  XZ are block coordinates. You must convert from local chunk space to world space.
 *  Returns a height value from `0.0` to `1.0`.
*/
float height_at_pos(float x, float z) {
    FastNoiseLite noise(0);
	noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    #define RESCALE(x) (std::max(std::min(x, 1.0f), -1.0f) + 1.0f) / 2.0f

    float mountain_height = RESCALE(noise.GetNoise(x, z)) / 2.0f;

    noise.mSeed += 1;
    float flat_height = RESCALE(noise.GetNoise((float)x / 5.0f, (float)z / 5.0f)) / 4.0f + .5f;

    noise.mSeed += 1;

    float filter = std::max(RESCALE(noise.GetNoise((float)x / 20.0f, (float)z / 20.0f)*4.0f) - 0.2f, 0.0f);

    float height = mountain_height * filter + flat_height * (1.0f - filter);

    /*
    #define height_function(topper) \
					(noise.GetNoise((float)(x * CHUNK_SIZE + bx), (float)(z * CHUNK_SIZE + bz)) + 1.0f) / 2.0f / 2.0f \
					/ (noise.GetNoise((float)(x * 4.0f + bx), (float)(z  * 4.0f + bz)) + 1.0f) / 2.0f / 2.0f /2.0f \ 
					> (float)(y * CHUNK_SIZE + by topper) / WORD_HEIGHT / CHUNK_SIZE
    */
    return height;
}