#pragma once

#include <chunk.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "helper.hpp"
#include "block.hpp"

bool safe_air_check(V3VECARRAY(Chunk)* chunks_ref, glm::vec3 ray_pos) {
    int cx = (int)floor(ray_pos.x/16.0f);
    int cy = (int)floor(ray_pos.y/16.0f);
    int cz = (int)floor(ray_pos.z/16.0f);

    V3VECARRAY(Chunk)& chunks = *chunks_ref;

    if(
        !(cx >= 0 && cy >= 0 && cz >= 0 &&
        cx < chunks.size() && cy < chunks[cx].size() && cz < chunks[cx][cy].size())
    ) {
        return true;
    }

    return chunks[cx][cy][cz]
            .blocks[((int)floor(ray_pos.x))%16][((int)floor(ray_pos.y))%16][((int)floor(ray_pos.z))%16].type == BlockType::AIR;
}

// Returns true when hit
bool raycast(V3VECARRAY(Chunk)* chunks_ref, glm::vec3* start_pos, glm::quat rot, glm::vec3* hit_pos, float max_dist = 6.0f, float step = .05) {
    glm::vec3 ray_pos = (*start_pos);
    
    while(
        safe_air_check(chunks_ref, ray_pos) &&
        V3DIST((*start_pos), ray_pos) <= max_dist
    ) {
        ray_pos -= glm::rotate(glm::inverse(rot), V3FORWARD) * glm::vec3(step);
    }

    bool hit = V3DIST((*start_pos), ray_pos) < max_dist;

    if(hit && hit_pos != nullptr) *hit_pos = ray_pos;

    return hit;
}