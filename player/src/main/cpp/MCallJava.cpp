//
// Created by lzm on 19-2-18.
//

#include "MCallJava.h"

MCallJava::MCallJava(JavaVM *javaVM, JNIEnv *env, jobject *obj) {

    this->javaVM=javaVM;
    this->jniEnv=env;
    this->jobj = env->NewGlobalRef(*obj);

    jclass jlz = env->GetObjectClass(jobj);

    if(!jlz)
    {

        if(LOG_DEBUG)
        {
            LOGE("get jclass wrong")
        }
        return;
    }

    jmid_prepared = env->GetMethodID(jlz, "onCallPrepared", "()V");
    jmid_load = env->GetMethodID(jlz, "onCallLoad", "(Z)V");
    jmid_timeinfo = env->GetMethodID(jlz, "onCallTimeInfo", "(II)V");
    jmid_error = env->GetMethodID(jlz,"onCallError","(ILjava/lang/String;)V");
}

MCallJava::~MCallJava() {

}

void MCallJava::onCallPrepared(int type) {


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

    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_load,load);

    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("GET CHILD THREAD");
                return;
            }
        }

        jniEnv->CallVoidMethod(jobj, jmid_load,load);
        javaVM->DetachCurrentThread();
    }


}

void MCallJava::onCallTimeInfo(int type, int cur, int total) {

    if (type == MAIN_THREAD) {
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
