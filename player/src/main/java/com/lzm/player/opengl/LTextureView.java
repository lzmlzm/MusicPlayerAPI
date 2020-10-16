package com.lzm.player.opengl;

import android.content.Context;
import android.util.AttributeSet;

public class LTextureView extends LEGLSurfaceView{


    public LTextureView(Context context) {
        this(context,null);
    }

    public LTextureView(Context context, AttributeSet attrs) {
        this(context, attrs,0);
    }

    public LTextureView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);

        setRender(new LTextureRender(context));

    }
}
