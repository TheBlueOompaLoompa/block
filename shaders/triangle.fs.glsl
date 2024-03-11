#version 120

varying vec3 f_color;

void main(void) {
    gl_FragColor = vec4(f_color.r, f_color.g, 0.0, 0.5);
}