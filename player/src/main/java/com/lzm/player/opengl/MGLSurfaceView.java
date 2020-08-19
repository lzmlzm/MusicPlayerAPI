package com.lzm.player.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

public class MGLSurfaceView extends GLSurfaceView {
    public MGLSurfaceView(Context context) {
        super(context);
    }

    public MGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);

        //
        setRenderer(new MRender(context));
    }
}
