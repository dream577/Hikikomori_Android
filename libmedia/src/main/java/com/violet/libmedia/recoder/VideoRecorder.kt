package com.violet.libmedia.recoder

import android.content.Context
import android.hardware.camera2.CameraManager
import com.violet.libmedia.recoder.VideoRecorder
import android.hardware.camera2.CameraAccessException
import kotlin.Throws
import android.graphics.ImageFormat
import android.hardware.camera2.CameraCharacteristics
import android.media.ImageReader
import android.os.Handler
import android.os.HandlerThread
import java.lang.Exception
import java.lang.RuntimeException
import java.util.concurrent.Callable

class VideoRecorder(private val mContext: Context, private val mCameraId: String) {
    private lateinit var mCameraManager: CameraManager

    private lateinit var characteristics: CameraCharacteristics

    private lateinit var imageReader: ImageReader

    private lateinit var cameraThread: HandlerThread

    private lateinit var cameraHandler: Handler

    private var mSupportIds: Array<String>? = null

    init {
        mCameraManager = mContext.getSystemService(Context.CAMERA_SERVICE) as CameraManager
        characteristics = mCameraManager.getCameraCharacteristics(mCameraId)
    }

    fun initCamera2(): Int {
        mCameraManager = mContext.getSystemService(Context.CAMERA_SERVICE) as CameraManager
        try {
            mSupportIds = mCameraManager.cameraIdList
            if (checkSupportCameraId(DEFAULT_CAMERA_ID.toString())) {
                return -1
            }
        } catch (e: CameraAccessException) {
            throw RuntimeException(e)
        }
        return 0
    }

    private fun checkSupportCameraId(cameraId: String?): Boolean {
        if (cameraId == null || mSupportIds == null || mSupportIds!!.size == 0) return false
        for (id in mSupportIds!!) {
            if (cameraId == id) {
                return true
            }
        }
        return false
    }

    //    private int initCameraParam() {
    //        CameraCharacteristics characteristics = null;
    //        try {
    //            characteristics = mCameraManager.getCameraCharacteristics(mCameraId);
    //        } catch (CameraAccessException e) {
    //            throw new RuntimeException(e);
    //        }
    //    }

    companion object {
        private const val DEFAULT_CAMERA_ID = 0
        private const val DEFAULT_IMAGE_FORMAT = ImageFormat.NV21
    }
}