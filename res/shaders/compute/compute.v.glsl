#version 330 core

attribute vec3 coord3d;

out vec2 UV;

void main(void) {
    gl_Position = vec4(coord3d, 1.0);
    UV = vec2((coord3d.x + 1.0) / 2.0, (coord3d.y + 1.0) / 2.0);
}