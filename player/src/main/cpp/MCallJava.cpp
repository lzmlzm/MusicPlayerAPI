//
// Created by lzm on 19-2-18.
//

#include "MCallJava.h"
//C++回调JAVA
//JNI层回调数据给JAVA

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
    jmid_db = env->GetMethodID(jlz,"onCallValueDB","(I)V")
}

MCallJava::~MCallJava() {

}

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
