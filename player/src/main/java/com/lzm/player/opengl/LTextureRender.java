package com.lzm.player.opengl;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.view.Surface;

import com.lzm.player.R;
import com.lzm.player.log.mylog;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

public class LTextureRender implements LEGLSurfaceView.MGLRender {

    private Context context;

    public static final int RENDER_YUV = 1;
    public static final int RENDER_MEDIACODEC = 2;

    private final float[] vertexData={
            -1f,-1f,
            1f,-1f,
            -1f,1f,
            1f,1f
    };

    private final float[] textureData={
            0f,1f,
            1f,1f,
            0f,0f,
            1f,0f
    };

    private FloatBuffer vertexBuffer;
    private FloatBuffer textureBuffer;

    //YUV
    private int program_yuv;
    private int avPosition_yuv;
    private int afPosition_yuv;

    private int sample_y;
    private int sample_u;
    private int sample_v;

    private int width_yuv;
    private int height_yuv;

    //保存C++传来的yuv数据
    private ByteBuffer y;
    private ByteBuffer u;
    private ByteBuffer v;

    private int[] textureId_yuv;

    //MediaCodec
    private int program_meidacodec;
    private int avPosition_meidacodec;
    private int afPosition_meidacodec;
    private int samplerOES_mediacodec;
    private int textureid_mediacodec;

    private SurfaceTexture surfaceTexture;
    private Surface surface;
    private MRender.OnSurfaceCreateListener onSurfaceCreateListener;
    private MRender.OnRenderListener onRenderListener;

    private int renderType = RENDER_YUV;

    //VBO buffer
    private int vboid;

    public LTextureRender(Context context) {
        this.context = context;

        vertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)//为顶点坐标分配buffer
                .order(ByteOrder.nativeOrder())//使用native方法
                .asFloatBuffer()//转为float
                .put(vertexData);//放入数据
        vertexBuffer.position(0);

        textureBuffer = ByteBuffer.allocateDirect(textureData.length * 4)//为顶点坐标分配buffer
                .order(ByteOrder.nativeOrder())//使用native方法
                .asFloatBuffer()//转为float
                .put(textureData);//放入数据
        textureBuffer.position(0);
    }

    @Override
    public void onSurfaceCreated() {

        initRenderYUV();
    }

    @Override
    public void onSurfaceChanged(int width, int height) {
        GLES20.glViewport(0,0,width,height);
    }

    @Override
    public void onDrawFrame() {
        renderYUV();
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP,0,4);
    }



    /**
     * 初始化YUV纹理
     */
    private void initRenderYUV()
    {
        //从文件中读取顶点shader源码
        String vertexShaderSource = MShaderUtil.readRawTextFile(context, R.raw.vertex_shader);
        //从文件中读取片源shader源码
        String fragmentShaderSource = MShaderUtil.readRawTextFile(context, R.raw.fragment_shader);
        //创建program
        program_yuv = MShaderUtil.createProgram(vertexShaderSource,fragmentShaderSource);
        if (program_yuv == 0)
        {
            mylog.d("program error");
            return;
        }

        //从着色器代码中获取"av_Position" "af_Color"

        avPosition_yuv = GLES20.glGetAttribLocation(program_yuv,"av_Position");
        afPosition_yuv = GLES20.glGetAttribLocation(program_yuv,"af_Position");

        sample_y = GLES20.glGetUniformLocation(program_yuv,"sampler_y");
        sample_u = GLES20.glGetUniformLocation(program_yuv,"sampler_u");
        sample_v = GLES20.glGetUniformLocation(program_yuv,"sampler_v");

        //创建VBO数组
        int [] vbos = new int[1];
        //令GLES生成VBO数据buffer
        GLES20.glGenBuffers(1,vbos,0);
        vboid = vbos[0];

        //绑定VBO
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER,vboid);
        //分配VBO需要的缓存大小以及缓存用途
        GLES20.glBufferData(GLES20.GL_ARRAY_BUFFER,vertexData.length*4+textureData.length*4,null,GLES20.GL_STATIC_DRAW);
        //为VBO设置顶点数据值,纹理数据值
        GLES20.glBufferSubData(GLES20.GL_ARRAY_BUFFER,0,vertexData.length*4,vertexBuffer);
        GLES20.glBufferSubData(GLES20.GL_ARRAY_BUFFER,0,textureData.length*4,textureBuffer);
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER,0);

        //创建YUV纹理
        textureId_yuv = new int[3];
        GLES20.glGenTextures(3,textureId_yuv,0);
        for(int i=0;i<3;i++)
        {
            //绑定纹理
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[i]);
            //设置环绕方式:
            //s,t=x,y,超出边界的重复绘制
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
            //设置过滤方式,纹理像素映射到坐标点
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        }
    }

    /**
     * 渲染YUV
     */
    public void renderYUV()
    {
        if(width_yuv > 0 && height_yuv > 0 && y != null && u != null && v != null)
        {
            GLES20.glUseProgram(program_yuv);

            //绑定VBO
            GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER,vboid);
            //使能顶点坐标
            GLES20.glEnableVertexAttribArray(avPosition_yuv);
            //将param5：顶点坐标传入，从VBO的偏移0开始取顶点数据
            GLES20.glVertexAttribPointer(avPosition_yuv,2,GLES20.GL_FLOAT,false,8, 0);//2*4=8
            //使能片源坐标
            GLES20.glEnableVertexAttribArray(afPosition_yuv);
            //将param5：纹理坐标传入,从VBO偏移textureData.length*4即为片源顶点数据
            GLES20.glVertexAttribPointer(afPosition_yuv,2,GLES20.GL_FLOAT,false,8, textureData.length*4);//2*4=8

            //激活纹理
            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            //绑定纹理
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[0]);
            //映射，传入y像素值
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D,0,GLES20.GL_LUMINANCE,
                    width_yuv,height_yuv,0,
                    GLES20.GL_LUMINANCE,GLES20.GL_UNSIGNED_BYTE,y);

            //激活纹理
            GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            //绑定纹理
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[1]);
            //映射，传入u像素值
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D,0,GLES20.GL_LUMINANCE,
                    width_yuv / 2,height_yuv / 2,0,
                    GLES20.GL_LUMINANCE,GLES20.GL_UNSIGNED_BYTE,u);

            //激活纹理
            GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
            //绑定纹理
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[2]);
            //映射，传入v像素值
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D,0,GLES20.GL_LUMINANCE,
                    width_yuv / 2,height_yuv / 2,0,
                    GLES20.GL_LUMINANCE,GLES20.GL_UNSIGNED_BYTE,v);

            GLES20.glUniform1i(sample_y,0);
            GLES20.glUniform1i(sample_u,1);
            GLES20.glUniform1i(sample_v,2);
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
            GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER,0);
            y.clear();
            u.clear();
            v.clear();

            y=null;
            u=null;
            v=null;
        }
    }
}
