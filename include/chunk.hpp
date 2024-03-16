#pragma once
#include <stdint.h>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "block.hpp"
#include "geometry.hpp"
#include "helper.hpp"

#define CHUNK_SIZE 16

struct Chunk
{
    int x = 0;
    int y = 0;
    int z = 0;

    Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; // XYZ

    GLuint vbo_vertices;
    GLuint vbo_elements;
    GLuint vbo_texcoords;

    std::vector<Vertex> vertices;

    Chunk* adjacent_chunks[6]; // up down left right forward back

    void updateMesh()
    {
        for(int my = 0; my < CHUNK_SIZE; my++) {
            for(int mz = 0; my < CHUNK_SIZE; my++) {
                for(int mx = 0; my < CHUNK_SIZE; my++) {
                    if(blocks[mx][my][mz].type == BlockType::AIR) continue;

                    generate_side(mx + 1, my, mz, V3FORWARD, V3UP, true);
                    generate_side(mx, my, mz,     V3FORWARD, V3UP);
                    generate_side(mx, my + 1, mz, V3);
                    generate_side(mx, my, mz,     );
                    generate_side(mx, my, mz + 1, );
                    generate_side(mx, my, mz,     );
                }
            }
        }
    }

    // Returns true if opaque block is at pos
    bool chk_block(int ix, int iy, int iz) {
        if(ix > CHUNK_SIZE - 1)
        { // Left adjacent chunk
            if(adjacent_chunks[2] == nullptr) return false;
            else return adjacent_chunks[2]->blocks[ix - CHUNK_SIZE][iy][iz].type != BlockType::AIR;
        }
        else if(ix < 0)
        { // Right
            if(adjacent_chunks[3] == nullptr) return false;
            else return adjacent_chunks[3]->blocks[ix + CHUNK_SIZE][iy][iz].type != BlockType::AIR;
        }
        else if(iy > CHUNK_SIZE - 1)
        { // Above
            if(adjacent_chunks[0] == nullptr) return false;
            else return adjacent_chunks[0]->blocks[ix][iy - CHUNK_SIZE][iz].type != BlockType::AIR;
        }
        else if(iy < 0)
        { // Below
            if(adjacent_chunks[1] == nullptr) return false;
            else return adjacent_chunks[1]->blocks[ix][iy + CHUNK_SIZE][iz].type != BlockType::AIR;
        }
        else if(iz > CHUNK_SIZE - 1)
        { // Front
            if(adjacent_chunks[4] == nullptr) return false;
            else return adjacent_chunks[4]->blocks[ix][iy][iz - CHUNK_SIZE].type != BlockType::AIR;
        }
        else if(iz < 0)
        { // Back
            if(adjacent_chunks[5] == nullptr) return false;
            else return adjacent_chunks[5]->blocks[ix][iy][iz + CHUNK_SIZE].type != BlockType::AIR;
        }
        else return blocks[ix][iy][iz].type != BlockType::AIR;
    }

    void generate_side(int sx, int sy, int sz, glm::vec3 up, glm::vec3 right, bool flip = false)
    {
        if(chk_block(sx, sy, sz)) return;

        vertices.push_back({ (float)sx, (float)sy, (float)sz, 0.0f, 0.0f });
        vertices.push_back({ (float)sx + right.x, (float)sy + right.y, (float)sz + right.z, 1.0f, 0.0f });
        vertices.push_back({ (float)sx + right.x + up.x, (float)sy + right.y + up.y, (float)sz + right.z + up.z, 1.0f, 1.0f });
        vertices.push_back({ (float)sx + up.x, (float)sy + up.y, (float)sz + up.z, 0.0f, 1.0f });
    }
};

Chunk a;
