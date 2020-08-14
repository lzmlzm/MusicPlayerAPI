//
// Created by lzm on 19-2-20.
//

#include "MQueue.h"

MQueue::MQueue(MPlaystatus *mPlaystatus) {

    this->playstatus = mPlaystatus;
    pthread_mutex_init(&mutexPacket,NULL);
    pthread_cond_init(&condPacket,NULL);

}

MQueue::~MQueue() {
    clearAvpacket();
    pthread_mutex_destroy(&mutexPacket);
    pthread_cond_destroy(&condPacket);

}

int MQueue::putAvpacket(AVPacket *avPacket) {
    //音视频数据包入队操作
    pthread_mutex_lock(&mutexPacket);
    queuePacket.push(avPacket);
    if(LOG_DEBUG)
    {
        LOGD("PUT ONE AVPACKET TO QUEUE,NUM:%d",queuePacket.size());
    }

    pthread_cond_signal(&condPacket);//发信号给消费者
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int MQueue::outAvpacket(AVPacket *avPacket) {
    //音视频数据出队操作
    pthread_mutex_lock(&mutexPacket);
    while(playstatus != NULL && !playstatus->exit)
    {

        if(queuePacket.size() >0)
        {
            AVPacket *avPacket1 = queuePacket.front();
            if(av_packet_ref(avPacket, avPacket1) == 0)
            {
                queuePacket.pop();
            }

            av_packet_free(&avPacket1);
            av_free(avPacket1);
            avPacket1 = NULL;
            if(LOG_DEBUG)
            {
                LOGD("GET AVPACKET FROM QUEUE ,LEAVE ：%d",queuePacket.size());
            }
            break;
        }else{
            pthread_cond_wait(&condPacket,&mutexPacket);

        }
    }

    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int MQueue::getQueueSIze() {

    int size = 0;
    pthread_mutex_lock(&mutexPacket);
    size = queuePacket.size();
    pthread_mutex_unlock(&mutexPacket);
    return size;

}

void MQueue::clearAvpacket() {

    pthread_cond_signal(&condPacket);
    pthread_mutex_lock(&mutexPacket);

    while(!queuePacket.empty())
    {
        AVPacket *avPacket = queuePacket.front();
        queuePacket.pop();
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }

    pthread_mutex_unlock(&mutexPacket);


}
