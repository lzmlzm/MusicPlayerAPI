package com.lzm.player.opengl;
import android.content.Context;
import android.opengl.GLES20;

import android.opengl.GLSurfaceView;
import android.util.Log;

import com.lzm.player.R;
import com.lzm.player.log.mylog;

import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MRender implements GLSurfaceView.Renderer {

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


    private Context context;
    private int program_yuv;
    private int avPosition_yuv;
    private int afPosition_yuv;
    private int texture_Id;

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

    //构造方法
    public  MRender(Context context)
    {
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
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        //初始化YUV纹理
        initRenderYUV();
    }


    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        //起点终点，长宽
        GLES20.glViewport(0,0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        //对颜色缓冲区清屏
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(0,0,0,1);
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
     * 设置YUV数据
     * @param width
     * @param height
     * @param y
     * @param u
     * @param v
     */
    public void setYUVData(int width, int height, byte[] y,  byte[] u,  byte[] v)
    {
        this.width_yuv = width;
        this.height_yuv = height;
        //滑入yuv byte到buffer缓冲区内
        this.y = ByteBuffer.wrap(y);
        this.u = ByteBuffer.wrap(u);
        this.v = ByteBuffer.wrap(v);
    }

    /**
     * 渲染YUV
     */
    public void renderYUV()
    {
        if(width_yuv > 0 && height_yuv > 0 && y != null && u != null && v != null)
        {
            GLES20.glUseProgram(program_yuv);
            //使能顶点坐标
            GLES20.glEnableVertexAttribArray(avPosition_yuv);
            //将param5：顶点坐标传入
            GLES20.glVertexAttribPointer(avPosition_yuv,2,GLES20.GL_FLOAT,false,8, vertexBuffer);//2*4=8
            textureBuffer.position(0);
            //使能片源坐标
            GLES20.glEnableVertexAttribArray(afPosition_yuv);
            //将param5：纹理坐标传入
            GLES20.glVertexAttribPointer(afPosition_yuv,2,GLES20.GL_FLOAT,false,8, textureBuffer);//2*4=8

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
            y.clear();
            u.clear();
            v.clear();

            y=null;
            u=null;
            v=null;
        }
    }
}
