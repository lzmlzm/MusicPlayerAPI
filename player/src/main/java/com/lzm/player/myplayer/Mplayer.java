package com.lzm.player.myplayer;

import com.lzm.player.TimeInfo;
import com.lzm.player.listener.MOnErrorListener;
import com.lzm.player.listener.MOnLoadListener;
import com.lzm.player.listener.MOnPauseResumeListener;
import com.lzm.player.listener.MOnPreparedListener;
import com.lzm.player.listener.MOnTimeInfoListener;
import com.lzm.player.log.mylog;
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
    private static boolean playNext = false;

    private MOnPreparedListener mOnPreparedListener;//引入接口
    private MOnLoadListener mOnLoadListener;
    private MOnPauseResumeListener mOnPauseResumeListener;
    private MOnTimeInfoListener mOnTimeInfoListener;
    private TimeInfo mtimeInfo;
    private MOnErrorListener mOnErrorListener;


    public  Mplayer(){}

    public void prepared() {

        if (TextUtils.isEmpty(source)) {
            mylog.d("source not be empty");
            return;
        }
        onCallLoad(true);
        //判断链接是否为空，否则就开一个线程
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_prepared(source);
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
    public void setSource(String source) {
        this.source = source;
    }

    public void playNext(String url)
    {
        source = url;
        playNext = true;
        stop();
    }

    //设置接口
    public void setmOnPreparedListener(MOnPreparedListener mOnPreparedListener) {
        this.mOnPreparedListener = mOnPreparedListener;
    }

    public void onCallPrepared()
    {
        if (mOnPreparedListener != null)
        {
            mOnPreparedListener.onPrepared();
        }
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

    public void onCallLoad(boolean load) {
        if (mOnPreparedListener != null) {
            mOnLoadListener.onLoad(load);
        }
    }

    public void setmOnTimeInfoListener(MOnTimeInfoListener mOnTimeInfoListener) {
        this.mOnTimeInfoListener = mOnTimeInfoListener;
    }

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


    public void onCallNext()
    {
        if(playNext)
        {
            playNext = false;
            prepared();
        }
    }


    private native void n_prepared(String source);

    private native void n_start();

    private native void n_pause();

    private native void n_reusme();

    private native void n_stop();

    private native void n_seek(int secs);


}
