//
// Created by bronyna on 2023/2/25.
//

#ifndef HIKIKOMORI_CAMERAVIDEORECORDER_H
#define HIKIKOMORI_CAMERAVIDEORECORDER_H

#include "Callback.h"
#include "GLRenderWindow.h"
#include "ThreadSafeQueue.h"
#include "MediaDef.h"

#include <stdio.h>
#include <pthread.h>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/frame.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
#include "libavutil/avassert.h"
#include "libavutil/mathematics.h"
#include "libavutil/timestamp.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavutil/log.h"
#include "libavformat/avio.h"
}

#define   ADTS_HEADER_LEN   7

const int sampling_frequencies[] = {
        96000,  // 0x0
        88200,  // 0x1
        64000,  // 0x2
        48000,  // 0x3
        44100,  // 0x4
        32000,  // 0x5
        24000,  // 0x6
        22050,  // 0x7
        16000,  // 0x8
        12000,  // 0x9
        11025,  // 0xa
        8000   // 0xb
        // 0xc d e f是保留的
};

class AVOutputStream {
public:
    AVFormatContext *oc;
    AVCodecContext *cc;
    const AVCodec *c;
    AVStream *st;
    AVFrame *sf;
    AVFrame *df;
    SwsContext *ss;
    SwrContext *sr;
    long sampleCount;
    long nextPts;

    AVOutputStream() {
        oc = nullptr;
        cc = nullptr;
        c = nullptr;
        st = nullptr;
        sf = nullptr;
        df = nullptr;
        sr = nullptr;
        ss = nullptr;
        sampleCount = 0;
        nextPts = 0;
    }

    ~AVOutputStream() {
        if (cc) {
            avcodec_free_context(&cc);
            cc = nullptr;
        }
        st = nullptr;
        c = nullptr;
        if (sf) {
            av_frame_free(&sf);
            sf = nullptr;
        }
        if (df) {
            av_frame_free(&df);
            df = nullptr;
        }
        if (sr) {
            swr_free(&sr);
            sr = nullptr;
        }
        if (ss) {
            sws_freeContext(ss);
            ss = nullptr;
        }
        sampleCount = 0;
    }
};

class CameraVideoRecorder : public RenderCallback {

private:
    GLRenderWindow *m_RenderWindow;

    AVOutputStream *m_VideoOst;
    AVOutputStream *m_AudioOst;

    AVFormatContext *m_FormatCtx = nullptr;
    char m_FilePath[1024];
    char m_FileName[1024];

    /**
     * Video
     */
    ThreadSafeQueue *m_VideoEncoderQueue;
    ThreadSafeQueue *m_VideoRenderQueue;
    bool m_EnableVideo;
    int m_ImageWidth;
    int m_ImageHeight;
    int m_ImageFormat;
    int m_VideoBitRate;
    int fps;

    /**
     * Audio
     */
    ThreadSafeQueue *m_AudioEncoderQueue;
    bool m_EnableAudio;
    int m_SampleRate = 44100;
    int m_SampleFormat = AV_SAMPLE_FMT_S16;
    int m_ChannelLayout = AV_CH_LAYOUT_STEREO;

    pthread_t worker;
    volatile bool m_IsAudioRecording = false;
    volatile bool m_IsVideoRecording = false;
    volatile bool m_Exit = false;

    int AddStream(AVOutputStream *ost, AVCodecID codec_id);

    int OpenAudio(AVOutputStream *ost);

    int EncodeAudioFrame(AVOutputStream *ost, AudioFrame *frame);

    int OpenVideo(AVOutputStream *ost);

    int EncodeVideo1Frame(AVOutputStream *ost, VideoFrame *frame);

    void RealStopRecord();

    static void *StartRecordLoop(void *recorder);

public:

    CameraVideoRecorder();

    ~CameraVideoRecorder();

    int Init();

    int UnInit();

    void StartRecord();

    void StopRecord();

    void InputVideoData(uint8_t *data, int width, int height, int format, long timestamp);

    void InputAudioData(uint8_t *data, int size, long timestamp, int sample_rate,
                        int sample_format, int channel_layout);

    Frame *GetOneFrame(int type) override;

    GLRenderWindow *GetVideoRender() {
        return m_RenderWindow;
    }

public:
    class RecorderBuilder {
    private:
        CameraVideoRecorder *r = nullptr;
        char file_path[1024];
        char file_name[1024];

        bool enable_video_record = false;
        int image_width = 1920;
        int image_height = 1080;
        int image_format;
        int video_bit_rate;
        int fps;

        bool enable_audio_record = false;
        int sample_rate = 44100;
        int sample_format = AV_SAMPLE_FMT_FLTP;
        int channel_layout = AV_CH_LAYOUT_STEREO;
    public:
        RecorderBuilder() {}

        RecorderBuilder(CameraVideoRecorder *recorder) {
            this->r = recorder;
        }

        RecorderBuilder *InitFile(const char *p_file_path, const char *p_file_name) {
            strcpy(file_path, p_file_path);
            strcpy(file_name, p_file_name);
            return this;
        }

        RecorderBuilder *EnableAudioRecord(bool enable) {
            enable_audio_record = enable;
            return this;
        }

        RecorderBuilder *ConfigAudioParam(int p_sample_rate, int p_sample_format,
                                          int p_channel_layout) {
            this->sample_rate = p_sample_rate;
            this->sample_format = p_sample_format;
            this->channel_layout = p_channel_layout;
            return this;
        }

        RecorderBuilder *EnableVideoRecord(bool enable) {
            enable_video_record = enable;
            return this;
        }

        RecorderBuilder *ConfigVideoParam(int p_image_width, int p_image_height, int p_image_format,
                                          int p_video_bit_rate, int p_fps) {
            this->image_width = p_image_width;
            this->image_height = p_image_height;
            this->image_format = p_image_format;
            this->video_bit_rate = p_video_bit_rate;
            this->fps = p_fps;
            return this;
        }

        CameraVideoRecorder *Build() {
            if (!r) {
                r = new CameraVideoRecorder();
            }
            strcpy(r->m_FilePath, file_path);
            strcpy(r->m_FileName, file_name);

            r->m_EnableVideo = enable_video_record;
            r->m_ImageWidth = image_width;
            r->m_ImageHeight = image_height;
            r->m_ImageFormat = image_format;
            r->m_VideoBitRate = video_bit_rate;
            r->fps = this->fps;

            r->m_EnableAudio = enable_audio_record;
            r->m_SampleRate = sample_rate;
            r->m_SampleFormat = sample_format;
            r->m_ChannelLayout = channel_layout;
            return r;
        }

        ~RecorderBuilder() {
            r = nullptr;
        }
    };

    RecorderBuilder *Rebuild(CameraVideoRecorder *r) {
        return new RecorderBuilder(r);
    }
};

#endif //HIKIKOMORI_CAMERAVIDEORECORDER_H
