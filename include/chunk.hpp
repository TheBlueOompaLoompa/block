#pragma once
#include <cstdio>
#include <stdint.h>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <sys/stat.h>

#include "save.hpp"
#include "util.hpp"
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


    void generate_chunk(World* world);
    
    // TODO: Hippo optimize mesh
    // Hungry hungry hippos baybee
    /*void hippo() {

    }*/

    void update_area(Chunk* (&chunks)[LOAD_DISTANCE][WORLD_HEIGHT][LOAD_DISTANCE]);
    void update_mesh();

    // Returns true if opaque block is at pos
    bool chk_block(int ix, int iy, int iz);

    void generate_side(BlockType block_type, int sx, int sy, int sz, glm::vec3 up, glm::vec3 right, bool chkFlip, bool flip = false);

    void save(const char* root);
    void load(const char* root);

    void destroy();
};

void move_chunks(World* world, Chunk* (&chunks)[LOAD_DISTANCE][WORLD_HEIGHT][LOAD_DISTANCE], bool xz, bool flip);

