#pragma once
#include <GL/glew.h>

GLuint vbo_ui_quad;

/*void setup_generation() {
    glGenVertexArrays(1, &vbo_ui_quad);
    glBindVertexArray(vbo_ui_quad);
    static const GLfloat g_quad_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
    };

    GLuint quad_vertexbuffer;
    glGenBuffers(1, &quad_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
}*/

GLuint gen_height_map(GLuint* height_program, int chunk_size, int x, int y, int width, int height) {
    GLuint fbuf;
    glGenFramebuffers(1, &fbuf);
    glBindFramebuffer(GL_FRAMEBUFFER, fbuf);

    GLuint render_texture;
    glGenTextures(1, &render_texture);

    glBindTexture(GL_TEXTURE_2D, render_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, chunk_size * width, chunk_size * height, 0, GL_R8, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, render_texture, 0);

    GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};

    if(*height_program == 0) return 0;

    glUseProgram(*height_program);
    

    glDrawBuffers(1, draw_buffers);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { return false; }

    glDeleteFramebuffers(1, &fbuf);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    printf("gen");
    return render_texture;
}