#pragma once
#include <stdint.h>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <reactphysics3d/reactphysics3d.h>

#include "block.hpp"
#include "geometry.hpp"
#include "helper.hpp"

#define CHUNK_SIZE 16

struct Chunk {
    int x = 0;
    int y = 0;
    int z = 0;

    Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; // XYZ

    GLuint vbo_vertices;
    GLuint nbo_normals;
    GLuint ibo_elements;

    std::vector<Vertex> vertices;
    std::vector<glm::vec3> normals;
    std::vector<GLushort> indices;

    Chunk* adjacent_chunks[6]; // up down left right forward back

    // TODO: Hippo optimize mesh
    // Hungry hungry hippos baybee
    void hippo() {

    }

    void updateMesh(rp3d::PhysicsCommon* physicsCommon, rp3d::PhysicsWorld* world) {
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

        for(uint i = 0; i < vertices.size(); i++) {
            vertices[i].pos += glm::vec3(x, y, z) * glm::vec3(CHUNK_SIZE);
        }

        glDeleteBuffers(1, &vbo_vertices);
        glGenBuffers(1, &vbo_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glDeleteBuffers(1, &nbo_normals);
        glGenBuffers(1, &nbo_normals);
        glBindBuffer(GL_ARRAY_BUFFER, nbo_normals);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);

        glDeleteBuffers(1, &ibo_elements);
        glGenBuffers(1, &ibo_elements);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);
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
        normals.push_back(surface_normal);
        normals.push_back(surface_normal);
        normals.push_back(surface_normal);
        normals.push_back(surface_normal);

        bool flip_y = false;
        bool flip_x = false;

        if(
            glm::dot(normal, V3UP) < 0 ||
            glm::dot(normal, V3FORWARD) < 0 ||
            glm::dot(normal, V3RIGHT) < 0
        ) { flip_x = true; }

        #define ATLAS_TEX_COUNT 2.0f
        // Subtract one because of air
        float atlas_offset = (float)block_type - 1;

        vertices.push_back({
            pos: glm::vec3((float)sx, (float)sy, (float)sz),
            uv: glm::vec2((flip_y ? 1.0f : 0.0f), ((flip_x ? 1.0f : 0.0f) + atlas_offset) / ATLAS_TEX_COUNT)
        });
        vertices.push_back({
            pos: glm::vec3((float)(sx + right.x), (float)(sy + right.y), (float)(sz + right.z)),
            uv: glm::vec2((flip_y ? 0.0f : 1.0f), ((flip_x ? 1.0f : 0.0f) + atlas_offset) / ATLAS_TEX_COUNT)
        });
        vertices.push_back({
            pos: glm::vec3((float)(sx + right.x + up.x), (float)(sy + right.y + up.y), (float)(sz + right.z + up.z)),
            uv: glm::vec2((flip_y ? 0.0f : 1.0f), ((flip_x ? 0.0f : 1.0f) + atlas_offset) / ATLAS_TEX_COUNT)
        });
        vertices.push_back({
            pos: glm::vec3((float)(sx + up.x), (float)(sy + up.y), (float)(sz + up.z)),
            uv: glm::vec2((flip_y ? 1.0f : 0.0f), ((flip_x ? 0.0f : 1.0f) + atlas_offset) / ATLAS_TEX_COUNT)
        });

        if(flip) {
            indices.push_back(vertices.size() - 4);
            indices.push_back(vertices.size() - 3);
            indices.push_back(vertices.size() - 2);

            indices.push_back(vertices.size() - 4);
            indices.push_back(vertices.size() - 2);
            indices.push_back(vertices.size() - 1);
        } else {
            indices.push_back(vertices.size() - 2);
            indices.push_back(vertices.size() - 3);
            indices.push_back(vertices.size() - 4);

            indices.push_back(vertices.size() - 1);
            indices.push_back(vertices.size() - 2);
            indices.push_back(vertices.size() - 4);
        }
    }

    void destroy() {
        glDeleteBuffers(1, &vbo_vertices);
        glDeleteBuffers(1, &ibo_elements);
    }
};