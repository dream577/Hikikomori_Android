package com.violet.libmedia.opengles;

public enum OpenGLSample {
    SAMPLE_TYPE_TRIANGLE(0),
    ;
    private final int type;

    OpenGLSample(int type) {
        this.type = type;
    }

    public int getType() {
        return type;
    }
}
