package com.violet.libmedia.util

object ShaderSource {
    const val TRIANGLE_VERTEX_SHADER =
        """
            #version 300 es
            layout(location = 0) in vec4 aPosition;
            layout(location = 1) in vec4 aColor;
            uniform mat4 u_MVPMatrix;
            out vec4 vColor;
            void main() {"
              gl_Position = u_MVPMatrix * aPosition;
              vColor = aColor;
            }
        """

    const val TRIANGLE_FRAGMENT_SHADER =
        """
            #version 300 es
            precision mediump float;
            in vec4 vColor;
            out vec4 fragColor;
            void main() {
                fragColor = vColor;
            }     
        """

    const val IMAGE_VERTEX_SHADER =
        """
            #version 300 es
            layout(location = 0) in vec4 a_position;
            layout(location = 1) in vec2 a_texCoord;
            uniform mat4 u_MVPMatrix;
            out vec2 v_texCoord;
            void main()
            {
                gl_Position = u_MVPMatrix * a_position;
                v_texCoord = a_texCoord;
            }
        """

    const val IMAGE_FRAGMENT_SHADER =
        """
            #version 300 es
            precision highp float;
            in vec2 v_texCoord;
            layout(location = 0) out vec4 outColor;
            uniform sampler2D s_texture0;
            uniform sampler2D s_texture1;
            uniform sampler2D s_texture2;
            uniform int u_nImgType;// 1:RGBA, 2:NV21, 3:NV12, 4:I420
            
            void main()
            {
                if(u_nImgType == 1) //RGBA
                {
                    outColor = texture(s_texture0, v_texCoord);
                }
                else if(u_nImgType == 2) //NV21
                {
                    vec3 yuv;
                    yuv.x = texture(s_texture0, v_texCoord).r;
                    yuv.y = texture(s_texture1, v_texCoord).a - 0.5;
                    yuv.z = texture(s_texture1, v_texCoord).r - 0.5;
                    highp vec3 rgb = mat3(1.0,    1.0,   1.0,
                                          0.0,   -0.344, 1.770,
                                          1.403, -0.714, 0.0) * yuv;
                    outColor = vec4(rgb, 1.0);
            
                }
                else if(u_nImgType == 3) //NV12
                {
                    vec3 yuv;
                    yuv.x = texture(s_texture0, v_texCoord).r;
                    yuv.y = texture(s_texture1, v_texCoord).r - 0.5;
                    yuv.z = texture(s_texture1, v_texCoord).a - 0.5;
                    highp vec3 rgb = mat3(1.0,    1.0,     1.0,
                                          0.0,   -0.344,   1.770,
                                          1.403,  -0.714,  0.0) * yuv;
                    outColor = vec4(rgb, 1.0);
                }
                else if(u_nImgType == 4) //I420
                {
                    vec3 yuv;
                    yuv.x = texture(s_texture0, v_texCoord).r;
                    yuv.y = texture(s_texture1, v_texCoord).r - 0.5;
                    yuv.z = texture(s_texture2, v_texCoord).r - 0.5;
                    highp vec3 rgb = mat3(1.0,    1.0,     1.0,
                                          0.0,   -0.344,   1.770,
                                          1.403, -0.714,   0.0) * yuv;
                    outColor = vec4(rgb, 1.0);
                }
                else
                {
                    outColor = vec4(1.0);
                }
            }
        """
}