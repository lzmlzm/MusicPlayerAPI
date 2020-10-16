package com.lzm.player.opengl;

import android.opengl.GLES20;

public class LRender implements LEGLSurfaceView.MGLRender {


    public LRender() {

    }

    @Override
    public void onSurfaceCreated() {

    }

    @Override
    public void onSurfaceChanged(int width, int height) {

        GLES20.glViewport(0,0,width,height);

    }

    @Override
    public void onDrawFrame() {

        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(0.0f,0.0f,0.0f,1.0f);
    }
}
