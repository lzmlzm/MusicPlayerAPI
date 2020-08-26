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

/**
 * ffmpeg解码函数
 * @param data
 * @return
 */
void *decodeFFmpeg(void *data)
{
    MFFmpeg *mfFmpeg = (MFFmpeg *)data;//数据强制转换
    mfFmpeg->decodeFFmpegThread();
    pthread_exit(&mfFmpeg->decodeThread);
}
/**
 * 资源解析
 */
void MFFmpeg::prepared() {
    //创建数据解析线程
    pthread_create(&decodeThread,NULL,decodeFFmpeg,this);

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


/**
 * ffmpeg资源解析函数
 */
void MFFmpeg::decodeFFmpegThread() {
    pthread_mutex_lock(&init_mutex);

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
            //回调java
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
        //获取音频流
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            if(audio == NULL)
            {
                //设置OpenSL ES
                audio = new MAudio(mPlaystatus, pFormatCtx->streams[i]->codecpar->sample_rate,
                                   callJava);
                audio->streamIndex = i;
                //设置音频解码器属性
                audio->codecPar = pFormatCtx->streams[i]->codecpar;
                audio->duration = pFormatCtx->duration / AV_TIME_BASE;//计算时间
                audio->time_base = pFormatCtx->streams[i]->time_base;
                duration = audio->duration;
                //回调pcm samplerate
                callJava->onCallPcmRate(audio->sample_rate);
            }
        }
        //获取视频流
        else if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if(mVideo == NULL)
            {
                mVideo = new MVideo(mPlaystatus,callJava);
                //设置索引
                mVideo->streamindex = i;
                //设置视频解码器属性
                mVideo->avCodecParameters=pFormatCtx->streams[i]->codecpar;
                mVideo->time_base = pFormatCtx->streams[i]->time_base;

                int num = pFormatCtx->streams[i]->avg_frame_rate.num;
                int den = pFormatCtx->streams[i]->avg_frame_rate.den;
                //获取默认延迟时间 1 / fps
                if(num != 0 && den != 0)
                {
                    int fps = num/den;//（30/1）
                    mVideo->defaultDelayTime = 1.0 / fps;
                }
            }
        }
    }

    //获取音频解码器上下文
    if(audio!=NULL)
    {
        getCodecContext(audio->codecPar,&audio->avCodecCtx);
    }
    //获取视频解码器上下文
    if(mVideo!=NULL)
    {
        getCodecContext(mVideo->avCodecParameters,&mVideo->avCodecContext);
    }

    if(callJava!=NULL)
    {
        if(mPlaystatus!=NULL &&!mPlaystatus->exit)
        {
            callJava->onCallPrepared(CHILD_THREAD);
        }else{
            exit = true;
        }
    }

    pthread_mutex_unlock(&init_mutex);

}


//开始处理数据
/**
 * 开始解码
 */
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
    //
    //开始从队列接收数据并播放
    //
    mVideo->audio = audio;
    audio->play();
    mVideo->playVideo();
    //检测播放状态
    while(mPlaystatus !=NULL && !mPlaystatus->exit)
    {
        if(mPlaystatus->seek)
        {
            av_usleep(1000*100);
            continue;
        }
        //缓存30个包
        if(audio->queue->getQueueSIze() > 50)
        {
            av_usleep(1000*100);
            continue;//存40帧再处理数据
        }
        //一个avpacket为一帧的数据包
        AVPacket *avPacket = av_packet_alloc();

        pthread_mutex_lock(&seek_mutex);

        //能否读到帧数据
        int ret = av_read_frame(pFormatCtx,avPacket);

        pthread_mutex_unlock(&seek_mutex);

        if(ret == 0)
        {   //读到有效帧数据,将avPacket入队
            if(avPacket->stream_index == audio->streamIndex)
            {
                //音频avPacket入队
                audio->queue->putAvpacket(avPacket);
            }
            else if(avPacket->stream_index == mVideo->streamindex)
            {
                //视频avPacket入队
                LOGD("视频数据入队")
                mVideo->mQueue->putAvpacket(avPacket);
            }
            else{
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
            }
        }else{
            //读不到有效帧数据，释放资源并推出播放
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            while (mPlaystatus!= NULL && !mPlaystatus->exit)
            {
                if(audio->queue->getQueueSIze() > 0)
                {
                    av_usleep(1000*100);
                    continue;
                } else{
                    if(!mPlaystatus->seek)
                    {
                        av_usleep(1000*500);
                        mPlaystatus->exit = true;
                    }
                    break;
                }
            }
        }
    }

    //？？？待注释
    while(audio->queue->getQueueSIze() > 0) {
        AVPacket *avPacket = av_packet_alloc();
        audio->queue->outAvpacket(avPacket);
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }
    callJava->onCallComplete(CHILD_THREAD);
}

/**
 * 设置暂停播放
 */
void MFFmpeg::pause() {
    if(mPlaystatus!=NULL)
    {
        mPlaystatus->pause = true;
    }
    if (audio != NULL) {
        audio->pause();
    }
}
/**
 * 恢复播放
 */
void MFFmpeg::resume() {
    if(mPlaystatus!=NULL)
    {
        mPlaystatus->pause = false;
    }
    if (audio != NULL) {
        audio->resume();
    }
}

/**
 * 资源释放
 */
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
    if(mVideo != NULL)
    {

        mVideo->release();
        delete(mVideo);
        mVideo = NULL;
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

/**
 * seek功能
 * @param secs
 */
void MFFmpeg::seek(int64_t secs) {

    if(duration<=0)
    {
        return;
    }
    if(secs >= 0 && secs <= duration)
    {
        mPlaystatus->seek = true;
        pthread_mutex_lock(&seek_mutex);

        int64_t rel = secs*AV_TIME_BASE;
        avformat_seek_file(pFormatCtx,-1,INT64_MIN,rel,INT64_MAX,0);

        if(audio!=NULL)
        {

            audio->queue->clearAvpacket();//跳转播放前先清空队列
            audio->clock = 0;//时间置零
            audio->last_time = 0;
            //清除seek时残余的前几帧
            pthread_mutex_lock(&audio->codecMutex);
            avcodec_flush_buffers(audio->avCodecCtx);
            pthread_mutex_unlock(&audio->codecMutex);
        }
        if(mVideo!=NULL)
        {
            mVideo->mQueue->clearAvpacket();//跳转播放前先清空队列
            mVideo->clock = 0;//时间置零
            //清除seek时残余的前几帧
            pthread_mutex_lock(&mVideo->codecMutex);
            avcodec_flush_buffers(mVideo->avCodecContext);
            pthread_mutex_unlock(&mVideo->codecMutex);
        }

        pthread_mutex_unlock(&seek_mutex);//加锁否则奔溃
        mPlaystatus->seek = false;
    }

}

/**
 * 设置音量
 * @param percent
 */
//包装底层SL的音量设置函数
void MFFmpeg::setVolume(int percent) {
    if(audio!=NULL)
    {
        //设置音量
        audio->setVolume(percent);
    }
}
/**
 * ffmpeg层调用audio设置左右声道
 * @param mute
 */
void MFFmpeg::setMute(int mute) {
    if(audio!=NULL)
    {
        //设置声道
        audio->setMute(mute);
    }
}

/**
 * ffmpeg层调用audio设置音调
 * @param pitch
 */
void MFFmpeg::setPitch(float pitch) {
    if(audio!=NULL)
    {
        audio->setPitch(pitch);
    }
}

/**
 * ffmpeg调用audio设置播放速度
 * @param speed
 */
void MFFmpeg::setSpeed(float speed) {
    if(audio!=NULL)
    {
        audio->setSpeed(speed);
    }
}

/**
 * 返回采样率
 * @return 采样率
 */
int MFFmpeg::getSamplerate() {
    if (audio!=NULL)
    {
        return audio->avCodecCtx->sample_rate;
    }
    return 0;
}

/**
 * ffmpeg层设置录音状态
 * @param start
 */
void MFFmpeg::setRecordStatus(bool start) {
    if (audio!=NULL)
    {
        audio->setRecordStatus(start);
    }
}
/**
 * ffmpeg音频裁剪
 * @param start
 * @param end
 * @param returnPcm 是否返回pcm数据给java
 * @return
 */
bool MFFmpeg::cutAudio(int start, int end, bool returnPcm) {
    //限制时间范围
    if(start >= 0 && end <= duration && start < end)
    {
        audio->isCut = true;
        audio->end_time = end;
        audio->returnPcm = returnPcm;
        //先seek到开始时间
        seek(start);
        return true;
    }
    return false;
}

int MFFmpeg::getCodecContext(AVCodecParameters *avCodecParameters, AVCodecContext **avCodecCtx)
{
    //找到解码器

    AVCodec *avCodec = avcodec_find_decoder(avCodecParameters->codec_id);
    if(!avCodec)
    {
        if(LOG_DEBUG)
        {
            LOGE("CAN NOT FIND DECODER");
            callJava->onCallError(CHILD_THREAD,1003,"CAN NOT FIND DECODER");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }


    *avCodecCtx = avcodec_alloc_context3(avCodec);
    if(!audio->avCodecCtx)
    {
        if(LOG_DEBUG)
        {
            LOGE("CAN NOT ALLOC NEW DECODERCTX");
            callJava->onCallError(CHILD_THREAD,1004,"CAN NOT ALLOC NEW DECODERCTX");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    if(avcodec_parameters_to_context(*avCodecCtx,avCodecParameters) < 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("CAN NOT FILL DECODER");
            callJava->onCallError(CHILD_THREAD,1005,"CAN NOT FILL DECODER");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    //打开解码器上下文
    if(avcodec_open2(*avCodecCtx,avCodec,0) != 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("CAN NOT OPEN AUDIO STREAMS");
            callJava->onCallError(CHILD_THREAD,1006,"CAN NOT OPEN AUDIO STREAMS");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    return 0;
}


