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

        //处理avframe，420p直接给OpenGL，非420p要转换格式
        if(avFrame->format == AV_PIX_FMT_YUV420P)
        {
            mVideo->mCallJava->onCallRenderYUV(
                    mVideo->avCodecContext->width,
                    mVideo->avCodecContext->height,
                    avFrame->data[0],//Y
                    avFrame->data[1],//U
                    avFrame->data[2]//V
                    );
        } else{
            LOGD("不是YUV420P");
            AVFrame *avFrameYUV420P = av_frame_alloc();
            //从解码器上下文获得视频的长宽，计算要转成yuv420p的空间大小
            int outYUVBufferSize = av_image_get_buffer_size(
                    AV_PIX_FMT_YUV420P,
                    mVideo->avCodecContext->width,
                    mVideo->avCodecContext->height,1);

            uint8_t *outYUV420pBuffer = static_cast<uint8_t *>(av_malloc(outYUVBufferSize * sizeof(uint8_t)));

            //填充转换
            av_image_fill_arrays(
                    avFrameYUV420P->data,
                    avFrameYUV420P->linesize,
                    outYUV420pBuffer,
                    AV_PIX_FMT_YUV420P,
                    mVideo->avCodecContext->width,
                    mVideo->avCodecContext->height,
                    1);
            //转换上下文
            SwsContext *swsContext = sws_getContext(
                    mVideo->avCodecContext->width,
                    mVideo->avCodecContext->height,
                    mVideo->avCodecContext->pix_fmt,
                    mVideo->avCodecContext->width,
                    mVideo->avCodecContext->height,
                    AV_PIX_FMT_YUV420P,
                    SWS_BICUBIC,//转码算法
                    NULL,
                    NULL,
                    NULL
                    );
            //转码器上下文分配失败
            if(!swsContext)
            {
                av_frame_free(&avFrameYUV420P);
                av_free(avFrameYUV420P);
                av_free(outYUV420pBuffer);
                continue;
            }

            //转换
            sws_scale(
                    swsContext,
                    avFrame->data,
                    avFrame->linesize,
                    0,
                    avFrame->height,
                    avFrameYUV420P->data,
                    avFrameYUV420P->linesize
                    );
            //渲染
            mVideo->mCallJava->onCallRenderYUV(
                    mVideo->avCodecContext->width,
                    mVideo->avCodecContext->height,
                    avFrameYUV420P->data[0],//Y
                    avFrameYUV420P->data[1],//U
                    avFrameYUV420P->data[2]//V
            );
        }
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
