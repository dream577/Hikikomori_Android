package com.violet.libmedia.opengles;

import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

import com.violet.libmedia.OpenGLSampleClient;

import org.jetbrains.annotations.NotNull;

public class OpenGLSampleWrapper implements SurfaceHolder.Callback, Runnable {
    private final SurfaceView mSurfaceView;
    private final OpenGLSampleClient mClient;

    public OpenGLSampleWrapper(@NotNull SurfaceView view) {
        this.mSurfaceView = view;
        mSurfaceView.getHolder().addCallback(this);
        mClient = new OpenGLSampleClient();
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        mClient.initOpenGLESSampleRender();
        mClient.onSurfaceCreated(holder.getSurface());
        mClient.loadSample(OpenGLSample.SAMPLE_TYPE_TRIANGLE.getType());
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        mClient.onSurfaceChanged(width, height);
        Thread thread = new Thread(this);
        thread.start();
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        mClient.onSurfaceDestroyed();
    }


    @Override
    public void run() {
        int i = 0;
        do {
            mClient.onDrawFrame();
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        } while (i++ < 10);
    }
}
