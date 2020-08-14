//
// Created by lzm on 19-2-20.
//

#ifndef MUSIC2_MAUDIO_H
#define MUSIC2_MAUDIO_H

#include "MQueue.h"
#include "MPlaystatus.h"
#include "MCallJava.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
extern"C"
{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
};
class MAudio {

public:
    int streamIndex = -1;
    AVCodecParameters *codecPar = NULL;
    AVCodecContext *avCodecCtx = NULL;
    MQueue *queue = NULL;
    MPlaystatus *mPlaystatus = NULL;
    MCallJava *callJava = NULL;

    pthread_t pthread_play;
    AVPacket *avPacket = NULL;
    int ret = -1;
    AVFrame *avFrame = NULL;
    uint8_t *buffer =NULL;
    int data_size = 0;
    int sample_rate = 0;

    int duration = 0;
    AVRational time_base;
    double now_time = 0;
    double clock = 0;
    double last_time = 0;
    int defaultvolume = 60;
    //OpenSLES
    //定义引擎接口
    SLObjectItf engineObjectItf = NULL;
    SLEngineItf engineEngine = NULL;
    //混音器
    SLObjectItf mixouputObject = NULL;
    SLEnvironmentalReverbItf outptmixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONEROOM;
    //pcm
    SLObjectItf  pcmplayer = NULL;
    SLPlayItf slPlayItf = NULL;

    //声音接口
    SLVolumeItf slVolumeItf = NULL;

    //声道接口
    SLMuteSoloItf slMuteSoloItf = NULL;

    //PCM缓冲区
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;



public:
    MAudio(MPlaystatus *mPlaystatus, int sample_rate, MCallJava *CallJava);

    ~MAudio();

    void play();

    int resampleAudio();

    void initOpenSLES();

    int getCurrentSampleRateForOpenSLES(int sample_rate);

    void pause();

    void resume();

    void stop();

    void release();

    void setVolume(int percent);

    void setMute(int mute);


};


#endif //MUSIC2_MAUDIO_H
