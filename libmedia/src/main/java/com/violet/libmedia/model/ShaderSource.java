package com.violet.libmedia.model;

public class ShaderSource {
    public static final String TRIANGLE_VERTEX_SHADER
            = "#version 300 es\n"
            + "layout(location = 0) in vec4 aPosition;\n"
            + "uniform mat4 u_MVPMatrix;\n"
            + "void main() {\n"
            + "  gl_Position = u_MVPMatrix * aPosition;\n"
            + "}\n";

    public static final String TRIANGLE_FRAGMENT_SHADER
            = "#version 300 es\n"
            + "precision mediump float;\n"
            + "out vec4 fragColor;\n"
            + "void main() {\n"
            + "    fragColor = vec4(1.0, 1.0, 0.0, 1.0);\n"
            + "}\n";

    public static final String IMAGE_VERTEX_SHADER
            = "#version 300 es\n"
            + "layout(location = 0) in vec4 vPosition;\n"
            + "layout(location = 1) in vec2 vTexCoord;\n"
            + "uniform mat4 vMatrix;\n"
            + "out vec2 fTexCoord;\n"
            + "void main() {\n"
            + "  gl_Position = vMatrix * vPosition;\n"
            + "  vMatrix = vTexCoord;\n"
            + "}\n";

    public static final String IMAGE_FRAGMENT_SHADER
            = "#version 300 es\n"
            + "precision highp float;\n"
            + "in vec2 fTexCoord;\n"
            + "layout(location = 0) out vec4 outColor;\n"
            + "uniform sampler2D texture0;\n"
            + "uniform sampler2D texture1;\n"
            + "uniform sampler2D texture2;\n"
            + "uniform int imageType;\n"
            + "void main() {\n"
            + "  if (imageType == 0) {   // RGBA \n"
            + "    outColor = texture(texture0, fTexCoord);"
            + "  }\n"
            + "}\n";
}
