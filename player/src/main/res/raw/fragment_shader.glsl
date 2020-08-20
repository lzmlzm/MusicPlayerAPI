precision mediump float;
varying vec2 vTexture_Position;

uniform sampler2D sample_y;
uniform sampler2D sample_u;
uniform sampler2D sample_v;

void main() {

    float y,u,v;

    y=texture2D(sample_y, vTexture_Position).r;
    u=texture2D(sample_u, vTexture_Position).r-0.5;
    v=texture2D(sample_v, vTexture_Position).r-0.5;

    vec3 rgb;

    rgb.r = y+1.403*v;
    rgb.g = y-0.344*u-0.714*v;
    rgb.b = y+1.770*u;

    gl_FragColor = vec4(rgb, 1);

}