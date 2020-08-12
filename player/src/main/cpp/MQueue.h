//
// Created by lzm on 19-2-20.
//实现C++缓存AVPacket
//

#ifndef MUSIC2_MQUEUE_H
#define MUSIC2_MQUEUE_H

#include "queue"
#include "pthread.h"
#include "MPlaystatus.h"
#include "AndroidLog.h"

extern "C"
{
#include <libavcodec/avcodec.h>
};
class MQueue {

public:
    std::queue<AVPacket *>queuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    MPlaystatus *playstatus = NULL;
public:
    MQueue(MPlaystatus *mPlaystatus);
    ~MQueue();

    int putAvpacket(AVPacket *avPacket);
    int outAvpacket(AVPacket *avPacket);
    int getQueueSIze();

    void clearAvpacket();

};


#endif //MUSIC2_MQUEUE_H
