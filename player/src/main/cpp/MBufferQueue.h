//
// Created by nxf65025 on 2020/8/18.
//

#ifndef VIDEOAPP_MBUFFERQUEUE_H
#define VIDEOAPP_MBUFFERQUEUE_H

#include <deque>
#include "MPlaystatus.h"
#include "MPcmBean.h"
#include "pthread.h"
#include "AndroidLog.h"
extern "C"
{
#include <libavcodec/avcodec.h>
};

class MBufferQueue {

public:
    std::deque<MPcmBean *> queueBuffer;
    pthread_mutex_t mutexBuffer;
    pthread_cond_t condBuffer;
    MPlaystatus *mPlayStatus = NULL;

public:
    MBufferQueue(MPlaystatus *playStatus);
    ~MBufferQueue();

    int putBuffer(SAMPLETYPE *buffer, int size);

    int getBuffer(MPcmBean **pcmBean);

    int clearBuffer();

    void release();
    int getBufferSize();

    int noticeThread();
};

#endif //VIDEOAPP_MBUFFERQUEUE_H
