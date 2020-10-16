package com.lzm.player.opengl;

import android.content.Context;
import android.opengl.GLES20;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.lang.ref.WeakReference;

import javax.microedition.khronos.egl.EGLContext;

//用于渲染摄像头数据

public abstract class LEGLSurfaceView extends SurfaceView implements SurfaceHolder.Callback {

    private Surface surface;
    private EGLContext eglContext;

    private MEGLThread mEGLThread;
    private MGLRender mGLRender;

    public final static int RENDERMODE_WHEN_DIRTY = 0;
    public final static int RENDERMODE_CONTINUOUSLY = 1;

    private int mRenderMode = RENDERMODE_CONTINUOUSLY;


    public LEGLSurfaceView(Context context) {
        this(context, null);
    }

    public LEGLSurfaceView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public LEGLSurfaceView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        getHolder().addCallback(this);
    }

    public void setRender(MGLRender mGLRender) {
        this.mGLRender = mGLRender;
    }

    public void setRenderMode(int mRenderMode) {

        if(mGLRender == null)
        {
            throw  new RuntimeException("must set render before");
        }
        this.mRenderMode = mRenderMode;
    }

    public void setSurfaceAndEglContext(Surface surface, EGLContext eglContext)
    {
        this.surface = surface;
        this.eglContext = eglContext;
    }

    public EGLContext getEglContext()
    {
        if(mEGLThread != null)
        {
            return mEGLThread.getEglContext();
        }
        return null;
    }

    public void requestRender()
    {
        if(mEGLThread != null)
        {
            mEGLThread.requestRender();
        }
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if(surface == null)
        {
            surface = holder.getSurface();
        }
        mEGLThread = new MEGLThread(new WeakReference<LEGLSurfaceView>(this));
        mEGLThread.isCreate = true;
        mEGLThread.start();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

        mEGLThread.width = width;
        mEGLThread.height = height;
        mEGLThread.isChange = true;

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mEGLThread.onDestory();
        mEGLThread = null;
        surface = null;
        eglContext = null;
    }

    public interface MGLRender
    {
        void onSurfaceCreated();
        void onSurfaceChanged(int width, int height);
        void onDrawFrame();
    }


    static class MEGLThread extends Thread {

        private WeakReference<LEGLSurfaceView> meglSurfaceViewWeakReference;
        private LEGLHelper eglHelper = null;
        private Object object = null;

        private boolean isExit = false;
        private boolean isCreate = false;
        private boolean isChange = false;
        private boolean isStart = false;

        private int width;
        private int height;

        public MEGLThread(WeakReference<LEGLSurfaceView> meglSurfaceViewWeakReference) {
            this.meglSurfaceViewWeakReference = meglSurfaceViewWeakReference;
        }

        @Override
        public void run() {
            super.run();
            isExit = false;
            isStart = false;
            object = new Object();
            eglHelper = new LEGLHelper();
            eglHelper.initEgl(meglSurfaceViewWeakReference.get().surface, meglSurfaceViewWeakReference.get().eglContext);

            while (true)
            {
                if(isExit)
                {
                    //释放资源
                    release();
                    break;
                }

                if(isStart)
                {
                    if(meglSurfaceViewWeakReference.get().mRenderMode == RENDERMODE_WHEN_DIRTY)
                    {
                        synchronized (object)
                        {
                            try {
                                object.wait();
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                    else if(meglSurfaceViewWeakReference.get().mRenderMode == RENDERMODE_CONTINUOUSLY)
                    {
                        try {
                            Thread.sleep(1000 / 60);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                    else
                    {
                        throw  new RuntimeException("mRenderMode is wrong value");
                    }
                }


                onCreate();
                onChange(width, height);
                onDraw();

                isStart = true;

            }

        }

        private void onCreate()
        {
            if(isCreate && meglSurfaceViewWeakReference.get().mGLRender != null)
            {
                isCreate = false;
                meglSurfaceViewWeakReference.get().mGLRender.onSurfaceCreated();
            }
        }

        private void onChange(int width, int height)
        {
            if(isChange && meglSurfaceViewWeakReference.get().mGLRender != null)
            {
                isChange = false;
                meglSurfaceViewWeakReference.get().mGLRender.onSurfaceChanged(width, height);
            }
        }

        private void onDraw()
        {
            if(meglSurfaceViewWeakReference.get().mGLRender != null && eglHelper != null)
            {
                meglSurfaceViewWeakReference.get().mGLRender.onDrawFrame();
                if(!isStart)
                {
                    meglSurfaceViewWeakReference.get().mGLRender.onDrawFrame();

                }
                eglHelper.swapBuffers();

            }
        }

        private void requestRender()
        {
            if(object != null)
            {
                synchronized (object)
                {
                    object.notifyAll();
                }
            }
        }

        public void onDestory()
        {
            isExit = true;
            requestRender();
        }


        public void release()
        {
            if(eglHelper != null)
            {
                eglHelper.destoryEgl();
                eglHelper = null;
                object = null;
                meglSurfaceViewWeakReference = null;
            }
        }

        public EGLContext getEglContext()
        {
            if(eglHelper != null)
            {
                return eglHelper.getmEglContext();
            }
            return null;
        }

    }
}
