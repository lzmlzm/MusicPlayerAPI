package com.lzm.player.opengl;

import android.content.Context;
import android.opengl.GLES20;
import android.util.Log;

import com.lzm.player.log.mylog;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;


//shader加载类
public class MShaderUtil {

    /**
     * 从shader里读出信息
     * @param context
     * @param rawId
     * @return
     * @throws IOException
     */
    public static String readRawTextFile(Context context, int rawId) {
        InputStream inputStream = context.getResources().openRawResource(rawId);
        try {
            BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                sb.append(line).append("\n");
            }
            reader.close();
            return sb.toString();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * shader加载
     * @param shaderType 着色器类型
     * @param source raw读取到的shader源码
     * @return
     */
    public static int loadShader(int shaderType, String source)
    {
        int shader = GLES20.glCreateShader(shaderType);
        if(shader != 0)
        {
            //将源码与shader关联
            GLES20.glShaderSource(shader,source);
            //编译shader
            GLES20.glCompileShader(shader);
            int[] compile = new int[1];

            //检查shader,将编译状态传回给compile数组
            GLES20.glGetShaderiv(shader,GLES20.GL_COMPILE_STATUS,compile,0);

            //编译失败
            if(compile[0] != GLES20.GL_TRUE)
            {
                //删除shader
                Log.d("lzm","shader compile error!");
                GLES20.glDeleteShader(shader);
                shader = 0;
            }
        }

        return shader;
    }

    /**
     * 创建program
     * @param vertexSource 顶点着色器
     * @param fragmentSource 片源着色器
     * @return
     */
    public static int createProgram(String vertexSource, String fragmentSource) {
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, vertexSource);
        if (vertexShader == 0) {
            return 0;
        }
        int pixelShader = loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource);
        if (pixelShader == 0) {
            return 0;
        }

        int program = GLES20.glCreateProgram();
        if (program != 0) {
            GLES20.glAttachShader(program, vertexShader);
            checkGlError("glAttachShader");
            GLES20.glAttachShader(program, pixelShader);
            checkGlError("glAttachShader");
            GLES20.glLinkProgram(program);
            int[] linkStatus = new int[1];
            GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linkStatus, 0);
            if (linkStatus[0] != GLES20.GL_TRUE) {
                mylog.d("Could not link program: ");
                mylog.d(GLES20.glGetProgramInfoLog(program));
                GLES20.glDeleteProgram(program);
                program = 0;
            }
        }
        return program;
    }

    public static void checkGlError(String label) {
        int error;
        while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
            mylog.d(label + ": glError " + error);
            throw new RuntimeException(label + ": glError " + error);
        }
    }
}
