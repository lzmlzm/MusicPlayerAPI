package com.lzm.music;

import androidx.appcompat.app.AppCompatActivity;

import android.opengl.GLES20;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import com.lzm.player.opengl.MEglHelper;

public class LivePush extends AppCompatActivity {

    private SurfaceView surfaceView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_live_push);

        surfaceView  = findViewById(R.id.surfaceview);
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {

            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                new Thread(){

                    @Override
                    public void run() {
                        super.run();

                        //初始化EGLHELPER
                        MEglHelper mEglHelper = new MEglHelper();
                        mEglHelper.initEgl(holder.getSurface(),null);

                        while (true)
                        {
                            GLES20.glViewport(0,0,width,height);
                            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
                            GLES20.glClearColor(1.0f,0.0f,0.0f,1.0f);

                            mEglHelper.swapBuffers();



                            try {
                                Thread.sleep(10);
                            }catch (Exception e){
                                e.printStackTrace();
                            }
                        }
                    }
                }.start();
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });

    }



}