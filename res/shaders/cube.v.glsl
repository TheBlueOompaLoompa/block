#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout (location = 0) in vec3 coord3d;
layout (location = 1) uniform mat4 mvp;
//attribute vec2 texcoord;
varying vec4 color;
//varying vec2 f_texcoord;


out float instance;

void main(void) {
    gl_Position = mvp * vec4(coord3d, 1.0);
    instance = gl_InstanceID;
    color = vec4((coord3d + 16.0) / 32.0, 1.0);
    //f_texcoord = texcoord;
}
