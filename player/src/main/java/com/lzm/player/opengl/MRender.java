package com.lzm.player.opengl;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES31;
import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;

import com.lzm.player.R;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MRender implements GLSurfaceView.Renderer {

    //顶点坐标系
    //          ^
    //          |
    //          |
    //  --------------------->
    //          |
    //          |
    //          |
    private final float[] vertexData={
            -1f,-1f,
            1f,-1f,
            -1f,1f,
            1f,1f
    };
    //图片纹理坐标系
    //|
    //--------------------->
    //|
    //|
    //|
    //|
    //v
    private final float[] textureData={
            0f,1f,
            1f,1f,
            0f,0f,
            1f,0f
    };

    private FloatBuffer vertexBuffer;
    private FloatBuffer textureBuffer;


    private Context context;
    private int program;
    private int avPosition;
    private int afPosition;
    private int sTexture;
    private int texture_Id;

    //构造方法
    public  MRender(Context context)
    {
        this.context = context;

        vertexBuffer = ByteBuffer.allocateDirect(vertexData.length*4)//为顶点坐标分配buffer
                .order(ByteOrder.nativeOrder())//使用native方法
                .asFloatBuffer()//转为float
                .put(vertexData);//放入数据
        vertexBuffer.position(0);

        textureBuffer = ByteBuffer.allocateDirect(textureData.length*4)//为顶点坐标分配buffer
                .order(ByteOrder.nativeOrder())//使用native方法
                .asFloatBuffer()//转为float
                .put(textureData);//放入数据
        textureBuffer.position(0);

    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

        //从文件中读取顶点shader源码
        String vertexShaderSource = MShaderUtil.readRawShader(context, R.raw.vertex_shader);

        //从文件中读取片源shader源码
        String fragmentShaderSource = MShaderUtil.readRawShader(context, R.raw.fragment_shader);

        //创建program
        program = MShaderUtil.createProgram(vertexShaderSource,fragmentShaderSource);
        //success
        if(program>0)
        {
            //从着色器代码中获取"av_Position" "af_Color"
            avPosition = GLES31.glGetAttribLocation(program,"av_Position");
            afPosition = GLES31.glGetAttribLocation(program,"af_Position");
            sTexture = GLES31.glGetUniformLocation(program,"sTexture");

            //创建纹理
            int[] textureIds = new int[1];
            GLES31.glGenTextures(1,textureIds,0);
            if(textureIds[0] == 0)
            {
                return;
            }

            //保存纹理
            texture_Id = textureIds[0];
            //绑定纹理
            GLES31.glBindTexture(GLES31.GL_TEXTURE_2D, texture_Id);
            //设置环绕方式:
            //s,t=x,y,超出边界的重复绘制
            GLES31.glTexParameteri(GLES31.GL_TEXTURE_2D, GLES31.GL_TEXTURE_WRAP_S, GLES31.GL_REPEAT);
            GLES31.glTexParameteri(GLES31.GL_TEXTURE_2D, GLES31.GL_TEXTURE_WRAP_T, GLES31.GL_REPEAT);
            //设置过滤方式,纹理像素映射到坐标点
            GLES31.glTexParameteri(GLES31.GL_TEXTURE_2D, GLES31.GL_TEXTURE_MIN_FILTER, GLES31.GL_LINEAR);
            GLES31.glTexParameteri(GLES31.GL_TEXTURE_2D, GLES31.GL_TEXTURE_MAG_FILTER, GLES31.GL_LINEAR);


            BitmapFactory.Options options = new BitmapFactory.Options();
            //关闭缩放
            options.inScaled = false;
            //生成一个bitmap
            //Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(),);
            Bitmap bitmap =null;
            if(bitmap==null)
            {
                return;
            }
            //加载图片
            GLUtils.texImage2D(GLES31.GL_TEXTURE_2D,0,bitmap,0);
            bitmap.recycle();
            bitmap = null;
        }
    }


    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        //起点终点，长宽
        GLES31.glViewport(0,0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        //对颜色缓冲区清屏
        GLES31.glClear(GLES31.GL_COLOR_BUFFER_BIT);
        GLES31.glClearColor(1,1,1,1);

        //使用program
        GLES31.glUseProgram(program);

        //使能顶点坐标
        GLES31.glEnableVertexAttribArray(avPosition);
        //将param5：顶点坐标传入
        GLES31.glVertexAttribPointer(avPosition,2,GLES31.GL_FLOAT,false,8, vertexBuffer);//2*4=8

        //使能片源坐标
        GLES31.glEnableVertexAttribArray(afPosition);
        //将param5：纹理坐标传入
        GLES31.glVertexAttribPointer(afPosition,2,GLES31.GL_FLOAT,false,8, textureBuffer);//2*4=8
        GLES31.glDrawArrays(GLES31.GL_TRIANGLE_STRIP,0,4);

    }
}
