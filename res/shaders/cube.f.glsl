
#version 330 core


in float instance;

varying vec2 f_texcoord;
uniform sampler2D mytexture;

void main(void) {
    vec2 flipped = vec2(f_texcoord.x, 1.0 - f_texcoord.y);
    gl_FragColor = texture2D(mytexture, flipped);
}