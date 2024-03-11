#version 120

attribute vec2 coord2d;
varying vec3 f_color;

void main(void) {
  gl_Position = vec4(coord2d, 0.0, 1.0);
  f_color = vec3(coord2d, 1.0);         
}