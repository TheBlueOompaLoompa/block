#pragma once
#include <GL/glew.h>

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

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;

    glDeleteFramebuffers(1, &fbuf);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return 0;
}