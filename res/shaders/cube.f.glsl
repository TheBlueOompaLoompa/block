#version 330 core
out vec4 FragColor;

in vec3 f_normal;
varying vec2 f_texcoord;
uniform sampler2D mytexture;

vec3 light = vec3(.5, 1, .8);

void main(void) {
    vec2 flipped = vec2(f_texcoord.y, 1.0-f_texcoord.x);
    float b = min(max(0.3, dot(normalize(light), f_normal) * 2.0), 1.0);

    FragColor = texture2D(mytexture, flipped) * vec4(b, b, b, 1.0);
}