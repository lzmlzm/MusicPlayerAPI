//
// Created by lzm on 19-2-20.
//

#ifndef MUSIC2_MFFMPEG_H
#define MUSIC2_MFFMPEG_H

#include "MCallJava.h"
#include "pthread.h"
#include "MAudio.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
};

class MFFmpeg {


public:
    MCallJava *callJava = NULL;

    const char *url = NULL;

    pthread_t  decodeThread{};

    AVFormatContext *pFormatCtx = NULL;//上下文

    MAudio *audio = NULL;

    MPlaystatus *mPlaystatus = NULL;

    pthread_mutex_t init_mutex{};

    bool exit = false;

    int duration = 0;

    pthread_mutex_t seek_mutex{};

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
};


#endif //MUSIC2_MFFMPEG_H
