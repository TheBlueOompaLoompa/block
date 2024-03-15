
#version 330 core


//in float instance;

void main(void) {
    vec2 flipped = vec2(f_texcoord.x, 1.0 - f_texcoord.y);
    gl_FragColor = vec4(normalize(vec3(gl_NormalMatrix * gl_Normal)), 1.0);
}