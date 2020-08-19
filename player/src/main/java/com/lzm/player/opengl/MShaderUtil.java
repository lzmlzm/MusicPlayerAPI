package com.lzm.player.opengl;

import android.content.Context;
import android.opengl.GLES31;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import javax.microedition.khronos.opengles.GL;

//shader加载类
public class MShaderUtil {



    /**
     * 从shader里读出信息
     * @param context
     * @param rawId
     * @return
     * @throws IOException
     */
    public static  String readRawShader(Context context,int rawId) {
        //从RAW下打开输入数据流
        InputStream inputStream = context.getResources().openRawResource(rawId);
        //读取数据流
        BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));

        StringBuffer stringBuffer = new StringBuffer();

        String line;
        //一行一行读取
        try
        {
            while ((line = reader.readLine()) !=null)
            {
                stringBuffer.append(line).append("\n");
            }

            reader.close();
        }catch (Exception e)
        {
            e.printStackTrace();
        }
        return stringBuffer.toString();
    }


    /**
     * shader加载
     * @param shaderType 着色器类型
     * @param source raw读取到的shader源码
     * @return
     */
    public static int loadShader(int shaderType, String source)
    {
        int shader = GLES31.glCreateShader(shaderType);
        if(shader != 0)
        {
            //将源码与shader关联
            GLES31.glShaderSource(shader,source);
            //编译shader
            GLES31.glCompileShader(shader);
            int[] compile = new int[1];

            //检查shader,将编译状态传回给compile数组
            GLES31.glGetShaderiv(shader,GLES31.GL_COMPILE_STATUS,compile,0);

            //编译失败
            if(compile[0] != GLES31.GL_TRUE)
            {
                //删除shader
                Log.d("lzm","shader compile error!");
                GLES31.glDeleteShader(shader);
                shader = 0;
            }
        }

        return 0;
    }

    /**
     * 创建program
     * @param vertexSource 顶点着色器
     * @param fragmentSource 片源着色器
     * @return
     */
    public static int createProgram(String vertexSource,String fragmentSource)
    {
        //加载顶点着色器
        int vertexShader = loadShader(GLES31.GL_VERTEX_SHADER,vertexSource);
        if(vertexShader == 0)
        {
            return 0;
        }

        //加载片源着色器
        int fragmentShader = loadShader(GLES31.GL_FRAGMENT_SHADER,fragmentSource);
        if(fragmentShader == 0)
        {
            return 0;
        }

        //创建program
        int program = GLES31.glCreateProgram();
        if(program != 0)
        {
            //将shader 贴到 program上
            GLES31.glAttachShader(program,vertexShader);
            GLES31.glAttachShader(program,fragmentShader);

            //链接program
            GLES31.glLinkProgram(program);

            //j检查link是否成功
            int[] linkStatus = new int[1];
            GLES31.glGetProgramiv(program,GLES31.GL_LINK_STATUS,linkStatus,0);
            if(linkStatus[0] != GLES31.GL_TRUE)
            {
                Log.d("lzm","link program error");
                GLES31.glDeleteProgram(program);
                program = 0;
            }
        }

        return program;
    }
}
