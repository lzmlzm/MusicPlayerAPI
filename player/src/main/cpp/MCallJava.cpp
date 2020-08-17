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
         if(LOG_DEBUG)
         {
             LOGE("GET CHILD THREAD");
             return;
         }
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
            if (LOG_DEBUG) {
                LOGE("GET CHILD THREAD");
                return;
            }
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
            if (LOG_DEBUG) {
                LOGE("GET CHILD THREAD");
                return;
            }
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
            if(LOG_DEBUG)
            {
                LOGE("GET CHILD THREAD");
                return;
            }
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
            if (LOG_DEBUG) {
                LOGE("GET CHILD THREAD");
                return;
            }
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
            if (LOG_DEBUG) {
                LOGE("GET CHILD THREAD");
                return;
            }
        }
        //将C++层的buffer转换成jbyte array，传给java层
        jbyteArray jbuffer = jniEnv->NewByteArray(size);
        //填充数据
        jniEnv->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));

        //
        jniEnv->CallVoidMethod(jobj, jmid_callpcmtoaac, size, jbuffer);

        //销毁防止内存泄漏
        jniEnv->DeleteLocalRef(jbuffer);
    }

}
