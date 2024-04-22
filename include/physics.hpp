#pragma once

#include <chunk.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "helper.hpp"
#include "block.hpp"
#include "config.hpp"

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

	int bx = ((int)floor(ray_pos.x))%16;
	int by = ((int)floor(ray_pos.y))%16;
	int bz = ((int)floor(ray_pos.z))%16;

    return chunks[cx][cy][cz]
            .blocks[bx][by][bz].type == BlockType::AIR;
}

// Returns true when hit
bool raycast(V3VECARRAY(Chunk)* chunks_ref, glm::vec3* start_pos, glm::quat rot, glm::vec3* hit_pos = nullptr, /*glm::vec3* hit_block = nullptr,*/ float max_dist = 6.0f, float step = .05) {
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

bool raycast(V3VECARRAY(Chunk)* chunks_ref, glm::vec3* start_pos, glm::vec3 dir, glm::vec3* hit_pos = nullptr, float max_dist = 6.0f, float step = .05) {
    glm::vec3 ray_pos = (*start_pos);
    
    while(
        safe_air_check(chunks_ref, ray_pos) &&
        V3DIST((*start_pos), ray_pos) <= max_dist
    ) {
        ray_pos -= dir * glm::vec3(step);
    }

    bool hit = V3DIST((*start_pos), ray_pos) < max_dist;

    if(hit && hit_pos != nullptr) *hit_pos = ray_pos;

    return hit;
}

void process_entity_physics(Entity& entity, V3VECARRAY(Chunk)* chunks, float dt) {
	entity.is_grounded = false;
	
	#define radius .18f
	#define ground_radius \
		(radius-.1f)
	for(int x = 0; x < 2; x++) {
		for(int y = 0; y < 2; y++) {
			glm::vec3 ray_start = entity.position + glm::vec3((float)x*(ground_radius*2.0f) - ground_radius, 0.0f, (float)y*(ground_radius*2.0f) - ground_radius);
			if(raycast(chunks, &ray_start, glm::quat(glm::vec3(-M_PIf, 0.0f, 0.0f)), nullptr, .01f, .001f)) {
				entity.is_grounded = true;
			}

			ray_start.y += PLAYER_HEIGHT;
			bool hit_top = raycast(chunks, &ray_start, glm::quat(glm::vec3(M_PIf, 0.0f, 0.0f)), nullptr, .01f, .001f);
			if(hit_top) { entity.velocity.y = std::min(entity.velocity.y, 0.0f); }
		}		
	}

	// Gravity
	entity.velocity.y = std::max(entity.velocity.y - (GRAVITY*dt), -GRAVITY);
	if(entity.is_grounded) { entity.velocity.y = std::max(entity.velocity.y, 0.0f); }

	// Side collision check
	glm::vec3 check_pos = entity.position;
	check_pos.y += 0.1f;

	for(float i = 0; i < PLAYER_HEIGHT; i++) {
		bool hit_pos_z = raycast(chunks, &check_pos, glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)), nullptr, radius, .001f);
		bool hit_pos_x = raycast(chunks, &check_pos, glm::quat(glm::vec3(0.0f, M_PIf/2.0f, 0.0f)), nullptr, radius, .001f);
		bool hit_neg_z = raycast(chunks, &check_pos, glm::quat(glm::vec3(0.0f, M_PIf, 0.0f)), nullptr, radius, .001f);
		bool hit_neg_x = raycast(chunks, &check_pos, glm::quat(glm::vec3(0.0f, -M_PIf/2.0f, 0.0f)), nullptr, radius, .001f);

		if(hit_pos_x) entity.velocity.x = std::min(entity.velocity.x, 0.0f);
		if(hit_pos_z) entity.velocity.z = std::max(entity.velocity.z, 0.0f);
		if(hit_neg_x) entity.velocity.x = std::max(entity.velocity.x, 0.0f);
		if(hit_neg_z) entity.velocity.z = std::min(entity.velocity.z, 0.0f);

		// TODO: Implement better algorithm along with new raycasting system in order to eject entities inside blocks
		// Eject player from ground or ceiling
		if(hit_pos_x + hit_pos_z + hit_neg_x + hit_pos_z > 3) entity.position.y = i > 0 ? std::floor(entity.position.y) : std::ceil(entity.position.y);

		check_pos.y += i == PLAYER_HEIGHT - 2.0f ? 1.5f : 1.0f;
	}

	// Apply velocity to position
	entity.position += V3FORWARD * entity.velocity.z * dt;
	entity.position += V3UP * entity.velocity.y * dt;
	entity.position += V3RIGHT * entity.velocity.x * dt;
}