#version 120
#片源着色器


precision mediump float;


varying vec2 vTexture_Position;

#unifrm用于在app向vertex和fragement传递值

uniform sampler2D sTexture;

void main() {
    gl_FragColor=texture2D(sTexture, vTexture_Position);
}