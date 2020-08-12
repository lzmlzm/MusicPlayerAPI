package com.lzm.player;

/**
 * 版权：麻瓜 版权所有
 *
 * @author ${LZM}
 * 版本：1.0
 * 创建日期：19-2-1615
 * qq邮箱：1198492751@qq.com
 */
public class Demo {
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swscale-4");
    }

}
