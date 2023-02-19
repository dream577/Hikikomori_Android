package com.violet.libmedia.recoder;

import android.content.Context;
import android.graphics.ImageFormat;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;

import java.util.List;
import java.util.concurrent.Callable;

public class VideoRecorder implements Callable<Boolean> {

    private static final int DEFAULT_CAMERA_ID = 0;
    private static final int DEFAULT_IMAGE_FORMAT = ImageFormat.NV21;
    private Context mContext;
    private CameraManager mCameraManager;
    private String[] mSupportIds;
    private String mCameraId;

    public VideoRecorder(Context context) {
        this.mContext = context;
    }

    public int initCamera2() {
        mCameraManager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
        try {
            mCameraManager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
            mSupportIds = mCameraManager.getCameraIdList();
            mCameraId = String.valueOf(DEFAULT_CAMERA_ID);
            if (checkSupportCameraId(String.valueOf(DEFAULT_CAMERA_ID))) {
                return -1;
            }
        } catch (CameraAccessException e) {
            throw new RuntimeException(e);
        }
    }

    private boolean checkSupportCameraId(String cameraId) {
        if (cameraId == null || mSupportIds == null || mSupportIds.length == 0) return false;
        for (String id : mSupportIds) {
            if (cameraId.equals(id)) {
                return true;
            }
        }
        return false;
    }

    private int initCameraParam() {
        CameraCharacteristics characteristics = null;
        try {
            characteristics = mCameraManager.getCameraCharacteristics(mCameraId);
        } catch (CameraAccessException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public Boolean call() throws Exception {
        return null;
    }
}
