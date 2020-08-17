package com.lzm.player.myplayer;

import com.lzm.player.TimeInfo;
import com.lzm.player.listener.MOnCompleteListener;
import com.lzm.player.listener.MOnErrorListener;
import com.lzm.player.listener.MOnLoadListener;
import com.lzm.player.listener.MOnPauseResumeListener;
import com.lzm.player.listener.MOnPcmInfoListener;
import com.lzm.player.listener.MOnPreparedListener;
import com.lzm.player.listener.MOnRecordTimeListener;
import com.lzm.player.listener.MOnTimeInfoListener;
import com.lzm.player.listener.MOnValueDBListener;
import com.lzm.player.log.mylog;
import com.lzm.player.muteenum.MuteEnum;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.text.TextUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * 版权：麻瓜 版权所有
 *
 * @author ${LZM}
 * 版本：1.0
 * 创建日期：19-2-1715
 * qq邮箱：1198492751@qq.com
 */
public class Mplayer {
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swscale");
    }

    private static String source;
    private static int duration = -1;
    private static boolean playNext = false;
    private static  boolean initMediacodec = false;
    int defaultvolume = 60;

    private MOnPreparedListener mOnPreparedListener;//准备接口
    private MOnLoadListener mOnLoadListener;//加载接口
    private MOnPauseResumeListener mOnPauseResumeListener;//暂停恢复接口
    private MOnTimeInfoListener mOnTimeInfoListener;//时间信息接口
    private TimeInfo mtimeInfo;
    private MOnErrorListener mOnErrorListener; //出错接口
    private MOnValueDBListener mOnValueDBListener;//pcm 分贝接口
    private MOnRecordTimeListener mOnRecordTimeListener;//录音时间接口
    private MOnCompleteListener mOnCompleteListener;
    private MOnPcmInfoListener mOnPcmInfoListener;

    public  Mplayer(){}

    //实现播放

    /**
     * 准备播放资源
     */
    public void prepared() {
        //判断链接是否为空
        if (TextUtils.isEmpty(source)) {
            mylog.d("source not be empty");
            return;
        }
        //回调
        onCallLoad(true);

        new Thread(new Runnable() {
            @Override
            public void run() {
                n_prepared(source);//启动C++层的播放
            }
        }).start();
    }

    /**
     * 开启播放线程
     */
    public void start()
    {
        if(TextUtils.isEmpty(source))
        {
            mylog.d("source not be empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_start();
            }
        }).start();
    }

    /**
     * 暂停播放
     */
    public void pause() {
        n_pause();
        if (mOnPauseResumeListener != null) {
            mOnPauseResumeListener.onPause(true);
        }
    }

    /**
     * 恢复播放
     */
    public void resume() {
        n_reusme();
        if (mOnPauseResumeListener != null) {
            mOnPauseResumeListener.onPause(false);
        }
    }

    /**
     * 播放停止
     */
    public void stop()
    {
        mtimeInfo = null;
        duration = -1;//停止的时候还原值
        stopRecord();
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
            }
        }).start();
    }

    /**
     * seek功能（需要改进精确性）
     * @param secs
     */
    public void seek(int secs)
    {
        n_seek(secs);
    }

    /**
     * 设置播放音量
     * @param percent
     */
    public void setVolume(int percent)
    {
        //音量数值在0-100之间
        defaultvolume=percent;
        if(percent>=0 && percent<=100){
            n_volume(percent);
        }
    }

    /**
     * 获取当前音量
     * @return
     */
    public int getCurrentVolume()
    {
        return  defaultvolume;
    }

    /**
     * 设置播放的左右声道、枚举传入
     * @param muteEnum
     */
    public void setMute(MuteEnum muteEnum)
    {
        //获取枚举里的值
        n_mute(muteEnum.getValue());
    }

    /**
     * 设置播放资源
     * @param source
     */
    public void setSource(String source) {
        this.source = source;
    }

    /**
     * 获取播放时长
     * @return 音视频持续时间
     */
    public  int getDuaration()
    {
        if(duration < 0)
        {
            duration = n_duration();
        }
        return duration;
    }

    /**
     * 播放下一个资源
     * @param url
     */
    public void playNext(String url)
    {
        source = url;
        playNext = true;
        stop();
    }

    /**
     * 设置资源准备监听
     * @param mOnPreparedListener
     */
    //设置回调接口
    public void setmOnPreparedListener(MOnPreparedListener mOnPreparedListener) {
        this.mOnPreparedListener = mOnPreparedListener;
    }

    /**
     * 设置加载状态监听
     * @param mOnLoadListener
     */
    public void setmOnLoadListener(MOnLoadListener mOnLoadListener) {
        this.mOnLoadListener = mOnLoadListener;
    }

    /**
     * 设置播放暂停、恢复监听
     * @param mOnPauseResumeListener
     */
    public void setmOnPauseResumeListener(MOnPauseResumeListener mOnPauseResumeListener) {
        this.mOnPauseResumeListener = mOnPauseResumeListener;
    }

    /**
     * 设置出错监听
     * @param mOnErrorListener
     */
    public void setmOnErrorListener(MOnErrorListener mOnErrorListener) {
        this.mOnErrorListener = mOnErrorListener;
    }

    /**
     * 设置时间信息监听
     * @param mOnTimeInfoListener
     */
    public void setmOnTimeInfoListener(MOnTimeInfoListener mOnTimeInfoListener) {
        this.mOnTimeInfoListener = mOnTimeInfoListener;
    }

    /**
     * 设置分贝监听
     * @param mOnValueDBListener
     */
    public void setmOnValueDBListener(MOnValueDBListener mOnValueDBListener) {
        this.mOnValueDBListener = mOnValueDBListener;
    }

    /**
     * 录音时间监听
     * @param mOnRecordTimeListener
     */
    public void setmOnRecordTimeListener(MOnRecordTimeListener mOnRecordTimeListener){
        this.mOnRecordTimeListener = mOnRecordTimeListener;
    }

    /**
     * 完成监听
     * @param mOnCompleteListener
     */
    public void setmOnCompleteListener(MOnCompleteListener mOnCompleteListener) {
        this.mOnCompleteListener = mOnCompleteListener;
    }

    /**
     * PCM返回监听
     * @param mOnPcmInfoListener
     */
    public void setmOnPcmInfoListener(MOnPcmInfoListener mOnPcmInfoListener) {
        this.mOnPcmInfoListener = mOnPcmInfoListener;
    }

    /**
     * 资源准备回调
     */
    public void onCallPrepared()
    {
        if (mOnPreparedListener != null)
        {
            mOnPreparedListener.onPrepared();
        }
    }

    /**
     * 分贝接口回调
     * @param db
     */
    public void onCallValueDB(int db)
    {
        if(mOnValueDBListener!=null)
        {
            mOnValueDBListener.onDbValue(db);
        }
    }

    //回调函数，此函数会在C++调用JAVA使用，并在C++层回传load参数
    /**
     * C++层回传load参数
     * @param load
     */
    public void onCallLoad(boolean load) {
        if (mOnPreparedListener != null) {
            mOnLoadListener.onLoad(load);
        }
    }


    //C++层调用此JAVA函数并回传time信息，然后此JAVA函数调用接口，
    // main函数里覆写接口函数onTimeInfo获得接口的数据
    /**
     * 回传time信息
     * @param currentTime
     * @param totalTime
     */
    public void onCallTimeInfo(int currentTime, int totalTime) {
        if (mOnTimeInfoListener != null) {
            if (mtimeInfo == null) {
                mtimeInfo = new TimeInfo();
            }

            mtimeInfo.setCurrentTime(currentTime);
            mtimeInfo.setTotalTime(totalTime);
            mOnTimeInfoListener.onTimeInfo(mtimeInfo);
        }
    }

    /**
     * C++层出错回调
     * @param code
     * @param msg
     */
    public void onCallError(int code,String msg)
    {
        stop();
        if (mOnErrorListener != null){
            mOnErrorListener.OnError(code,msg);
        }
    }

    /**
     * 切换下一个播放源
     */
    public void onCallNext()
    {
        if(playNext)
        {
            playNext = false;
            prepared();
        }
    }

    /**
     * 监听结束状态
     */
    public void onCallComplete()
    {
        stop();
        if(mOnCompleteListener==null)
        {
            mOnCompleteListener.onCallComplete();
        }
    }

    /**
     * 监听PCM返回回调
     * @param retPcmBuffer
     * @param retPcmBufferSize
     */
    public void onCallPcmInfo(byte[] retPcmBuffer, int retPcmBufferSize)
    {
        if(mOnPcmInfoListener!=null)
        {
            mOnPcmInfoListener.onPcmInfo(retPcmBuffer,retPcmBufferSize);
        }
    }

    public void onCallPcmRate(int samplerate)
    {
        if(mOnPcmInfoListener!=null)
        {
            mOnPcmInfoListener.onPcmRate(samplerate,16,2);
        }
    }
    /**
     * 开始录音
     * @param outfile
     * @throws IOException
     */
    public void startRecord(File outfile) throws IOException {
        //初始化mediacodec
        if(!initMediacodec)
        {
            //获取采样率
            audioSamplerate = n_samplerate();
            if(audioSamplerate>0)
            {
                initMediacodec = true;
                initMediaCodec(audioSamplerate,outfile);
                n_record(true);
            }
        }

    }

    /**
     * 停止播放
     */
    public void stopRecord()
    {
        if (initMediacodec)
        {
            n_record(false);
            try {

                //停止释放资源
                releaseMediacodec();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

    }

    /**
     * 暂停录音
     */
    public void pauseRecord()
    {
        //暂停不释放资源
        n_record(false);
    }

    /**
     * 音频裁减
     * @param starttime
     * @param endtime
     * @param returnPcm
     */
    public void cutAudio(int starttime,int endtime ,boolean returnPcm)
    {
        if(n_cutaudio(starttime,endtime,returnPcm))
        {
            start();
        }else
        {
            stop();
            onCallError(2001,"cutaudio error!");
        }
    }

    /**
     * 继续录音
     */
    public void resumeRecord()
    {
        n_record(true);
    }

    /**
     * 设置音调
     * @param pitch
     */
    public void setPitch(float pitch)
    {
        n_pitch(pitch);
    }

    /**
     * 设置倍速
     * @param speed
     */
    public void setSpeed(float speed)
    {
        n_speed(speed);
    }

    //解码音频
    private native void n_prepared(String source);

    private native void n_start();

    private native void n_pause();

    private native void n_reusme();

    private native void n_stop();

    private native void n_seek(int secs);

    private native int n_duration();

    private native void n_volume(int percent);

    private native void n_mute(int mute);

    private native void n_pitch(float pitch);

    private native void n_speed(float speed);

    private native int n_samplerate();

    private native void n_record(boolean start);

    private native  boolean n_cutaudio(int start_time,int end_time,boolean returnpcm);

    //mediacodec参数设置
    private MediaFormat encoderFormat = null;
    private MediaCodec mediaCodec = null;
    private MediaCodecInfo mediaCodecInfo = null;
    private FileOutputStream outputStream = null;
    private MediaCodec.BufferInfo bufferInfo = null;
    private int everypcmsize = 0;
    private byte[] outbytebuffer = null;
    private int aac_samplerate = 4;//代表44100采样率
    private double recordTime = 0;//录音时间
    private int audioSamplerate = 0;

    /**
     * mediacodec初始化
     * @param samplerate
     * @param outfile
     */
    private void initMediaCodec(int samplerate, File outfile) throws IOException
    {
        try
        {
            aac_samplerate = getADTSsamplerate(samplerate);
            //创建AAC格式
            encoderFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC,aac_samplerate,2);

            //设置码率
            encoderFormat.setInteger(MediaFormat.KEY_BIT_RATE,96000);
            //LC头部信息里
            encoderFormat.setInteger(MediaFormat.KEY_AAC_PROFILE,MediaCodecInfo.CodecProfileLevel.AACObjectLC);

            encoderFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE,4096);

            //创建音频编码器
            mediaCodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
            //info 编解码视频音频信息
            bufferInfo =new MediaCodec.BufferInfo();

            if(mediaCodec==null)
            {
                mylog.d("create aac codec failed!");
                return;
            }
            //配置codec
            mediaCodec.configure(encoderFormat,null,null,MediaCodec.CONFIGURE_FLAG_ENCODE);

            //初始化输出流
            outputStream=new FileOutputStream(outfile);

            //录音时间置0
            recordTime = 0;

            mediaCodec.start();

        }catch (IOException e){
            e.printStackTrace();
        }

    }

    /**
     * mediacodec释放
     */
    private void releaseMediacodec() throws IOException {
        if(mediaCodec==null)
        {
            return;
        }
        outputStream.close();
        outputStream = null;

        recordTime = 0;
        mediaCodec.stop();
        mediaCodec.release();
        mediaCodec = null;
        encoderFormat = null;
        mediaCodecInfo = null;
        initMediacodec = false;
        mylog.d("mediacodec release录制完成");
    }

    /**
     * 编码Pcm为AAC,C++层调用
     * @param size
     * @param buffer
     */
    private void encodePcmToAAC(int size,byte[] buffer) throws IOException {
        if(buffer!=null && mediaCodec!=null)
        {
            recordTime += size*1.0f/(audioSamplerate*2*2);//两通道，16bit/8bit，8bit一个字节
            if(mOnRecordTimeListener!=null)
            {
                mOnRecordTimeListener.onRecordTime(recordTime);
            }

            //从队列里得到当前可用buffer空间的索引
            int inputBufferindex = mediaCodec.dequeueInputBuffer(0);
            if (inputBufferindex > 0)
            {
                //根据索引找到bytebuffer
                ByteBuffer byteBuffer = mediaCodec.getInputBuffer(inputBufferindex);
                byteBuffer.clear();
                //将输入的buffer放入缓冲区
                byteBuffer.put(buffer);
                //入队
                mediaCodec.queueInputBuffer(inputBufferindex,0,size,0,0);
            }


            int outbufferindex = mediaCodec.dequeueOutputBuffer(bufferInfo,0);
            while (outbufferindex > 0)
            {

                try {
                    //加入头部后每个pcm的大小
                    everypcmsize = bufferInfo.size +7;
                    outbytebuffer = new byte[everypcmsize];

                    //取出编码好的队列数据
                    ByteBuffer byteBuffer = mediaCodec.getOutputBuffer(outbufferindex);
                    byteBuffer.position(bufferInfo.offset);//起始位置
                    byteBuffer.limit(bufferInfo.size+bufferInfo.offset);//限制大小

                    //adts
                    addADTStoPacket(outbytebuffer,everypcmsize,aac_samplerate);

                    //17字节的bytebuffer，将bytebuffer放入outbytebuffer的第7字节到结束。前7位不放，放ADTS信息
                    byteBuffer.get(outbytebuffer,7,bufferInfo.size);
                    byteBuffer.position(bufferInfo.offset);
                    //写入文件
                    outputStream.write(outbytebuffer,0,everypcmsize);

                    //释放缓冲区
                    mediaCodec.releaseOutputBuffer(outbufferindex,0);
                    outbufferindex = mediaCodec.dequeueOutputBuffer(bufferInfo,0);
                    outbytebuffer=null;

                }catch (IOException e){
                    e.printStackTrace();
                }
            }
        }
    }


    /**
     * 添加ADTS头部信息
     * @param packet
     * @param packetLen
     * @param sample_rate
     */
    private void addADTStoPacket(byte[] packet, int packetLen, int sample_rate) {
        int profile = 2; // AAC LC
        int freqIdx = sample_rate; // 采样率
        int chanCfg = 2; // CPE

        // fill in ADTS data
        packet[0] = (byte) 0xFF;
        packet[1] = (byte) 0xF9;
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;

    }

    private int getADTSsamplerate(int sample_rate)
    {
        int rate = 0;
        switch (sample_rate)
        {
            case 96000:
                rate = 0;
                break;
            case 88200:
                rate = 1;
                break;
            case 64000:
                rate = 2;
                break;
            case 48000:
                rate = 3;
                break;
            case 44100:
                rate = 4;
                break;
            case 32000:
                rate = 5;
                break;
            case 24000:
                rate = 6;
                break;
            case 22050:
                rate = 7;
                break;
            case 16000:
                rate = 8;
                break;
            case 12000:
                rate = 9;
                break;
            case 11025:
                rate = 10;
                break;
            case 8000:
                rate = 11;
                break;
            case 7350:
                rate = 12;
                break;
        }
        return rate;
    }


}
