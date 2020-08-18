//
// Created by lzm on 2020/8/18.
//

#include "MVideo.h"

MVideo::MVideo(MPlaystatus *Playstatus, MCallJava *mCallJava) {
    this->videoPlaystatus = Playstatus;
    this->mCallJava = mCallJava;
    mQueue = new MQueue(videoPlaystatus);

}
/**
 *
 * @param data
 * @return
 */
void *playvideo(void *data)
{
    MVideo *mVideo = static_cast<MVideo *>(data);

    while (mVideo->videoPlaystatus!=NULL && !mVideo->videoPlaystatus->exit)
    {

        if(mVideo->videoPlaystatus->seek)
        {
            av_usleep(1000*100);
            continue;
        }
        if(mVideo->mQueue->getQueueSIze() == 0)
        {
            if(!mVideo->videoPlaystatus->load)
            {
                mVideo->videoPlaystatus->load=true;
                mVideo->mCallJava->onCallLoad(CHILD_THREAD,true);
            }
            av_usleep(1000*100);
            continue;
        }else{

            if(mVideo->videoPlaystatus->load)
            {
                //开始播放
                mVideo->videoPlaystatus->load= false;
                mVideo->mCallJava->onCallLoad(CHILD_THREAD, false);
            }
        }

        AVPacket *avPacket = av_packet_alloc();
        //队列里无数据
        if(mVideo->mQueue->outAvpacket(avPacket) != 0)
        {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        if(avcodec_send_packet(mVideo->avCodecContext,avPacket) != 0)
        {
            //出错,可能有丢帧
            continue;
        }

        AVFrame *avFrame = av_frame_alloc();
        //接受一帧,0:success
        if(avcodec_receive_frame(mVideo->avCodecContext, avFrame)!=0)
        {
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame=NULL;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        LOGD("解码AVFRAME：success");

        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame=NULL;
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }




    pthread_exit(&mVideo->threadPlayVideo);
}
/**
 *
 */
void MVideo::playVideo() {
    pthread_create(&threadPlayVideo,NULL,playvideo,this);
}
/**
 *
 */
void MVideo::release() {
    if(mQueue != NULL)
    {
        delete(mQueue);
        mQueue = NULL;
    }
    if(avCodecContext != NULL)
    {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }
    if(videoPlaystatus != NULL)
    {
        videoPlaystatus = NULL;
    }
    if(mCallJava != NULL)
    {
        mCallJava = NULL;
    }
}

MVideo::~MVideo() {

}
