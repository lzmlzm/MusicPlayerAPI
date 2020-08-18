//
// Created by nxf65025 on 2020/8/18.
//

#include "MBufferQueue.h"

MBufferQueue::MBufferQueue(MPlaystatus *playStatus) {
    mPlayStatus = playStatus;
    pthread_mutex_init(&mutexBuffer, NULL);
    pthread_cond_init(&condBuffer, NULL);
}

MBufferQueue::~MBufferQueue() {
    mPlayStatus = NULL;
    pthread_mutex_destroy(&mutexBuffer);
    pthread_cond_destroy(&condBuffer);
    if(LOG_DEBUG){
        LOGE("MBufferQueue 释放完了");
    }
}

void MBufferQueue::release() {
    if(LOG_DEBUG){
        LOGE("MBufferQueue::release");
        noticeThread();
        clearBuffer();
    }

    if(LOG_DEBUG){
        LOGE("MBufferQueue::release success");
    }
}

int MBufferQueue::putBuffer(SAMPLETYPE *buffer, int size) {
    pthread_mutex_lock(&mutexBuffer);
    MPcmBean *pcmBean = new MPcmBean(buffer, size);
    queueBuffer.push_back(pcmBean);
    pthread_cond_signal(&condBuffer);
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int MBufferQueue::getBuffer(MPcmBean **pcmBean) {
    pthread_mutex_lock(&mutexBuffer);

    while(mPlayStatus != NULL && !mPlayStatus->exit){
        if(queueBuffer.size() > 0){
            *pcmBean = queueBuffer.front();
            queueBuffer.pop_front();
            break;
        } else {
            if(!mPlayStatus->exit){
                pthread_cond_wait(&condBuffer, &mutexBuffer);
            }
        }
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int MBufferQueue::clearBuffer() {

    pthread_cond_signal(&condBuffer);
    pthread_mutex_lock(&mutexBuffer);
    while (!queueBuffer.empty()){
        MPcmBean *pcmBean = queueBuffer.front();
        queueBuffer.pop_front();
        delete(pcmBean);
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int MBufferQueue::getBufferSize() {
    int size = 0;
    pthread_mutex_lock(&mutexBuffer);
    size = queueBuffer.size();
    pthread_mutex_unlock(&mutexBuffer);
    return size;
}

int MBufferQueue::noticeThread() {
    pthread_cond_signal(&condBuffer);
    return 0;
}