package com.violet.libmedia.recoder;

public interface Recorder {
    int init();

    int receiveOneFrame();

    int unInit();
}
