package com.violet.libmedia.recoder.video.camera.util;

import android.graphics.ImageFormat;
import android.media.Image;

import com.violet.libbasetools.util.KLog;

import java.nio.ByteBuffer;

public class ImageUtils {

    public static byte[] convert_YUV420_888_To_YUV420I_Data(Image image) {
        int imageWidth = image.getWidth();
        int imageHeight = image.getHeight();
        Image.Plane[] planes = image.getPlanes();
        byte[] data = new byte[imageWidth * imageHeight * ImageFormat.getBitsPerPixel(ImageFormat.YUV_420_888) / 8];
        int offset = 0;

//        KLog.d("ImageSize", String.format("width = %s, height = %s", imageWidth, imageHeight));

        for (int i = 0; i < planes.length; i++) {
            int rowStride = planes[i].getRowStride();
            int pixelStride = planes[i].getPixelStride();
//            KLog.d("ImageSize", String.format("plane%s:[rawStride = %s, pixelStride=%s]", i, rowStride, pixelStride));

            int planeWidth = i == 0 ? imageWidth : imageWidth / 2;
            int planeHeight = i == 0 ? imageHeight : imageHeight / 2;

            ByteBuffer buffer = planes[i].getBuffer();

            if (pixelStride == 1 && rowStride == planeWidth) {
                buffer.get(data, offset, planeWidth * planeHeight);
                offset += planeWidth * planeHeight;
            } else {
                byte[] rowData = new byte[rowStride];
                for (int row = 0; row < planeHeight - 1; row++) {
                    buffer.get(rowData, 0, rowStride);
                    for (int col = 0; col < planeWidth; col++) {
                        data[offset++] = rowData[col * pixelStride];
                    }
                }

                buffer.get(rowData, 0, Math.min(rowStride, buffer.remaining()));
                for (int col = 0; col < planeWidth; ++col) {
                    data[offset++] = rowData[col * pixelStride];
                }
            }
        }
        return data;
    }
}
