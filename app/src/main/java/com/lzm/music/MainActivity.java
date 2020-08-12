package com.lzm.music;

import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
;
import com.lzm.player.TimeInfo;
import com.lzm.player.listener.MOnErrorListener;
import com.lzm.player.listener.MOnLoadListener;
import com.lzm.player.listener.MOnPauseResumeListener;
import com.lzm.player.listener.MOnPreparedListener;
import com.lzm.player.listener.MOnTimeInfoListener;
import com.lzm.player.myplayer.*;
import com.lzm.player.log.mylog;
import com.lzm.player.util.MTimeUtil;

public class MainActivity extends AppCompatActivity {



    private Mplayer mplayer;
    private TextView tvTime;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tvTime = findViewById(R.id.tv_time);
        mplayer = new Mplayer();
        mplayer.setmOnPreparedListener(new MOnPreparedListener() {
            @Override
            public void onPrepared() {
                mylog.d("onPrepared");
                mplayer.start();
            }
        });
        mplayer.setmOnLoadListener(new MOnLoadListener() {
            @Override
            public void onLoad(boolean load) {
                if (load) {
                    mylog.d("OnLoading......");
                } else {
                    mylog.d("playing......");
                }
            }
        });

        mplayer.setmOnPauseResumeListener(new MOnPauseResumeListener() {
            @Override
            public void onPause(boolean pause) {
                if (pause) {
                    mylog.d("pausing......");

                } else {
                    mylog.d("playing......");
                }
            }
        });

        mplayer.setmOnTimeInfoListener(new MOnTimeInfoListener() {
            @Override
            public void onTimeInfo(TimeInfo timeInfo) {
                Message message = Message.obtain();
                message.what = 1;
                message.obj = timeInfo;
                handler.sendMessage(message);
            }
        });

        mplayer.setmOnErrorListener(new MOnErrorListener() {
            @Override
            public void OnError(int code, String msg) {
                mylog.d("code:"+code+",msg"+msg);
            }
        });
    }




    public void prepared(View view) {

    }
    public void begin(View view) {
        mplayer.setSource("/sdcard/netease/cloudmusic/Music/TheFatRat - Unity.m4a");
        mplayer.prepared();
    }

    public void pause(View view) {
        mplayer.pause();
    }

    public void resume(View view) {
        mplayer.resume();
    }

    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (msg.what == 1) {
                TimeInfo timeInfo = (TimeInfo) msg.obj;
                tvTime.setText(MTimeUtil.secdsToDateFormat(timeInfo.getTotalTime(), timeInfo.getTotalTime())
                        + "/" + MTimeUtil.secdsToDateFormat(timeInfo.getCurrentTime(), timeInfo.getTotalTime()));
            }
        }
    };

    public void stop(View view) {
        mplayer.stop();
    }

    public void seek(View view) {
        mplayer.seek(200);
    }

    public void next(View view) {
        mplayer.playNext("/sdcard/netease/cloudmusic/Music/蔡健雅 - 红色高跟鞋.mp3");
    }
}
