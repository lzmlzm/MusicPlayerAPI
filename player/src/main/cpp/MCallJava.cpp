//
// Created by lzm on 19-2-18.
//

#include "MCallJava.h"
//C++回调JAVA
//JNI层回调数据给JAVA
/**
 * 设置jvm环境
 */
MCallJava::MCallJava(JavaVM *javaVM, JNIEnv *env, jobject *obj) {

    //设置JAVA虚拟机、环境、obj
    this->javaVM=javaVM;
    this->jniEnv=env;
    this->jobj = env->NewGlobalRef(*obj);

    //根据传入的jobj获取jclass
    jclass jlz = env->GetObjectClass(jobj);

    if(!jlz)
    {

        if(LOG_DEBUG)
        {
            LOGE("get jclass wrong")
        }
        return;
    }
    //根据jclass获取methodid
    jmid_prepared = env->GetMethodID(jlz, "onCallPrepared", "()V");
    jmid_load = env->GetMethodID(jlz, "onCallLoad", "(Z)V");
    jmid_timeinfo = env->GetMethodID(jlz, "onCallTimeInfo", "(II)V");
    jmid_error = env->GetMethodID(jlz,"onCallError","(ILjava/lang/String;)V");
    jmid_db = env->GetMethodID(jlz,"onCallValueDB","(I)V");
    jmid_callpcmtoaac = env->GetMethodID(jlz,"encodePcmToAAC","(I[B)V");
    jmid_callcomplete = env->GetMethodID(jlz,"onCallComplete","()V");
    jmid_retpcm = env->GetMethodID(jlz,"onCallPcmInfo","([BI)V");
    jmid_retpcmRate = env->GetMethodID(jlz,"onCallPcmRate","(I)V");
    jmid_renderyuv = env->GetMethodID(jlz,"onCallRenderYUV","(II[B[B[B)V");
    jmid_supportMediacodec = env->GetMethodID(jlz, "onCallisSupportMediaCodec","(Ljava/lang/String;)Z");
    jmid_initMediaCodec = env->GetMethodID(jlz, "initMediaCodec","(Ljava/lang/String;II[B[B)V");
    jmid_decodeavpacket = env->GetMethodID(jlz, "decodeAVPacket", "(I[B)V");

}

MCallJava::~MCallJava() {

}

/**
 * 调用JAVA层资源准备函数
 * @param type
 */
void MCallJava::onCallPrepared(int type) {

    //判断回调函数在主线程还是子线程
    if(type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj,jmid_prepared);
    }
    else if (type == CHILD_THREAD)
    {
     JNIEnv *jniEnv;
     if(javaVM->AttachCurrentThread(&jniEnv,0)!= JNI_OK)
     {
         return;
     }

     jniEnv->CallVoidMethod(jobj,jmid_prepared);
     javaVM->DetachCurrentThread();
    }

}

/**
 * 传回播放加载状态
 * @param type
 * @param load
 */
void MCallJava::onCallLoad(int type, bool load) {
    //判断回调函数在主线程还是子线程
    if (type == MAIN_THREAD) {
        //回调load值给java层的onCallLoad
        jniEnv->CallVoidMethod(jobj, jmid_load,load);

    } else if (type == CHILD_THREAD) {
        //子线程必须通过JVM来获取当前线程的Jnienv
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }

        //回调load值给java层的onCallLoad
        jniEnv->CallVoidMethod(jobj, jmid_load,load);
        javaVM->DetachCurrentThread();
    }

}

/**
 * 传回播放时长
 * @param type
 * @param cur
 * @param total
 */
void MCallJava::onCallTimeInfo(int type, int cur, int total) {

    if (type == MAIN_THREAD) {

        //将当前时间和总时间回调给java层
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, cur, total);

    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }

        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, cur, total);
        javaVM->DetachCurrentThread();
    }
}

/**
 * 传回播放出错信息
 * @param type
 * @param code
 * @param msg
 */
void MCallJava::onCallError(int type, int code, char *msg) {

    if(type == MAIN_THREAD)
    {
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj,jmid_error,code,jmsg);
        jniEnv->DeleteGlobalRef(jmsg);

    }
    else if (type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv,0)!= JNI_OK)
        {
            return;
        }

        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj,jmid_error,code,jmsg);
        jniEnv->DeleteGlobalRef(jmsg);
        javaVM->DetachCurrentThread();
    }

}

/**
 * 回传分贝值
 * @param type
 * @param db
 */
void MCallJava::onCallValueDB(int type, int db) {

    if (type == MAIN_THREAD) {

        //
        jniEnv->CallVoidMethod(jobj, jmid_db, db);

    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }

        jniEnv->CallVoidMethod(jobj, jmid_db, db);
        javaVM->DetachCurrentThread();
    }
}

/**
 * pcm转aac数据，将C++层的pcm数据给JAVA层处理
 * @param type
 * @param size
 * @param buffer
 */
void MCallJava::onCallPcmToAAC(int type, int size, void *buffer) {
    if (type == MAIN_THREAD) {
        //将C++层的buffer转换成jbyte array，传给java层
        jbyteArray jbuffer = jniEnv->NewByteArray(size);
        //填充数据
        jniEnv->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));

        //
        jniEnv->CallVoidMethod(jobj, jmid_callpcmtoaac, size, jbuffer);

        //销毁防止内存泄漏
        jniEnv->DeleteLocalRef(jbuffer);

    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        //将C++层的buffer转换成jbyte array，传给java层
        jbyteArray jbuffer = jniEnv->NewByteArray(size);
        //填充数据
        jniEnv->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));

        //
        jniEnv->CallVoidMethod(jobj, jmid_callpcmtoaac, size, jbuffer);

        //销毁防止内存泄漏
        jniEnv->DeleteLocalRef(jbuffer);
        javaVM->DetachCurrentThread();
    }

}

void MCallJava::onCallComplete(int type) {
    if (type == MAIN_THREAD) {

        jniEnv->CallVoidMethod(jobj, jmid_callcomplete);

    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
                return;
        }

        jniEnv->CallVoidMethod(jobj, jmid_callcomplete);
        javaVM->DetachCurrentThread();
    }
}

void MCallJava::onCallPcmInfo(void *buffer, int size) {
    //将buffer转为jbytearray传给java
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
    {
        return;
    }

    //将C++层的buffer转换成jbyte array，传给java层
    jbyteArray jbuffer = jniEnv->NewByteArray(size);
    //填充数据
    jniEnv->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));

    //
    jniEnv->CallVoidMethod(jobj, jmid_retpcm, size, jbuffer, size);

    //销毁防止内存泄漏
    jniEnv->DeleteLocalRef(jbuffer);

    javaVM->DetachCurrentThread();

}

/**
 * 返回采样率
 * @param samplerate
 */
void MCallJava::onCallPcmRate(int samplerate) {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
    {
        return;
    }

    jniEnv->CallVoidMethod(jobj, jmid_retpcmRate,samplerate);
    javaVM->DetachCurrentThread();

}

void MCallJava::onCallRenderYUV(int width, int height, uint8_t *frame_y, uint8_t *frame_u,
                                uint8_t *frame_v) {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
    {
        return;
    }
    //将yuv原始的uint8转为jbyte
    jbyteArray jbyteY = jniEnv->NewByteArray(width*height);
    jniEnv->SetByteArrayRegion(jbyteY, 0, width*height, reinterpret_cast<const jbyte *>(frame_y));
    //u 数据大小长宽各占1/2  YUV420P Y:UV= 4:2
    jbyteArray jbyteU = jniEnv->NewByteArray(width* height/4);
    jniEnv->SetByteArrayRegion(jbyteU, 0, width*height/4, reinterpret_cast<const jbyte *>(frame_u));
    //v数据大小各占1/2
    jbyteArray jbyteV = jniEnv->NewByteArray(width* height/4);
    jniEnv->SetByteArrayRegion(jbyteV, 0, width* height/4, reinterpret_cast<const jbyte *>(frame_v));

    jniEnv->CallVoidMethod(jobj, jmid_renderyuv,width,height,jbyteY,jbyteU,jbyteV);

    jniEnv->DeleteLocalRef(jbyteY);
    jniEnv->DeleteLocalRef(jbyteU);
    jniEnv->DeleteLocalRef(jbyteV);

    javaVM->DetachCurrentThread();
}

bool MCallJava::onCallisSupportMediaCodec(const char *codecName) {

    bool support = false;
    JNIEnv *jniEnv;
    if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
    {
        if(LOG_DEBUG)
        {
            LOGE("call onCallComplete worng");
        }
        return support;
    }

    jstring type = jniEnv->NewStringUTF(codecName);
    support = jniEnv->CallBooleanMethod(jobj, jmid_supportMediacodec, type);
    jniEnv->DeleteLocalRef(type);
    /*javaVM->DetachCurrentThread();*/
    return support;
}

void MCallJava::onCallInitMediaCodec(const char *mime, int width,int height,int csd0_size,int csd1_size,  uint8_t *csd0, uint8_t *csd1) {

    //JNIEnv *jniEnv;

    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
    {
        if(LOG_DEBUG)
        {
                LOGE("oncallmediacodec error");
        }
        return;
    }


    jstring type = jniEnv->NewStringUTF(mime);

    jbyteArray jbytecsd0 = jniEnv->NewByteArray(csd0_size);
    jniEnv->SetByteArrayRegion(jbytecsd0, 0, csd0_size, reinterpret_cast<const jbyte *>(csd0));

    jbyteArray jbytecsd1 = jniEnv->NewByteArray(csd1_size);
    jniEnv->SetByteArrayRegion(jbytecsd1, 0, csd1_size, reinterpret_cast<const jbyte *>(csd1));

    jniEnv->CallVoidMethod(jobj, jmid_initMediaCodec,type,width,height,jbytecsd0,jbytecsd1);

    jniEnv->DeleteLocalRef(type);
    jniEnv->DeleteLocalRef(jbytecsd0);
    jniEnv->DeleteLocalRef(jbytecsd1);

    //javaVM->DetachCurrentThread();
}

void MCallJava::onCallDecodeAvPacket(int datasize, uint8_t *pdata) {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
    {
        if(LOG_DEBUG)
        {
            LOGE("oncallmediacodec error");
        }
        return;
    }

    jbyteArray data = jniEnv->NewByteArray(datasize);
    jniEnv->SetByteArrayRegion(data, 0, datasize, reinterpret_cast<const jbyte *>(pdata));
    jniEnv->CallVoidMethod(jobj, jmid_decodeavpacket, datasize, data);
    jniEnv->DeleteLocalRef(data);
    javaVM->DetachCurrentThread();
}
