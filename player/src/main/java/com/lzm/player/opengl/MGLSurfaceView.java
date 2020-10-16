package com.lzm.player.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;

import com.lzm.player.log.mylog;


//用于渲染视频播放器
public abstract class MGLSurfaceView extends GLSurfaceView {

    private MRender mRender;

    public MGLSurfaceView(Context context) {
        super(context);
    }

    public MGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);

        setEGLContextClientVersion(2);
        mRender = new MRender(context);
        setRenderer(mRender);
        //RENDERMODE_WHEN_DIRTY 一帧一帧渲染
        //RENDERMODE_CONTINUOUSLY 一直渲染
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        //监听
        mRender.setOnRenderListener(new MRender.OnRenderListener() {
            @Override
            public void onRender() {
                mylog.d("request render");
                requestRender();
            }
        });

    }


    public void setYUVData(int width, int height, byte[] y,  byte[] u,  byte[] v)
    {
        if(mRender!=null)
        {
            mRender.setYUVData(width,height,y,u,v);
            //设置数据后请求渲染
            requestRender();
        }
    }


    public MRender getmRender() {
        return mRender;
    }
}
