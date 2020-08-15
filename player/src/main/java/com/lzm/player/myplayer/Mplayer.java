package com.lzm.player.myplayer;

import com.lzm.player.TimeInfo;
import com.lzm.player.listener.MOnErrorListener;
import com.lzm.player.listener.MOnLoadListener;
import com.lzm.player.listener.MOnPauseResumeListener;
import com.lzm.player.listener.MOnPreparedListener;
import com.lzm.player.listener.MOnTimeInfoListener;
import com.lzm.player.log.mylog;
import com.lzm.player.muteenum.MuteEnum;

import android.text.TextUtils;

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
}
