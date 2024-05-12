#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#include "../geometry.hpp"

struct Mesh {
    GLuint vbo_vertices;
    GLuint vbo_normals;
    GLuint ibo_elements;

    std::vector<Vertex> vertices;
    std::vector<glm::vec3> normals;
    std::vector<GLushort> indices;

    int load_obj(const char* path);

    void render() {
        
    }

    void new_gl_buffers() {
        clear_gl();

        glGenBuffers(1, &vbo_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &vbo_normals);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &ibo_elements);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

        clear_buffers();
    }

    void clear_gl() {
        glDeleteBuffers(1, &vbo_vertices);
        glDeleteBuffers(1, &vbo_normals);
        glDeleteBuffers(1, &ibo_elements);
    }

    void clear_buffers() {
        vertices.clear();
        normals.clear();
        indices.clear();
    }

    void clear() {
        clear_buffers();
        clear_gl();
    }
};