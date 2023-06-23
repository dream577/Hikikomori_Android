package com.violet.libmedia.util;

public class ShaderSource {
    public static final String TRIANGLE_VERTEX_SHADER
            = "#version 300 es\n"
            + "layout(location = 0) in vec4 aPosition;\n"
            + "layout(location = 1) in vec4 aColor;\n"
            + "uniform mat4 u_MVPMatrix;\n"
            + "out vec4 vColor;\n"
            + "void main() {\n"
            + "  gl_Position = u_MVPMatrix * aPosition;\n"
            + "  vColor = aColor;\n"
            + "}\n";

    public static final String TRIANGLE_FRAGMENT_SHADER
            = "#version 300 es\n"
            + "precision mediump float;\n"
            + "in vec4 vColor;\n"
            + "out vec4 fragColor;\n"
            + "void main() {\n"
            + "    fragColor = vColor;\n"
            + "}\n";

    public static final String IMAGE_VERTEX_SHADER
            = "#version 300 es\n"
            + "layout(location = 0) in vec4 a_position;\n"
            + "layout(location = 1) in vec2 a_texCoord;\n"
            + "uniform mat4 u_MVPMatrix;\n"
            + "out vec2 v_texCoord;\n"
            + "void main()\n"
            + "{\n"
            + "    gl_Position = u_MVPMatrix * a_position;\n"
            + "    v_texCoord = a_texCoord;\n"
            + "}";

    public static final String IMAGE_FRAGMENT_SHADER
            = "#version 300 es\n"
            + "precision highp float;\n"
            + "in vec2 v_texCoord;\n"
            + "layout(location = 0) out vec4 outColor;\n"
            + "uniform sampler2D s_texture0;\n"
            + "uniform sampler2D s_texture1;\n"
            + "uniform sampler2D s_texture2;\n"
            + "uniform int u_nImgType;// 1:RGBA, 2:NV21, 3:NV12, 4:I420\n"
            + "\n"
            + "void main()\n"
            + "{\n"
            + "\n"
            + "    if(u_nImgType == 1) //RGBA\n"
            + "    {\n"
            + "        outColor = texture(s_texture0, v_texCoord);\n"
            + "    }\n"
            + "    else if(u_nImgType == 2) //NV21\n"
            + "    {\n"
            + "        vec3 yuv;\n"
            + "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
            + "        yuv.y = texture(s_texture1, v_texCoord).a - 0.5;\n"
            + "        yuv.z = texture(s_texture1, v_texCoord).r - 0.5;\n"
            + "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
            + "        0.0, \t-0.344, \t1.770,\n"
            + "        1.403,  -0.714,     0.0) * yuv;\n"
            + "        outColor = vec4(rgb, 1.0);\n"
            + "\n"
            + "    }\n"
            + "    else if(u_nImgType == 3) //NV12\n"
            + "    {\n"
            + "        vec3 yuv;\n"
            + "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
            + "        yuv.y = texture(s_texture1, v_texCoord).r - 0.5;\n"
            + "        yuv.z = texture(s_texture1, v_texCoord).a - 0.5;\n"
            + "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
            + "        0.0, \t-0.344, \t1.770,\n"
            + "        1.403,  -0.714,     0.0) * yuv;\n"
            + "        outColor = vec4(rgb, 1.0);\n"
            + "    }\n"
            + "    else if(u_nImgType == 4) //I420\n"
            + "    {\n"
            + "        vec3 yuv;\n"
            + "        yuv.x = texture(s_texture0, v_texCoord).r;\n"
            + "        yuv.y = texture(s_texture1, v_texCoord).r - 0.5;\n"
            + "        yuv.z = texture(s_texture2, v_texCoord).r - 0.5;\n"
            + "        highp vec3 rgb = mat3(1.0,       1.0,     1.0,\n"
            + "                              0.0, \t-0.344, \t1.770,\n"
            + "                              1.403,  -0.714,     0.0) * yuv;\n"
            + "        outColor = vec4(rgb, 1.0);\n"
            + "    }\n"
            + "    else\n"
            + "    {\n"
            + "        outColor = vec4(1.0);\n"
            + "    }\n"
            + "}";
}
