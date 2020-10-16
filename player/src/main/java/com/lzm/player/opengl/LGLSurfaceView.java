package com.lzm.player.opengl;


import android.content.Context;
import android.util.AttributeSet;

//渲染推流app
public class LGLSurfaceView extends LEGLSurfaceView {

    public LGLSurfaceView(Context context) {
        this(context,null);
    }

    public LGLSurfaceView(Context context, AttributeSet attrs) {
        this(context, attrs,0);
    }

    public LGLSurfaceView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        setRender(new LRender());
        setRenderMode(LEGLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }
}
