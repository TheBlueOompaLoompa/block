#pragma once
#include <stdint.h>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <sys/stat.h>

#include "worldgen.hpp"
#include "block.hpp"
#include "geometry.hpp"
#include "helper.hpp"
#include "mesh/mesh.hpp"

struct ChunkSaveData {
    Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

struct Chunk {
    glm::ivec3 position;

    Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; // XYZ

    bool changed = false;

    Mesh mesh;

    Chunk* adjacent_chunks[6]; // up down left right forward back


    void generate_chunk() {
        destroy();

        for(int bx = 0; bx < CHUNK_SIZE; bx++) {
            for(int bz = 0; bz < CHUNK_SIZE; bz++) {
                for(int by = 0; by < CHUNK_SIZE; by++) {
                    blocks[bx][by][bz].type =
                        generate_block(position.x, position.y, position.z, bx, by, bz);
                }
            }
        }
    }
    
    // TODO: Hippo optimize mesh
    // Hungry hungry hippos baybee
    void hippo() {

    }

    void update_area(Chunk* (&chunks)[LOAD_DISTANCE][WORLD_HEIGHT][LOAD_DISTANCE]) {
        update_mesh();
        if(position.x > 0) chunks[position.x - 1][position.y][position.z]->update_mesh();
        if(position.y > 0) chunks[position.x][position.y - 1][position.z]->update_mesh();
        if(position.z > 0) chunks[position.x][position.y][position.z - 1]->update_mesh();
        if(position.x < LOAD_DISTANCE - 1)   chunks[position.x + 1][position.y][position.z]->update_mesh();
        if(position.y < WORLD_HEIGHT - 1)    chunks[position.x][position.y + 1][position.z]->update_mesh();
        if(position.z < LOAD_DISTANCE - 1)   chunks[position.x][position.y][position.z + 1]->update_mesh();
    }

    void update_mesh() {
        mesh.clear();
        for(int my = 0; my < CHUNK_SIZE; my++) {
            for(int mz = 0; mz < CHUNK_SIZE; mz++) {
                for(int mx = 0; mx < CHUNK_SIZE; mx++) {
                    if(blocks[mx][my][mz].type == BlockType::AIR) continue;
                    
                    generate_side(blocks[mx][my][mz].type, mx, my + 1, mz, V3FORWARD, V3RIGHT,true        );  // TOP
                    generate_side(blocks[mx][my][mz].type, mx, my, mz,     V3FORWARD, V3RIGHT,false, true );  // BOTTOM
                    
                    generate_side(blocks[mx][my][mz].type, mx + 1, my, mz, V3FORWARD, V3UP,   true,  true );  // RIGHT
                    generate_side(blocks[mx][my][mz].type, mx, my, mz,     V3FORWARD, V3UP,   false, false); // LEFT

                    generate_side(blocks[mx][my][mz].type, mx, my, mz + 1, V3RIGHT,   V3UP,   true,  false); // FRONT
                    generate_side(blocks[mx][my][mz].type, mx, my, mz,     V3RIGHT,   V3UP,   false, true );  // BACK
                }
            }
        }

        for(uint i = 0; i < mesh.vertices.size(); i++) {
            mesh.vertices[i].pos += glm::vec3(position.x, position.y, position.z) * glm::vec3(CHUNK_SIZE);
        }

        mesh.new_gl_buffers();
    }

    // Returns true if opaque block is at pos
    bool chk_block(int ix, int iy, int iz) {
        if(ix > CHUNK_SIZE - 1) {
            // Left adjacent chunk
            if(adjacent_chunks[2] == nullptr) return false;
            else return adjacent_chunks[2]->chk_block(ix - CHUNK_SIZE, iy, iz);
        }
        else if(ix < 0) {
            // Right
            if(adjacent_chunks[3] == nullptr) return false;
            else return adjacent_chunks[3]->chk_block(ix + CHUNK_SIZE, iy, iz);
        }
        else if(iy > CHUNK_SIZE - 1) {
            // Above
            if(adjacent_chunks[0] == nullptr) return false;
            else return adjacent_chunks[0]->chk_block(ix, iy - CHUNK_SIZE, iz);
        }
        else if(iy < 0) {
            // Below
            if(adjacent_chunks[1] == nullptr) return false;
            else return adjacent_chunks[1]->chk_block(ix, iy + CHUNK_SIZE, iz);
        }
        else if(iz > CHUNK_SIZE - 1) {
            // Front
            if(adjacent_chunks[4] == nullptr) return false;
            else return adjacent_chunks[4]->chk_block(ix, iy, iz - CHUNK_SIZE);
        }
        else if(iz < 0) {
            // Back
            if(adjacent_chunks[5] == nullptr) return false;
            else return adjacent_chunks[5]->chk_block(ix, iy, iz + CHUNK_SIZE);
        }
        else return blocks[ix][iy][iz].type != BlockType::AIR;
    }

    void generate_side(BlockType block_type, int sx, int sy, int sz, glm::vec3 up, glm::vec3 right, bool chkFlip, bool flip = false) {
        glm::vec3 normal = glm::cross(up, right);
        glm::vec3 dir = glm::cross(up, right) * glm::vec3(chkFlip ? 0 : (flip ? -1 : 1));
        if(chk_block(dir.x + sx, dir.y + sy, dir.z + sz)) return;
        // TODO: Find a way to store the surface normal only once

        normal *= !chkFlip ? -1 : 1;

        glm::vec3 surface_normal = normal;
        mesh.normals.push_back(surface_normal);
        mesh.normals.push_back(surface_normal);
        mesh.normals.push_back(surface_normal);
        mesh.normals.push_back(surface_normal);

        bool flip_y = false;
        bool flip_x = false;

        if(
            glm::dot(normal, V3UP) < 0 ||
            glm::dot(normal, V3FORWARD) < 0 ||
            glm::dot(normal, V3RIGHT) < 0
        ) { flip_x = true; }

        // Subtract one because of air
        float atlas_offset = (float)block_type - 1;

        mesh.vertices.push_back({
            pos: glm::vec3((float)sx, (float)sy, (float)sz),
            uv: glm::vec2((flip_y ? 1.0f : 0.0f) / ATLAS_ROWS + (ATLAS_ROWS - 1.0) / ATLAS_ROWS, ((flip_x ? 1.0f : 0.0f) + atlas_offset) / ATLAS_COLS)
        });
        mesh.vertices.push_back({
            pos: glm::vec3((float)(sx + right.x), (float)(sy + right.y), (float)(sz + right.z)),
            uv: glm::vec2((flip_y ? 0.0f : 1.0f) / ATLAS_ROWS + (ATLAS_ROWS - 1.0) / ATLAS_ROWS, ((flip_x ? 1.0f : 0.0f) + atlas_offset) / ATLAS_COLS)
        });
        mesh.vertices.push_back({
            pos: glm::vec3((float)(sx + right.x + up.x), (float)(sy + right.y + up.y), (float)(sz + right.z + up.z)),
            uv: glm::vec2((flip_y ? 0.0f : 1.0f) / ATLAS_ROWS + (ATLAS_ROWS - 1.0) / ATLAS_ROWS, ((flip_x ? 0.0f : 1.0f) + atlas_offset) / ATLAS_COLS)
        });
        mesh.vertices.push_back({
            pos: glm::vec3((float)(sx + up.x), (float)(sy + up.y), (float)(sz + up.z)),
            uv: glm::vec2((flip_y ? 1.0f : 0.0f) / ATLAS_ROWS + (ATLAS_ROWS - 1.0) / ATLAS_ROWS, ((flip_x ? 0.0f : 1.0f) + atlas_offset) / ATLAS_COLS)
        });

        if(flip) {
            mesh.indices.push_back(mesh.vertices.size() - 4);
            mesh.indices.push_back(mesh.vertices.size() - 3);
            mesh.indices.push_back(mesh.vertices.size() - 2);

            mesh.indices.push_back(mesh.vertices.size() - 4);
            mesh.indices.push_back(mesh.vertices.size() - 2);
            mesh.indices.push_back(mesh.vertices.size() - 1);
        } else {
            mesh.indices.push_back(mesh.vertices.size() - 2);
            mesh.indices.push_back(mesh.vertices.size() - 3);
            mesh.indices.push_back(mesh.vertices.size() - 4);

            mesh.indices.push_back(mesh.vertices.size() - 1);
            mesh.indices.push_back(mesh.vertices.size() - 2);
            mesh.indices.push_back(mesh.vertices.size() - 4);
        }
    }

    void save(const char* root) {
        char* fname = (char*)malloc(100);

        sprintf(fname, "%s/chunks", root);
        chk_mkdir(fname);
        sprintf(fname, "%s/chunks/%i %i %i.chunk", root, position.x, position.y, position.z);

        ChunkSaveData data;
        memcpy(&data.blocks, &blocks, sizeof(data.blocks));

        save_data(fname, &data, sizeof(ChunkSaveData));
        free(fname);
    }

    void load(const char* root) {
        char* fname = (char*)malloc(100);
        sprintf(fname, "%s/chunks/%i %i %i.chunk", root, position.x, position.y, position.z);

        if(!std::filesystem::exists(fname)) {
            free(fname);
            return;
        }

        ChunkSaveData chunk_data;
        load_data(fname, &chunk_data, sizeof(ChunkSaveData));
        memcpy(&blocks, &chunk_data.blocks, sizeof(chunk_data.blocks));

        changed = true;

        free(fname);
    }

    void destroy() {
        glDeleteBuffers(1, &mesh.vbo_vertices);
        glDeleteBuffers(1, &mesh.vbo_normals);
        glDeleteBuffers(1, &mesh.ibo_elements);
        mesh.vertices.clear();
        mesh.normals.clear();
        mesh.indices.clear();
    }
};

void move_chunks(Chunk* (&chunks)[LOAD_DISTANCE][WORLD_HEIGHT][LOAD_DISTANCE], bool xz, bool flip) {
    Chunk* wall_chunks[LOAD_DISTANCE][WORLD_HEIGHT];

    // TODO: Implement x-axis
    if(!xz) { // X axis
        
    }else { // Z axis
        // Save
        memcpy(wall_chunks, chunks[flip ? LOAD_DISTANCE - 1 : 0], sizeof(int*) * LOAD_DISTANCE * WORLD_HEIGHT);

        /*if(flip) {
            memmove(chunks[1], wall_chunks[0], sizeof(int*) * LOAD_DISTANCE * WORLD_HEIGHT * (LOAD_DISTANCE - 1));
        }else {
            memmove(chunks[0], &wall_chunks[1], sizeof(int*) * LOAD_DISTANCE * WORLD_HEIGHT * (LOAD_DISTANCE - 1));
        }*/

        // Load
        memcpy(wall_chunks, chunks[!flip ? LOAD_DISTANCE - 1 : 0], sizeof(int*) * LOAD_DISTANCE * WORLD_HEIGHT);
        for(int x = 0; x < LOAD_DISTANCE; x++) {
            for(int y = 0; y < WORLD_HEIGHT; y++) {
                wall_chunks[x][y]->position = glm::ivec3(x, y, 0.0f);
                wall_chunks[x][y]->generate_chunk();
            }
        }
    }
}