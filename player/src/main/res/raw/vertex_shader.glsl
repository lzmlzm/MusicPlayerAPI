#version 120

#顶点着色器

#顶点坐标系
attribute vec4 av_Position;
#纹理坐标系
attribute vec2 af_Position;
#在顶点与片源之间传递值
varying vec2 vTexture_Position;

void main() {
    vTexture_Position = af_Position;
    gl_Position = av_Position;
}