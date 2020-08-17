
// Created by lzm on 19-2-20.
//

#include <libavutil/time.h>
#include "MAudio.h"

MAudio::MAudio(MPlaystatus *mPlaystatus, int samplerate, MCallJava *CallJava) {

    this->callJava = CallJava;
    this->mPlaystatus = mPlaystatus;
    this->sample_rate = samplerate;
    queue = new MQueue(mPlaystatus);
    buffer = static_cast<uint8_t *>(av_malloc(sample_rate * 2 * 2));

    samplebuffer = static_cast<SAMPLETYPE *>(malloc(sample_rate*2*2));
    soundTouch = new SoundTouch();

    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);

    //音调设置
    soundTouch->setPitch(this->pitch);
    //速度设置
    soundTouch->setTempo(this->speed);
}

MAudio::~MAudio() {
    ////

}

/**
 * 解码函数
 * @param data
 * @return
 */
void *decodeplay(void *data)
{
    MAudio *mAudio = static_cast<MAudio *>(data);

    //创建音频播放器进入音频播放状态
    mAudio->initOpenSLES();

    pthread_exit(&mAudio->pthread_play);
}

/**
 * 开启播放线程
 */
void MAudio::play() {

    //创建音频播放器线程
    pthread_create(&pthread_play, NULL, decodeplay, this);

}

/**
 * 音频重采样
 * @param pcmbuffer
 * @return
 */
int MAudio::resampleAudio(void **pcmbuffer) {

    data_size=0;
    while (mPlaystatus != NULL && !mPlaystatus->exit)
    {
        if(mPlaystatus->seek)
        {
            av_usleep(1000*100);
            continue;
        }
        if (queue->getQueueSIze() == 0)
        {
            if (!mPlaystatus->load)
            {
                mPlaystatus->load = true;
                callJava->onCallLoad(CHILD_THREAD, true);
            }
            av_usleep(1000*100);
            continue;
        } else {

            if (mPlaystatus->load) {
                mPlaystatus->load = false;
                callJava->onCallLoad(CHILD_THREAD, false);
            }
        }

        avPacket = av_packet_alloc();//分配空间
        if(queue->outAvpacket(avPacket) != 0)
        {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;//释放内存
            continue;
        }

        ret = avcodec_send_packet(avCodecCtx,avPacket);//将PACKET放到解码器进行解码
        if(ret != 0)
        {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;//释放内存
            continue;
        }
        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecCtx,avFrame);//接受数据
        if(ret == 0)
        {
            //success
            //对声音进行重采样
            if(avFrame->channels>0 && avFrame->channel_layout==0)
            {
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            }
            else if(avFrame->channels==0 && avFrame->channel_layout>0)
            {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }


            SwrContext *swr_ctx= NULL;
            swr_ctx = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    static_cast<AVSampleFormat >(avFrame->format),
                    avFrame->sample_rate,
                    NULL,
                    NULL);
            if(!swr_ctx || swr_init(swr_ctx) <0)
            {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;//释放内存
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                if(swr_ctx != NULL)
                {
                    swr_free(&swr_ctx);
                    swr_ctx = NULL;
                }
                continue;
            }
            //重采样获取pcm的buffer
            nb= swr_convert(swr_ctx, &buffer,
                    avFrame->nb_samples,
                    (const uint8_t **)(avFrame->data),
                    avFrame->nb_samples);

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);//通过实际布局求出输出声道数

            //计算并返回buffersize
            data_size = nb*out_channels*av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            //根据PTS获取当前AVFRAME的时间
            now_time = avFrame->pts * av_q2d(time_base);
            if (now_time < clock) {
                now_time = clock;
            }
            clock = now_time;//设置时钟为当前时间

            if(LOG_DEBUG)
            {
                LOGE("DATA SIZE IS :%d",data_size);
            }

            //传人pcmbuffer传给Sountouch
            *pcmbuffer = buffer;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;//释放内存
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
            swr_ctx = NULL;
            break;

        }else{
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;//释放内存
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }


    }
    return data_size;
}

//通过SL缓冲队列回调，将PCM数据入队
/**
 * opensl es缓冲队列回调，将PCM数据入队
 * @param bf
 * @param context
 */
void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {

    MAudio *mAudio = (MAudio *)(context);
    if(mAudio!=NULL)
    {
        //重采样获得PCM buffer数据和大小
        //resampleAudio将解码出来的数据写入buffer->sountouch
        int buffersize = mAudio->getSoundTouchdata();
        if (buffersize > 0)
        {
            //设置播放时长：PCM时间数据大小/每秒理论PCM大小
            mAudio->clock += buffersize / ((double) (mAudio->sample_rate * 2 * 2));

            if (mAudio->clock - mAudio->last_time >= 0.1) {
                mAudio->last_time = mAudio->clock;
                //回调播放时长信息
                mAudio->callJava->onCallTimeInfo(CHILD_THREAD, mAudio->clock, mAudio->duration);
            }

            //根据录音状态处理pcm
            if(mAudio->isRecordpcm)
            {
                //将pcm buffer传给java处理为aac
                mAudio->callJava->onCallPcmToAAC(CHILD_THREAD,buffersize*4,mAudio->samplebuffer);
            }

            //回调分贝值给JAVA
            mAudio->callJava->onCallValueDB(CHILD_THREAD,
                    mAudio->getPcmdb(reinterpret_cast<char *>(mAudio->samplebuffer), buffersize * 4));
            //将重采样的soundtouch处理的pcm buffer数据入队
            (*mAudio->pcmBufferQueue)->Enqueue(mAudio->pcmBufferQueue,(char *)mAudio->samplebuffer,buffersize*2*2);
        }


    }
}

/**
 * 初始化opensl es
 */
void MAudio::initOpenSLES() {
    SLresult result;
    //OpenSLES初始化

    //1.创建引擎接口
    result=slCreateEngine(&engineObjectItf,0,0,0,0,0);
    //2.实例化接口并且获得引擎
    result=(*engineObjectItf)->Realize(engineObjectItf,SL_BOOLEAN_FALSE);

    result=(*engineObjectItf)->GetInterface(engineObjectItf,SL_IID_ENGINE,&engineEngine);//获得实际得到的Engine

    //3.使用引擎创建混音器
    const SLInterfaceID  mids[1]= {SL_IID_ENVIRONMENTALREVERB};

    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};

    result=(*engineEngine)->CreateOutputMix(engineEngine,&mixouputObject,1,mids,mreq);
    (void)result;
    result=(*mixouputObject)->Realize(mixouputObject,SL_BOOLEAN_FALSE);
    (void)result;
    result=(*mixouputObject)->GetInterface(mixouputObject,SL_IID_ENVIRONMENTALREVERB,&outptmixEnvironmentalReverb);

    if(SL_RESULT_SUCCESS == result) {
        result = (*outptmixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outptmixEnvironmentalReverb, &reverbSettings);//设置属性
        (void)result;
    }

    //4.创建播放器或者录音器

    //创建android buffer队列
    SLDataLocator_AndroidBufferQueue androidBufferQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};

    SLDataFormat_PCM pcm = {

            //设置PCM数据类型
            SL_DATAFORMAT_PCM,
            2,
            static_cast<SLuint32>(getCurrentSampleRateForOpenSLES(sample_rate)),//采样率
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//左右声道
            SL_BYTEORDER_LITTLEENDIAN//数据存放类型

    };

    //数据源为buffer队列和数据源的类型为PCM
    SLDataSource slDataSource = {&androidBufferQueue,&pcm};

    //播放器实现的功能
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX,mixouputObject};

    SLDataSink audioSink = {&outputMix,NULL,};

    //必须加上SL_IID_VOLUME ID否则无法控制音量
    const SLInterfaceID ids[4]={SL_IID_BUFFERQUEUE,SL_IID_VOLUME,SL_IID_MUTESOLO,SL_IID_PLAYBACKRATE};

    const SLboolean req[4] = {SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};

    (*engineEngine)->CreateAudioPlayer(engineEngine,&pcmplayer,&slDataSource, &audioSink,4,ids,req);

    (*pcmplayer)->Realize(pcmplayer,SL_BOOLEAN_FALSE);

    (*pcmplayer)->GetInterface(pcmplayer,SL_IID_PLAY,&slPlayItf);

    //获取音量控制接口
    (*pcmplayer)->GetInterface(pcmplayer,SL_IID_VOLUME,&slVolumeItf);
    //获取声道控制接口
    (*pcmplayer)->GetInterface(pcmplayer,SL_IID_MUTESOLO,&slMuteSoloItf);

    //5.设置缓冲区和回调函数
    (*pcmplayer)->GetInterface(pcmplayer,SL_IID_BUFFERQUEUE,&pcmBufferQueue);
    //设置默认音量
    setVolume(defaultvolume);

    //pcmBufferCallBack将实际的PCM数据放入pcmBufferQueue
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue,pcmBufferCallBack,this);

    //6.设置播放状态
    (*slPlayItf)->SetPlayState(slPlayItf,SL_PLAYSTATE_PLAYING);

    pcmBufferCallBack(pcmBufferQueue,this);

}

/**
 * 获取采样率
 * @param sample_rate
 * @return
 */
int MAudio::getCurrentSampleRateForOpenSLES(int sample_rate) {

    int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}
/**
 * 设置暂停状态
 */
void MAudio::pause() {
    if (pcmplayer != NULL) {
        (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PAUSED);
    }
}
/**
 * 设置恢复播放状态
 */
void MAudio::resume() {
    if (pcmplayer != NULL) {
        (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);
    }
}
/**
 * 设置停止状态
 */
void MAudio::stop() {
    if (pcmplayer != NULL) {
        (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_STOPPED);
    }
}


/**
 * 释放内存
 */
void MAudio::release() {

    stop();
    if(queue != NULL)
    {
        delete(queue);
        queue = NULL;
    }

    /*释放OPENSLES*/
    if(pcmplayer != NULL)
    {
        (*pcmplayer)->Destroy(pcmplayer);
        pcmplayer = NULL;
        slPlayItf = NULL;
        pcmBufferQueue = NULL;
        slMuteSoloItf = NULL;
        slVolumeItf = NULL;
    }

    /*1释放混音器*/
    if(mixouputObject != NULL)
    {
        (*mixouputObject)->Destroy(mixouputObject);
        mixouputObject = NULL;
        outptmixEnvironmentalReverb = NULL;
    }

    /*2释放引擎*/
    if(engineObjectItf != NULL)
    {
        (*engineObjectItf)->Destroy(engineObjectItf);
        engineObjectItf = NULL;
        engineEngine = NULL;
    }

    if(buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }

    if(outbuffer!=NULL)
    {
        outbuffer=NULL;
    }
    if(soundTouch==NULL)
    {
        delete(soundTouch);
        soundTouch=NULL;
    }

    if(samplebuffer!=NULL)
    {
        free(samplebuffer);//释放malloc的空间
        samplebuffer=NULL;
    }
    if(avCodecCtx != NULL)
    {
        avcodec_close(avCodecCtx);
        avcodec_free_context(&avCodecCtx);
        avCodecCtx = NULL;
    }

    if(mPlaystatus != NULL);
    {
        mPlaystatus = NULL;
    }

    if(callJava != NULL)
    {
        callJava = NULL;
    }
}


/**
 * 设置音量
 * @param percent
 */
void MAudio::setVolume(int percent) {
    if(slVolumeItf!=NULL)
    {   //音量分段处理使得音量大小过渡流畅

        //需要回传当前音量给JAVA，在界面上显示。
        defaultvolume = percent;
        //传入pcm播放器引擎对象
        if(percent >30)
        {
            (*slVolumeItf)->SetVolumeLevel(slVolumeItf, (100-percent)*-20);
        }else if(percent >25)
        {
            (*slVolumeItf)->SetVolumeLevel(slVolumeItf, (100-percent)*-22);
        }else if(percent >20)
        {
            (*slVolumeItf)->SetVolumeLevel(slVolumeItf, (100-percent)*-25);
        }else if(percent >15)
        {
            (*slVolumeItf)->SetVolumeLevel(slVolumeItf, (100-percent)*-28);
        }else if(percent >10)
        {
            (*slVolumeItf)->SetVolumeLevel(slVolumeItf, (100-percent)*-30);
        }else if(percent >5)
        {
            (*slVolumeItf)->SetVolumeLevel(slVolumeItf, (100-percent)*-37);
        }else if(percent >3)
        {
            (*slVolumeItf)->SetVolumeLevel(slVolumeItf, (100-percent)*-40);
        }else if(percent >0)
        {
            (*slVolumeItf)->SetVolumeLevel(slVolumeItf, (100-percent)*-100);
        }

    }

}

/**
 * 设置左右声道
 * @param mute
 */
void MAudio::setMute(int mute) {
    //判断接口存不存在
    if(slMuteSoloItf!=NULL)
    {
        LOGD("AUDIO 收到")
        if(mute==0)
        {
            //right
            LOGD("右声道开启")
            (*slMuteSoloItf)->SetChannelMute(slMuteSoloItf,1, false);//关闭左声道
            (*slMuteSoloItf)->SetChannelMute(slMuteSoloItf,0, true);//开启右声道
        }else if(mute==1)
        {
            //left
            (*slMuteSoloItf)->SetChannelMute(slMuteSoloItf,1, true);//开启左声道
            (*slMuteSoloItf)->SetChannelMute(slMuteSoloItf,0, false);//关闭右声道
        }else if(mute==2)
        {
            //threed
            (*slMuteSoloItf)->SetChannelMute(slMuteSoloItf,1, false);//开启左声道
            (*slMuteSoloItf)->SetChannelMute(slMuteSoloItf,0, false);//开启右声道
        }
    }

}


/**
 * 获取soundtouch采样后的数据
 * @return 采样点个数
 */
int MAudio::getSoundTouchdata() {

    //确保每次调用缓冲区为空
    outbuffer=NULL;
    //只有程序运行时有效
    while (mPlaystatus != NULL && !mPlaystatus->exit)
    {
        if(isSoundTouchEnd)
        {
            isSoundTouchEnd = false;
            data_size = resampleAudio(reinterpret_cast<void **>(&outbuffer));

            if(data_size>0)
            {
                //pcm 8bit to 16bit
                for(int i=0;i < data_size/2 + 1;i++)
                {
                    //pcm第二个8位数据填充到16位的后8位
                    samplebuffer[i]=(outbuffer[i*2] | ((outbuffer[2*i + 1])<<8));
                }
                //传入实际采样个数
                soundTouch->putSamples(samplebuffer,nb);
                //返回处理后的采样个数
                soudTouchnum=soundTouch->receiveSamples(samplebuffer,data_size/4);
            }else{
                //=0 数据读取完，调用输出
                soundTouch->flush();
            }
        }

        if(soudTouchnum==0)
        {
            isSoundTouchEnd = true;
            continue;
        } else{
            if(outbuffer==NULL)
            {
                soudTouchnum = soundTouch->receiveSamples(samplebuffer,data_size/4);
                if(soudTouchnum==0)
                {
                    isSoundTouchEnd = true;
                    continue;
                }
            }
            return soudTouchnum;
        }
    }
    return 0;
}

/**
 * 设置播放音频的音调
 */
void MAudio::setPitch(float pitch) {

    this->pitch = pitch;
    if(soundTouch!=NULL)
    {
        soundTouch->setPitch(this->pitch);
    }

}

/**
 * 设置播放音频的速度
 * @param speed
 */
void MAudio::setSpeed(float speed) {

    this->speed = speed;
    if(soundTouch!=NULL)
    {
        soundTouch->setTempo(this->speed);
    }
}

/**
 * 计算分贝
 * @param pcmdata
 * @param pcmsize
 * @return 分贝
 */
int MAudio::getPcmdb(char *pcmdata, size_t pcmsize) {
    int db = 0;
    short int pervalue = 0;//每一帧的分贝值
    double sum = 0;//分贝总和

    for(int i=0;i<pcmsize;i += 2)
    {
        //取pcmdata中的两个值给value
        memcpy(&pervalue,pcmdata+i,2);
        sum  += abs(pervalue);
    }
    //由于每次取两个，所以/2
    sum = sum / (pcmsize/2);
    if(sum>0)
    {
        //分贝计算公式
        db = (int)20*log10(sum);
    }
    return db;
}

/**
 * audio层设置录音状态
 * @param flags
 */
void MAudio::setRecordStatus(bool flags) {
    this->isRecordpcm = flags;
}

