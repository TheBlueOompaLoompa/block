#version 330 core
//$../FastNoiseLite.glsl

in vec2 UV;

out float color;

uniform vec2 resolution;

void main(){
    color = UV.x * UV.y;
}