//
// Created by lzm on 2020/8/18.
//

#ifndef VIDEOAPP_MVIDEO_H
#define VIDEOAPP_MVIDEO_H

#include "MQueue.h"
#include "MCallJava.h"
#include "pthread.h"
extern"C"
{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};
class MVideo{

public:
    //视频数据流索引
    int  streamindex =-1;
    AVCodecContext *avCodecContext = NULL;
    //参数属性
    AVCodecParameters *avCodecParameters = NULL;

    AVRational time_base;

    MQueue *mQueue = NULL;
    MPlaystatus *videoPlaystatus = NULL;
    MCallJava *mCallJava = NULL;

    pthread_t threadPlayVideo;

public:
    MVideo(MPlaystatus *mPlaystatus, MCallJava *mCallJava);
    ~MVideo();

    void playVideo();

    void release();

};
#endif //VIDEOAPP_MVIDEO_H
