package com.lzm.player.util;

import android.media.MediaCodecInfo;
import android.media.MediaCodecList;

import com.lzm.player.log.mylog;

import java.util.HashMap;
import java.util.Map;

public class MVideoSupportUtil {

    private static Map<String, String> codecMap = new HashMap<>();

    static {
        codecMap.put("h264", "video/avc");
    }

    /**
     * 寻找编码器
     * @param codecName
     * @return
     */
    public  static String findVideoCodecName(String codecName)
    {
        if(codecMap.containsKey(codecName))
        {
            return codecMap.get(codecName);
        }

        return "";
    }

    /**
     *
     * @param codecType
     * @return
     */
    public static boolean isSupportCodec(String codecType)
    {
        boolean supportVideo = false;
        int count = MediaCodecList.getCodecCount();
        for(int i = 0; i < count; i++)
        {
            String[] types = MediaCodecList.getCodecInfoAt(i).getSupportedTypes();
            for(int j = 0; j<types.length; j++)
            {
                if(types[j].equals(findVideoCodecName(codecType)))
                {
                    mylog.d("支持");
                    supportVideo = true;
                    break;
                }
            }
            if(supportVideo)
            {
                break;
            }
        }
        return supportVideo;
    }






}
