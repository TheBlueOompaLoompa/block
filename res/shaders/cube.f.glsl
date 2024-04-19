#version 330 core
out vec4 FragColor;

in vec3 f_normal;
varying vec2 f_texcoord;
uniform sampler2D mytexture;
uniform float time;

vec3 light = vec3(.5*(cos(time)/2.0+.5), time, .8*(cos(time)/2.0+.5));

void main(void) {
    vec2 flipped = vec2(f_texcoord.y, 1.0-f_texcoord.x);
    float b = min(max(0.3, dot(normalize(light), f_normal) * 2.0), 1.0);

    FragColor = texture2D(mytexture, flipped);

    // Light color/angle brightness
    FragColor *= vec4(b, b, b, 1.0);

    // Time of day brightness
    FragColor *= max(cos(time)/2.0+.5, .1);
}