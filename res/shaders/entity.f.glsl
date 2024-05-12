#version 330 core
out vec4 FragColor;

#include "shade.glsl"

in vec3 f_normal;
varying vec2 f_texcoord;
uniform sampler2D mytexture;
uniform float time;

void main(void) {
    vec2 flipped = vec2(f_texcoord.y, 1.0-f_texcoord.x);

    FragColor = texture2D(mytexture, flipped);

    // Light color/angle brightness
    FragColor *= apply_light(f_normal);

    // Time of day brightness
    FragColor *= max(cos(time)/2.0+.5, .1);
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}