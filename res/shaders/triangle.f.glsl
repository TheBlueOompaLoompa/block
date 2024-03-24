#version 140

in vec2 f_pos;
out vec4 color;

void main() {
    color = vec4((f_pos.x+1.0) / 2.0, (f_pos.y+1.0) / 2.0, 0.0, 1.0);
}