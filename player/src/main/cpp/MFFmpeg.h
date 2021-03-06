//
// Created by lzm on 19-2-20.
//

#ifndef MUSIC2_MFFMPEG_H
#define MUSIC2_MFFMPEG_H

#include "MCallJava.h"
#include "pthread.h"
#include "MAudio.h"
#include "MVideo.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
};

class MFFmpeg {


public:
    MCallJava *callJava = NULL;

    const char *url = NULL;

    pthread_t  decodeThread;

    AVFormatContext *pFormatCtx = NULL;//上下文

    MAudio *audio = NULL;

    MVideo *mVideo = NULL;

    MPlaystatus *mPlaystatus = NULL;

    pthread_mutex_t init_mutex;

    bool exit = false;

    int duration = 0;

    pthread_mutex_t seek_mutex;

    bool supportMediaCodec = false;

    const AVBitStreamFilter *avBitStreamFilter = NULL;




public:
    MFFmpeg(MPlaystatus *mPlaystatus,MCallJava *callJava, const char*url);

    ~MFFmpeg();

    void prepared();

    void decodeFFmpegThread();

    void start();

    void pause();

    void resume();

    void release();

    void seek(int64_t secs);

    void setVolume(int percent);

    void setMute(int mute);

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getSamplerate();

    void setRecordStatus(bool start);

    bool cutAudio(int start,int end, bool returnPcm);

    int getCodecContext(AVCodecParameters *avCodecParameters, AVCodecContext **avCodecCtx);


};


#endif //MUSIC2_MFFMPEG_H
