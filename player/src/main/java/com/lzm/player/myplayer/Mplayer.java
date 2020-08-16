package com.lzm.player.myplayer;

import com.lzm.player.TimeInfo;
import com.lzm.player.listener.MOnErrorListener;
import com.lzm.player.listener.MOnLoadListener;
import com.lzm.player.listener.MOnPauseResumeListener;
import com.lzm.player.listener.MOnPreparedListener;
import com.lzm.player.listener.MOnTimeInfoListener;
import com.lzm.player.listener.MOnValueDBListener;
import com.lzm.player.log.mylog;
import com.lzm.player.muteenum.MuteEnum;

import android.bluetooth.le.ScanSettings;
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
    int defaultvolume = 60;

    private MOnPreparedListener mOnPreparedListener;//准备接口
    private MOnLoadListener mOnLoadListener;//加载接口
    private MOnPauseResumeListener mOnPauseResumeListener;//暂停恢复接口
    private MOnTimeInfoListener mOnTimeInfoListener;//时间信息接口
    private TimeInfo mtimeInfo;
    private MOnErrorListener mOnErrorListener; //出错接口
    private MOnValueDBListener mOnValueDBListener;//pcm 分贝接口





    public  Mplayer(){}

    //实现播放
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

    public void pause() {
        n_pause();
        if (mOnPauseResumeListener != null) {
            mOnPauseResumeListener.onPause(true);
        }
    }

    public void resume() {
        n_reusme();
        if (mOnPauseResumeListener != null) {
            mOnPauseResumeListener.onPause(false);
        }
    }

    public void stop()
    {
        mtimeInfo = null;
        duration = -1;//停止的时候还原值
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
            }
        }).start();
    }

    public void seek(int secs)
    {
        n_seek(secs);
    }

    public void setVolume(int percent)
    {
        //音量数值在0-100之间
        defaultvolume=percent;
        if(percent>=0 && percent<=100){
            n_volume(percent);
        }
    }
    //获取当前音量
    public int getCurrentVolume()
    {
        return  defaultvolume;
    }

    //设置声道,枚举传入
    public void setMute(MuteEnum muteEnum)
    {
        //获取枚举里的值
        n_mute(muteEnum.getValue());
    }
    public void setSource(String source) {
        this.source = source;
    }

    //获取播放时长
    public  int getDuaration()
    {
        if(duration < 0)
        {
            duration = n_duration();
        }
        return duration;
    }

    public void playNext(String url)
    {
        source = url;
        playNext = true;
        stop();
    }
    public void onCallPrepared()
    {
        if (mOnPreparedListener != null)
        {
            mOnPreparedListener.onPrepared();
        }
    }

    //设置回调接口
    public void setmOnPreparedListener(MOnPreparedListener mOnPreparedListener) {
        this.mOnPreparedListener = mOnPreparedListener;
    }

    public void setmOnLoadListener(MOnLoadListener mOnLoadListener) {
        this.mOnLoadListener = mOnLoadListener;
    }

    public void setmOnPauseResumeListener(MOnPauseResumeListener mOnPauseResumeListener) {
        this.mOnPauseResumeListener = mOnPauseResumeListener;
    }

    public void setmOnErrorListener(MOnErrorListener mOnErrorListener) {
        this.mOnErrorListener = mOnErrorListener;
    }

    public void setmOnTimeInfoListener(MOnTimeInfoListener mOnTimeInfoListener) {
        this.mOnTimeInfoListener = mOnTimeInfoListener;
    }

    public void setmOnValueDBListener(MOnValueDBListener mOnValueDBListener) {
        this.mOnValueDBListener = mOnValueDBListener;
    }

    //分贝接口回调
    public void onCallValueDB(int db)
    {
        if(mOnValueDBListener!=null)
        {
            mOnValueDBListener.onDbValue(db);
        }
    }

    //回调函数，此函数会在C++调用JAVA使用，并在C++层回传load参数
    public void onCallLoad(boolean load) {
        if (mOnPreparedListener != null) {
            mOnLoadListener.onLoad(load);
        }
    }
    //C++层调用此JAVA函数并回传time信息，然后此JAVA函数调用接口，
    // main函数里覆写接口函数onTimeInfo获得接口的数据
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

    public void onCallError(int code,String msg)
    {
        stop();
        if (mOnErrorListener != null){
            mOnErrorListener.OnError(code,msg);
        }
    }

    //切换下一个播放源
    public void onCallNext()
    {
        if(playNext)
        {
            playNext = false;
            prepared();
        }
    }

    //开始录音
    public void startRecord(File outfile) throws IOException {
        //初始化mediacodec
        if(n_samplerate()>0)
        {
            initMediaCodec(n_samplerate(),outfile);
        }
    }

    public void setPitch(float pitch)
    {
        n_pitch(pitch);
    }

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

    //mediacodec参数设置
    private MediaFormat encoderFormat = null;
    private MediaCodec mediaCodec = null;
    private MediaCodecInfo mediaCodecInfo = null;
    private FileOutputStream outputStream = null;
    private MediaCodec.BufferInfo bufferInfo = null;
    private int everypcmsize = 0;
    private byte[] outbytebuffer = null;
    private int aac_samplerate = 4;//代表44100采样率


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

            mediaCodec.start();

        }catch (IOException e){
            e.printStackTrace();
        }

    }

    /**
     * 编码Pcm为AAC,C++层调用
     * @param size
     * @param buffer
     */
    private void encodePcmToAAC(int size,byte[] buffer) throws IOException {
        if(buffer!=null && mediaCodec!=null){
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
