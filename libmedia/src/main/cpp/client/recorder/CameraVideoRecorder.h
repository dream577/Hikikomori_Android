//
// Created by bronyna on 2023/2/25.
//

#ifndef HIKIKOMORI_CAMERAVIDEORECORDER_H
#define HIKIKOMORI_CAMERAVIDEORECORDER_H

#include "Callback.h"
#include "GLRenderWindow.h"
#include "CustomContainer.h"
#include "FFVideoEncoder.h"
#include "FFAudioEncoder.h"

class CameraVideoRecorder : public RenderCallback, public EncoderCallback {

private:
    shared_ptr<GLRenderWindow> m_RenderWindow;

    AVFormatContext *m_FormatCtx = nullptr;
    char m_FilePath[1024];
    char m_FileName[1024];

    /**
     * Video
     */
    shared_ptr<FFVideoEncoder> m_VideoEncoder;
    AVStream *m_VideoStream = nullptr;
    bool m_IsCameraRecorder = true;
    shared_ptr<LinkedBlockingQueue<MediaFrame>> m_VideoRenderQueue;

    bool m_EnableVideo;
    int m_ImageWidth;
    int m_ImageHeight;
    int m_ImageFormat;
    int m_VideoBitRate;
    int m_FrameRate;

    /**
     * Audio
     */
    shared_ptr<FFAudioEncoder> m_AudioEncoder;
    AVStream *m_AudioStream = nullptr;

    bool m_EnableAudio;
    int m_SampleRate = 44100;
    int m_SampleFormat = AV_SAMPLE_FMT_S16;
    int m_ChannelLayout = AV_CH_LAYOUT_STEREO;

    pthread_t worker;
    volatile bool m_IsAudioRecording = false;
    volatile bool m_IsVideoRecording = false;
    volatile bool m_RecordModeExit = true;

    void RealStopRecord();

    static void *StartRecordLoop(void *recorder);

public:

    CameraVideoRecorder(bool isCameraRecorder);

    ~CameraVideoRecorder();

    int Init();

    int UnInit();

    void StartRecord();

    void StopRecord();

    void InputVideoFrame(uint8_t *data, int width, int height, int format);

    void InputAudioFrame(uint8_t *data, int size, int sample_rate, int sample_format, int channel_layout);


    shared_ptr<MediaFrame> GetOneFrame(int type) override;

    void FrameRendFinish(shared_ptr<MediaFrame> frame) override;

    void OnFrameEncoded(AVPacket *pkt, AVMediaType type) override;

    shared_ptr<GLRenderWindow> GetVideoRender() {
        return m_RenderWindow;
    }

public:
    class RecorderBuilder {
    private:
        CameraVideoRecorder *r = nullptr;
        char file_path[1024];
        char file_name[1024];
        bool use_camera_record = true;

        bool enable_video_record = false;
        int image_width = 1920;
        int image_height = 1080;
        int image_format = AV_PIX_FMT_YUV420P;
        int64_t video_bit_rate = 0;
        int frame_rate = 15;

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

        RecorderBuilder *UseCameraRecord(bool use) {
            use_camera_record = use;
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
                                          int64_t p_video_bit_rate, int p_frame_rate) {
            this->image_width = p_image_width;
            this->image_height = p_image_height;
            this->image_format = p_image_format;
            this->video_bit_rate = p_video_bit_rate;
            this->frame_rate = p_frame_rate;
            return this;
        }

        CameraVideoRecorder *Build() {
            if (!r) {
                r = new CameraVideoRecorder(use_camera_record);
            }
            strcpy(r->m_FilePath, file_path);
            strcpy(r->m_FileName, file_name);
            r->m_IsCameraRecorder = use_camera_record;

            r->m_EnableVideo = enable_video_record;
            r->m_ImageWidth = image_width;
            r->m_ImageHeight = image_height;
            r->m_ImageFormat = image_format;
            r->m_VideoBitRate = video_bit_rate;
            r->m_FrameRate = this->frame_rate;

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
