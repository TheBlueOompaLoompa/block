#version 140

in vec3 position;
out vec3 f_pos;

void main() {
    gl_Position = vec4(position, 1.0);
    f_pos = position;
}