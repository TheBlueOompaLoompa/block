
#version 330 core

attribute vec3 coord3d;
attribute vec2 texcoord;
varying vec2 f_texcoord;
uniform mat4 mvp;

out float instance;

void main(void) {
    gl_Position = mvp * vec4(coord3d, 1.0);
    gl_Position.x += -5 + gl_InstanceID * 2;
    gl_Position.y += gl_InstanceID * 4 - 13;
    instance = gl_InstanceID;
    f_texcoord = texcoord;
}
