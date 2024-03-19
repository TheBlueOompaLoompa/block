#version 330 core

layout (location = 0) in vec3 coord3d;
layout (location = 1) uniform mat4 mvp;

//out float instance;

void main(void) {
    gl_Position = mvp * vec4(coord3d, 1.0);
    //instance = gl_InstanceID;
    f_texcoord = texcoord;
}
