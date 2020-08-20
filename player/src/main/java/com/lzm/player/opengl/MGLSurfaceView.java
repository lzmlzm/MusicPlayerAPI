package com.lzm.player.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;

public class MGLSurfaceView extends GLSurfaceView {

    private MRender mRender;

    public MGLSurfaceView(Context context) {
        this(context,null);
    }

    public MGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);

        setEGLContextClientVersion(2);
        mRender = new MRender(context);
        setRenderer(mRender);
        //RENDERMODE_WHEN_DIRTY 一帧一帧渲染
        //RENDERMODE_CONTINUOUSLY 一直渲染
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public void setYUVData(int width, int height, byte[] y,  byte[] u,  byte[] v)
    {
        if(mRender!=null)
        {
            mRender.setYUVData(width,height,y,u,v);
            //设置数据后请求渲染
            requestRender();
            Log.d("TAG", "setYUVData: succsee");
        }
    }
}
