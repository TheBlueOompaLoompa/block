#version 330 core

attribute vec3 coord3d;
attribute vec2 texcoord;
attribute vec3 normal;
uniform mat4 mvp;

out vec2 f_texcoord;
out vec3 f_normal;

void main(void) {
    if(texcoord.x > -1) {
        gl_Position = mvp * vec4(coord3d, 1.0);

        
        f_texcoord = texcoord;
        f_normal = normal;
    }
}