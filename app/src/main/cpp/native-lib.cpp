#include <jni.h>
#include <string>
#include "AndroidLog.h"
#include "MCallJava.h"
#include "MFFmpeg.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>

}
/*
 *
 * JAVA层调用C++函数
 *
 * */

JavaVM *javaVM = NULL;


MCallJava *callJava = NULL;
MFFmpeg *mfFmpeg = NULL;
MPlaystatus *mPlaystatus = NULL;

bool nexit = true;

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    jint result = -1;
    javaVM=vm;
    JNIEnv *env;
    if(vm->GetEnv((void **)&env,JNI_VERSION_1_4) != JNI_OK)
    {
        return result;
    }

    return JNI_VERSION_1_4;
}




extern "C"
JNIEXPORT void JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1prepared(JNIEnv *env, jobject instance, jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);

    if(mfFmpeg == NULL)
    {
        if(callJava == NULL)
        {
            callJava = new MCallJava(javaVM,env,&instance);
        }
        callJava->onCallLoad(MAIN_THREAD,true);
        mPlaystatus = new MPlaystatus();
        mfFmpeg = new MFFmpeg(mPlaystatus,callJava,source);//解析URL
        mfFmpeg->prepared();
    }


    //env->ReleaseStringUTFChars(source_, source);
}extern "C"
JNIEXPORT void JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1start(JNIEnv *env, jobject instance) {

    if(mfFmpeg != NULL)
    {
        mfFmpeg->start();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1pause(JNIEnv *env, jobject instance) {

    if (mfFmpeg != NULL) {
        mfFmpeg->pause();
    }


}extern "C"
JNIEXPORT void JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1reusme(JNIEnv *env, jobject instance) {

    if (mfFmpeg != NULL) {
        mfFmpeg->resume();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1stop(JNIEnv *env, jobject instance) {

    if(!nexit)
    {
        return;
    }

    jclass jlz = env->GetObjectClass(instance);
    jmethodID jmid_next = env->GetMethodID(jlz,"onCallNext","()V");//获取JAVA方法

    nexit = false;
    if(mfFmpeg != NULL)
    {
        mfFmpeg->release();
        delete(mfFmpeg);
        mfFmpeg = NULL;
        if(callJava != NULL)
        {
            delete(callJava);
            callJava = NULL;
        }
        if(mPlaystatus != NULL)
        {
            delete(mPlaystatus);
            mPlaystatus = NULL;
        }
    }
    nexit = true;
    env->CallVoidMethod(instance,jmid_next);//回调下一个播放
}extern "C"
JNIEXPORT void JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1seek(JNIEnv *env, jobject instance, jint secs) {

    if(mfFmpeg != NULL)
    {
        mfFmpeg->seek(secs);
    }

}