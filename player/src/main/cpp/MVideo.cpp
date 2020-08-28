//
// Created by lzm on 2020/8/18.
//


#include "MVideo.h"

MVideo::MVideo(MPlaystatus *Playstatus, MCallJava *mCallJava) {
    this->videoPlaystatus = Playstatus;
    this->mCallJava = mCallJava;
    mQueue = new MQueue(videoPlaystatus);

    pthread_mutex_init(&codecMutex,NULL);

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
        if(mVideo->videoPlaystatus->pause)
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

        //判断是否支持硬解码
        if(mVideo->codecType == CODEC_MEDIACODEC)
        {
            //
            if(av_bsf_send_packet(mVideo->avbsfContext,avPacket) != 0)
            {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }
            while (av_bsf_receive_packet(mVideo->avbsfContext,avPacket) == 0)
            {
                LOGE("开始硬解码");

                double diff = mVideo->getFrameTimeDiff(NULL,avPacket);
                LOGE("diff is %f",diff);

                av_usleep(mVideo->getDelayTime(diff) * 1000000);//微秒与秒的转换

                mVideo->mCallJava->onCallDecodeAvPacket(avPacket->size,avPacket->data);

                av_packet_free(&avPacket);
                av_free(avPacket);
                continue;
            }
            avPacket = NULL;

        } else if(mVideo->codecType == CODEC_YUV)
        {
            pthread_mutex_lock(&mVideo->codecMutex);
            if(avcodec_send_packet(mVideo->avCodecContext,avPacket) != 0)
            {
                //出错,可能有丢帧
                pthread_mutex_unlock(&mVideo->codecMutex);
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
                pthread_mutex_unlock(&mVideo->codecMutex);
                continue;
            }

            //处理avframe，420p直接给OpenGL，非420p要转换格式
            if(avFrame->format == AV_PIX_FMT_YUV420P)
            {
                double diff = mVideo->getFrameTimeDiff(avFrame,NULL);
                av_usleep(mVideo->getDelayTime(diff) * 1000000);//微秒与秒的转换


                mVideo->mCallJava->onCallRenderYUV(
                        mVideo->avCodecContext->width,
                        mVideo->avCodecContext->height,
                        avFrame->data[0],//Y
                        avFrame->data[1],//U
                        avFrame->data[2]//V
                );
            } else{
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
                    pthread_mutex_unlock(&mVideo->codecMutex);
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
                double diff = mVideo->getFrameTimeDiff(avFrameYUV420P,NULL);
                av_usleep(mVideo->getDelayTime(diff) * 1000000);//微秒与秒的转换
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
            pthread_mutex_unlock(&mVideo->codecMutex);
        }

    }

    return 0;
    //pthread_exit(&mVideo->threadPlayVideo);
}
/**
 *
 */
void MVideo::playVideo() {

    if(videoPlaystatus !=NULL && !videoPlaystatus->exit)
    {
        pthread_create(&threadPlayVideo,NULL,playvideo,this);
    }
}
/**
 *
 */
void MVideo::release() {

    //等待线程退出
    if(mQueue != NULL)
    {
        mQueue->noticeQueue();
    }

    pthread_join(threadPlayVideo,NULL);

    if(mQueue != NULL)
    {
        delete(mQueue);
        mQueue = NULL;
    }

    if(avbsfContext != NULL)
    {
        av_bsf_free(&avbsfContext);
        avbsfContext = NULL;
    }
    if(avCodecContext != NULL)
    {
        pthread_mutex_lock(&codecMutex);
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
        pthread_mutex_unlock(&codecMutex);
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
    pthread_mutex_destroy(&codecMutex);
}

double MVideo::getFrameTimeDiff(AVFrame *avFrame,AVPacket *avPacket) {
    //获取当前时间戳
    double pts = 0;
    if(avFrame != NULL)
    {
        pts = av_frame_get_best_effort_timestamp(avFrame);
    }
    if(avPacket!=NULL)
    {
        pts = avPacket->pts;
    }

    if(pts == AV_NOPTS_VALUE)
    {
        pts = 0;
    }

    //获取视频流
    pts *= av_q2d(time_base);

    //保存全局时钟
    if(pts > 0)
    {
        clock = pts;
    }

    //音频时钟减视频时钟
    double diff = audio->clock-clock;

    return diff;
}

double MVideo::synchronize(AVFrame *srcFrame, double pts) {
    double frame_delay;

    if (pts != 0)
        video_clock = pts; // Get pts,then set video clock to it
    else
        pts = video_clock; // Don't get pts,set it to video clock

    frame_delay = av_q2d(time_base);
    frame_delay += srcFrame->repeat_pict * (frame_delay * 0.5);

    video_clock += frame_delay;

    return pts;
}

/**
 *
 * @param diff
 * @return
 */
double MVideo::getDelayTime(double diff) {

    if(diff > 0.003)
    {
        delayTime = delayTime / 3 * 2;

        if(delayTime < defaultDelayTime / 2)
        {
            delayTime = defaultDelayTime / 3 * 2;
        }
        else if(delayTime > defaultDelayTime * 2)
        {
            delayTime = defaultDelayTime * 2;
        }

    }
    else if(diff < -0.003)
    {
        delayTime = delayTime * 3 / 2;
        if(delayTime < defaultDelayTime / 2)
        {
            delayTime = defaultDelayTime / 3 * 2;
        }
        else if(delayTime > defaultDelayTime * 2)
        {
            delayTime = defaultDelayTime * 2;
        }
    }else if(diff == 0)
    {
        delayTime = defaultDelayTime;
    }
    if(diff > 1.0)
    {
        delayTime = 0;
    }
    if(diff < -1.0)
    {
        delayTime = defaultDelayTime * 2;
    }
    if(fabs(diff) > 10)
    {
        delayTime = defaultDelayTime;
    }
    return delayTime;
}

