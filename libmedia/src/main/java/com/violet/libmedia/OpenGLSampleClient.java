package com.violet.libmedia;

public class OpenGLSampleClient {
    static {
        System.loadLibrary("moonmedia");
    }

    private long address;

    public void initOpenGLESSampleRender() {
        address = native_initOpenGLESSampleRender();
    }

    public void loadSample(int type) {
        native_loadOpenGLSample(address, type);
    }

    public void onDrawFrame() {
        native_onDrawFrame(address);
    }

    public void onSurfaceCreated(Object surface) {
        native_onSurfaceCreated(address, surface);
    }

    public void onSurfaceChanged(int w, int h) {
        native_onSurfaceChanged(address, w, h);
    }

    public void onSurfaceDestroyed() {
        native_onSurfaceDestroyed(address);
    }

    private native long native_initOpenGLESSampleRender();

    private native void native_loadOpenGLSample(long address, int type);

    private native void native_onDrawFrame(long address);

    private native void native_onSurfaceCreated(long address, Object surface);

    private native void native_onSurfaceChanged(long address, int w, int h);

    private native void native_onSurfaceDestroyed(long address);
}
