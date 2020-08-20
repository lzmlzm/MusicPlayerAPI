attribute vec4 av_Position;
attribute vec2 af_Position;

varying vec2 vTexture_Position;

void main() {

    vTexture_Position = af_Position;
    gl_Position = av_Position;

}