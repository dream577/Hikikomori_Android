#version 300 es

precision highp float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_texture0;
uniform sampler2D s_texture1;
uniform sampler2D s_texture2;
uniform int u_nImageType;

void main() {
    highp float y = texture(s_texture0, v_texCoord).r;
    highp float u = texture(s_texture1, v_texCoord).r - 0.5;
    highp float v = texture(s_texture2, v_texCoord).r - 0.5;

    highp float r = y + 1.402 * v;
    highp float g = y - 0.344 * u - 0.714 * v;
    highp float b = y + 1.722 * u;

    outColor = vec4(r, g, b, 1.0);
}
