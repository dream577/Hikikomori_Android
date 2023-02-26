package com.violet.libmedia.recoder.video.camera;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.text.TextUtils;
import android.util.Size;
import android.view.Display;
import android.view.Surface;

import androidx.annotation.NonNull;

import com.violet.libbasetools.util.KLog;
import com.violet.libmedia.MediaContext;
import com.violet.libmedia.recoder.video.camera.util.CameraSizesKt;
import com.violet.libmedia.recoder.video.camera.util.ImageUtils;

import org.jetbrains.annotations.NotNull;

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

public class VioletCamera {
    private static final String TAG = "VioletCamera";
    private final String DEFAULT_CAMERA_ID = "0";
    private final float THRESHOLD = 0.001f;

    private Context mContext;
    private CameraManager mCameraManager;
    private CameraCharacteristics mCharacteristics;
    private CameraDevice mCameraDevice;
    private String mCameraId;

    private String[] mSupportCameraIds;
    private CameraCaptureSession mCaptureSession;
    private CaptureRequest mPreviewRequest, mCaptureRequest;
    private ImageReader mPreviewImageReader, mCaptureImageReader;
    private Surface mPreviewSurface, mCaptureSurface;
    private Integer mSensorOrientation;

    private Size mDefaultPreviewSize = new Size(1280, 720);
    private Size mDefaultCaptureSize = new Size(1280, 720);
    private Size mPreviewSize = mDefaultPreviewSize;
    private Size mCaptureSize = mDefaultCaptureSize;
    private List<Size> mSupportPreviewSize, mSupportCaptureSize;

    private HandlerThread mPreviewThread;
    private Handler mCameraHandler;

    private final CameraFrameCallback mCallback;

    private final Semaphore cameraLock = new Semaphore(1);

    public VioletCamera(@NotNull Context context, @NotNull CameraFrameCallback callback) {
        mContext = context;
        mCallback = callback;
        mCameraId = DEFAULT_CAMERA_ID;
    }

    public boolean initializeCamera(@NotNull Display display) throws CameraAccessException {
        boolean result = false;
        do {
            mCameraManager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
            if (mCameraManager == null) break;

            boolean isSupportCameraId = false;
            mSupportCameraIds = mCameraManager.getCameraIdList();
            for (String cameraId : mSupportCameraIds) {
                if (TextUtils.equals(cameraId, mCameraId)) {
                    isSupportCameraId = true;
                    break;
                }
            }
            if (!isSupportCameraId) break;

            mCharacteristics = mCameraManager.getCameraCharacteristics(mCameraId);
            int[] capabilities = mCharacteristics.get(CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES);
            boolean isBackWardCompatible = false;
            for (int capability : capabilities) {
                if (capability == CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES_BACKWARD_COMPATIBLE) {
                    isBackWardCompatible = true;
                    break;
                }
            }
            if (!isBackWardCompatible) break;
            StreamConfigurationMap config = mCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            if (config == null) break;
            mSupportPreviewSize = Arrays.asList(config.getOutputSizes(SurfaceTexture.class));
            mSupportCaptureSize = Arrays.asList(config.getOutputSizes(ImageFormat.YUV_420_888));
            mPreviewSize = CameraSizesKt.getPreviewOutputSize(display, mCharacteristics, SurfaceTexture.class, null);
            mCaptureSize = CameraSizesKt.getPreviewOutputSize(display, mCharacteristics, SurfaceTexture.class, ImageFormat.YUV_420_888);
            mSensorOrientation = mCharacteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);

            KLog.d(TAG, String.format("mPreviewSize=[%s,%s]  mSensorOrientation=%s", mPreviewSize.getWidth(), mPreviewSize.getHeight(), mSensorOrientation.toString()));
            result = true;
        } while (false);
        return result;
    }

    @SuppressLint("MissingPermission")
    public void startCamera() {
        mPreviewThread = new HandlerThread("CameraThread");
        mPreviewThread.start();
        mCameraHandler = new Handler(mPreviewThread.getLooper());

        if (mPreviewImageReader == null && mPreviewSize != null) {
            mPreviewImageReader = ImageReader.newInstance(mPreviewSize.getWidth(), mPreviewSize.getHeight(), ImageFormat.YUV_420_888, 2);
            mPreviewImageReader.setOnImageAvailableListener(mPreviewImageAvailableListener, mCameraHandler);
            mPreviewSurface = mPreviewImageReader.getSurface();
        }

        if (mCaptureImageReader == null && mCaptureSize != null) {
            mCaptureImageReader = ImageReader.newInstance(mCaptureSize.getWidth(), mCaptureSize.getHeight(), ImageFormat.YUV_420_888, 2);
            mCaptureImageReader.setOnImageAvailableListener(mCaptureImageAvailableListener, mCameraHandler);
            mCaptureSurface = mCaptureImageReader.getSurface();
        }

        try {
            if (!cameraLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }
            mCameraManager.openCamera(mCameraId, mCameraStateCallback, mCameraHandler);
        } catch (CameraAccessException e) {
            KLog.d(TAG, "openCamera 发生异常");
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void createSession() {
        if (mCameraDevice == null || mPreviewSurface == null) {
            return;
        }
        List<Surface> targets = List.of(mPreviewSurface);
        try {
            mCameraDevice.createCaptureSession(targets, mCaptureStateCallback, mCameraHandler);
        } catch (CameraAccessException e) {
            KLog.d(TAG, "createCaptureSession 发生异常");
            e.printStackTrace();
        }
    }

    private void createPreviewRequest() {
        if (mCameraDevice == null || mPreviewSurface == null || mCaptureSession == null || mCameraHandler == null) {
            return;
        }
        try {
            CaptureRequest.Builder builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            builder.addTarget(mPreviewSurface);
            mPreviewRequest = builder.build();
            mCaptureSession.setRepeatingRequest(mPreviewRequest, null, mCameraHandler);
        } catch (CameraAccessException e) {
            KLog.d(TAG, "createCaptureRequest 发生异常");
            e.printStackTrace();
        }
    }

//    private boolean createCaptureRequest() {
//        if (mCameraDevice == null || mCaptureSurface == null || mCaptureSession == null) {
//            return false;
//        }
//        try {
//            CaptureRequest.Builder builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
//            builder.addTarget(mPreviewImageReader.getSurface());
//            mPreviewRequest = builder.build();
//            mCaptureSession.capture(mPreviewRequest, null, mPreviewHandler);
//        } catch (CameraAccessException e) {
//            KLog.d(TAG, "createCaptureRequest 发生异常");
//            e.printStackTrace();
//            return false;
//        }
//        return true;
//    }


    private final CameraDevice.StateCallback mCameraStateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            mCameraDevice = camera;
            createSession();
            cameraLock.release();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice camera) {
            camera.close();
            cameraLock.release();
        }

        @Override
        public void onError(@NonNull CameraDevice camera, int error) {
            camera.close();
            cameraLock.release();
            String message = "";
            switch (error) {
                case ERROR_CAMERA_DEVICE:
                    message = "Fatal (device)";
                    break;
                case ERROR_CAMERA_DISABLED:
                    message = "Device policy";
                    break;
                case ERROR_CAMERA_IN_USE:
                    message = "Camera in use";
                    break;
                case ERROR_CAMERA_SERVICE:
                    message = "Fatal (service)";
                    break;
                case ERROR_MAX_CAMERAS_IN_USE:
                    message = "Maximum cameras in use";
                    break;
                default:
                    message = "Unknown";
            }
            KLog.d(TAG, "Camera " + mCameraId + "error: " + message);
        }

    };

    private final CameraCaptureSession.StateCallback mCaptureStateCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(@NonNull CameraCaptureSession session) {
            mCaptureSession = session;
            createPreviewRequest();
        }

        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession session) {
            KLog.d(TAG, "onConfigureFailed");
        }
    };

    private final ImageReader.OnImageAvailableListener mPreviewImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            Image image = reader.acquireLatestImage();
            byte[] data = ImageUtils.convert_YUV420_888_To_YUV420I_Data(image);
            mCallback.onPreviewFrame(data, image.getWidth(), image.getHeight(), MediaContext.VIDEO_FRAME_FORMAT_I420, image.getTimestamp());
            image.close();
        }
    };

    private final ImageReader.OnImageAvailableListener mCaptureImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {

        }
    };

    public void closeCamera() {
        try {
            cameraLock.acquire();
            if (mCaptureSession != null) {
                mCaptureSession.close();
            }

            if (mCameraDevice != null) {
                mCameraDevice.close();
            }

            if (mPreviewImageReader != null) {
                mPreviewImageReader.close();
            }

            if (mCaptureImageReader != null) {
                mCaptureImageReader.close();
            }
            cameraLock.release();
        } catch (InterruptedException e) {
            e.printStackTrace();
            KLog.d(TAG, "closeCamera error");
        }
    }

    public int getSensorOrientation() {
        if (mSensorOrientation == null) return 0;
        return mSensorOrientation;
    }
}
