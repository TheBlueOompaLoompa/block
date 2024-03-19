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
    GLuint ibo_elements;
    GLuint vbo_texcoords;

    std::vector<Vertex> vertices;
    std::vector<GLushort> indices;

    Chunk* adjacent_chunks[6]; // up down left right forward back

    void updateMesh()
    {
        for(int my = 0; my < CHUNK_SIZE; my++) {
            for(int mz = 0; mz < CHUNK_SIZE; mz++) {
                for(int mx = 0; mx < CHUNK_SIZE; mx++) {
                    if(blocks[mx][my][mz].type == BlockType::AIR) continue;
                    printf("X: %i Y: %i Z: %i\n", mx, my, mz);

                    generate_side(mx + 1, my, mz, V3FORWARD, V3UP,   true);
                    generate_side(mx, my, mz,     V3FORWARD, V3UP,   true);
                    generate_side(mx, my + 1, mz, V3FORWARD, V3RIGHT     );
                    generate_side(mx, my, mz,     V3FORWARD, V3RIGHT     );
                    generate_side(mx, my, mz + 1, V3RIGHT,   V3UP        );
                    generate_side(mx, my, mz,     V3RIGHT,   V3UP,   true);
                }
            }
        }

        glDeleteBuffers(1, &vbo_vertices);
        glGenBuffers(1, &vbo_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glDeleteBuffers(1, &ibo_elements);
        glGenBuffers(1, &ibo_elements);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);
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

        // Front
        vertices.push_back({
            x: (float)sx,
            y: (float)sy,
            z: (float)sz,
            0.0f, 0.0f });
        vertices.push_back({
            x: (float)(sx + right.x),
            y: (float)(sy + right.y),
            z: (float)(sz + right.z),
            1.0f, 0.0f });
        vertices.push_back({
            x: (float)(sx + right.x + up.x),
            y: (float)(sy + right.y + up.y),
            z: (float)(sz + right.z + up.z),
            1.0f, 1.0f });
        vertices.push_back({
            x: (float)(sx + up.x),
            y: (float)(sy + up.y),
            z: (float)(sz + up.z),
            0.0f, 1.0f });

        if(flip)
        {
            indices.push_back(vertices.size() - 4);
            indices.push_back(vertices.size() - 3);
            indices.push_back(vertices.size() - 1);

            indices.push_back(vertices.size() - 1);
            
            indices.push_back(vertices.size() - 3);
            indices.push_back(vertices.size() - 2);
        }
        else
        {
            indices.push_back(vertices.size() - 1);
            indices.push_back(vertices.size() - 3);
            indices.push_back(vertices.size() - 4);

            indices.push_back(vertices.size() - 1);
            indices.push_back(vertices.size() - 2);
            indices.push_back(vertices.size() - 3);
        }
        

        /*for(int i = 0; i < 6; i++) {
            printf("%i ", indices[indices.size() - 6 + i]);
        }*/

        /*for(int i = 0; i < 4; i++) {
            printf("%f %f %f\n", vertices[vertices.size() - 4 + i].x, vertices[vertices.size() - 4 + i].y, vertices[vertices.size() - 4 + i].z);
        }

        printf("Vertices: %li Indices: %li\n XYZ: %i %i %i\n", vertices.size(), indices.size(), sx, sy, sz);*/
    }

    void destroy() {
        glDeleteBuffers(1, &vbo_vertices);
        glDeleteBuffers(1, &ibo_elements);
    }
};

Chunk a;
