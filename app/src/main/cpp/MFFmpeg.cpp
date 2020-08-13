//
// Created by lzm on 19-2-20.
//


#include "MFFmpeg.h"

MFFmpeg::MFFmpeg(MPlaystatus *mPlaystatus,MCallJava *callJava, const char *url) {

    this->callJava=callJava;
    this->url=url;
    this->mPlaystatus = mPlaystatus;
    pthread_mutex_init(&init_mutex,NULL);
    pthread_mutex_init(&seek_mutex,NULL);

}

void *decodeFFmpeg(void *data)
{
    MFFmpeg *mfFmpeg = (MFFmpeg *)data;//数据强制转换
    mfFmpeg->decodeFFmpegThread();
    pthread_exit(&mfFmpeg->decodeThread);
}
void MFFmpeg::prepared() {
    pthread_create(&decodeThread,NULL,decodeFFmpeg,this);//创建数据解析线程

}

int avformat_callback(void *ctx)
{
    MFFmpeg *mfFmpeg = (MFFmpeg *)(ctx);
    if(mfFmpeg->mPlaystatus->exit)
    {
        return AVERROR_EOF;
    }
    return  0;

}

void MFFmpeg::decodeFFmpegThread() {
    pthread_mutex_lock(&init_mutex);
    //解码线程！

    //1.注册管理器
    av_register_all();

    //2.网络初始化

    avformat_network_init();

    pFormatCtx = avformat_alloc_context();

    pFormatCtx->interrupt_callback.callback = avformat_callback;
    pFormatCtx->interrupt_callback.opaque = this;
    //打开资源
    if(avformat_open_input(&pFormatCtx,url,NULL,NULL) != 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("CAN NOT OPEN URL:%s", url);
            callJava->onCallError(CHILD_THREAD,1001,"CAN NOT OPEN URL");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    if(avformat_find_stream_info(pFormatCtx,NULL) < 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("CAN NOT FIND STREAM FROM URL:%s",url);
            callJava->onCallError(CHILD_THREAD,1002,"CAN NOT FIND STREAM FROM URL");

        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }


    //获取音频流
    for(int i=0;i < pFormatCtx->nb_streams;i++)
    {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)//判断类型是否是AUDIO
        {
            if(audio == NULL)
            {
                audio = new MAudio(mPlaystatus, pFormatCtx->streams[i]->codecpar->sample_rate,
                                   callJava);
                audio->streamIndex = i;
                audio->codecPar = pFormatCtx->streams[i]->codecpar;
                audio->duration = pFormatCtx->duration / AV_TIME_BASE;//计算时间
                audio->time_base = pFormatCtx->streams[i]->time_base;
                duration = audio->duration;

            }
        }

    }

    //找到解码器

    AVCodec *avCodec = avcodec_find_decoder(audio->codecPar->codec_id);
    if(!avCodec)
    {
        if(LOG_DEBUG)
        {
            LOGE("CAN NOT FIND DECODER");
            callJava->onCallError(CHILD_THREAD,1003,"CAN NOT FIND DECODER");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }


    audio->avCodecCtx = avcodec_alloc_context3(avCodec);
    if(!audio->avCodecCtx)
    {
        if(LOG_DEBUG)
        {
            LOGE("CAN NOT ALLOC NEW DECODERCTX");
            callJava->onCallError(CHILD_THREAD,1004,"CAN NOT ALLOC NEW DECODERCTX");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if(avcodec_parameters_to_context(audio->avCodecCtx,audio->codecPar) < 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("CAN NOT FILL DECODER");
            callJava->onCallError(CHILD_THREAD,1005,"CAN NOT FILL DECODER");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    if(avcodec_open2(audio->avCodecCtx,avCodec,0) != 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("CAN NOT OPEN AUDIO STREAMS");
            callJava->onCallError(CHILD_THREAD,1006,"CAN NOT OPEN AUDIO STREAMS");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    callJava->onCallPrepared(CHILD_THREAD);
    pthread_mutex_unlock(&init_mutex);


}

void MFFmpeg::start() {
    if(audio == NULL)
    {
        if(LOG_DEBUG)
        {
            LOGE("AUDIO IS NULL");
            callJava->onCallError(CHILD_THREAD,1007,"AUDIO IS NULL");
        }
        return;
    }
    audio->play();

    int count;
    while(mPlaystatus !=NULL && !mPlaystatus->exit)
    {
        if(mPlaystatus->seek)
        {
            continue;
        }

        if(audio->queue->getQueueSIze() > 40)
        {
            continue;//存40帧
        }

        AVPacket *avPacket = av_packet_alloc();

        pthread_mutex_lock(&seek_mutex);

        int ret = av_read_frame(pFormatCtx,avPacket);

        pthread_mutex_unlock(&seek_mutex);

        if(ret == 0)
        {
            if(avPacket->stream_index == audio->streamIndex)
            {
                count++;
                if(LOG_DEBUG)
                {
                    LOGE("DECODE %d FRAME", count);
                }
                audio->queue->putAvpacket(avPacket);
            } else{
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
            }
        }else{
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            while (mPlaystatus!= NULL && !mPlaystatus->exit)
            {
                if(audio->queue->getQueueSIze() > 0){
                    continue;
                } else{
                    mPlaystatus->exit = true;
                    break;
                }
            }
        }
    }

    exit = true;

    while(audio->queue->getQueueSIze() > 0) {
        AVPacket *avPacket = av_packet_alloc();
        audio->queue->outAvpacket(avPacket);
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }


    if(LOG_DEBUG)
    {
        LOGD("OK!");
    }



}

void MFFmpeg::pause() {
    if (audio != NULL) {
        audio->pause();
    }
}

void MFFmpeg::resume() {
    if (audio != NULL) {
        audio->resume();
    }
}

void MFFmpeg::release() {

    if(mPlaystatus->exit)
    {
        return;
    }
    mPlaystatus->exit = true;
    pthread_mutex_lock(&init_mutex);

    int sleepCount = 0;
    while (!exit)
    {
        if (sleepCount > 1000)
        {
            exit = true;
        }

        if(LOG_DEBUG)
        {
            LOGE("WAIT FFMPEG EXIT %d",sleepCount);
        }
        sleepCount++;
        av_usleep(1000 * 10);
    }
    if(audio != NULL)
    {

        audio->release();
        delete(audio);
        audio = NULL;
    }
    if(pFormatCtx != NULL)
    {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = NULL;
    }
    if(mPlaystatus != NULL)
    {
        mPlaystatus = NULL;
    }
    if(callJava != NULL)
    {
        callJava = NULL;
    }
    pthread_mutex_unlock(&init_mutex);

}

MFFmpeg::~MFFmpeg() {

    pthread_mutex_destroy(&seek_mutex);
    pthread_mutex_destroy(&init_mutex);
}

void MFFmpeg::seek(int64_t secs) {

    if(duration<=0)
    {
        return;
    }
    if(secs >= 0 && secs <= duration)
    {
        if(audio!=NULL)
        {
            mPlaystatus->seek = true;
            audio->queue->clearAvpacket();//跳转播放前先清空队列
            audio->clock = 0;//时间置零
            audio->last_time = 0;
            pthread_mutex_lock(&seek_mutex);

            int64_t rel = secs*AV_TIME_BASE;
            avformat_seek_file(pFormatCtx,-1,INT64_MIN,rel,INT64_MAX,0);

            pthread_mutex_unlock(&seek_mutex);
            mPlaystatus->seek = false;

        }
    }

}