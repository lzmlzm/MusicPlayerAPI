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

bool next = true;

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
        mfFmpeg = new MFFmpeg(mPlaystatus,callJava,source);
        mfFmpeg->prepared();//解析URL音频数据
    }


    //env->ReleaseStringUTFChars(source_, source);
}extern "C"
JNIEXPORT void JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1start(JNIEnv *env, jobject instance) {

    if(mfFmpeg != NULL)
    {
        //ffmpeg开始解码
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

    if(!next)
    {
        return;
    }
    //Call next方法
    jclass jlz = env->GetObjectClass(instance);
    jmethodID jmid_next = env->GetMethodID(jlz,"onCallNext","()V");//获取JAVA方法

    next = false;
    //释放
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
    next = true;

    env->CallVoidMethod(instance,jmid_next);//回调播放下一个

}extern "C"
JNIEXPORT void JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1seek(JNIEnv *env, jobject instance, jint secs) {
    if(mfFmpeg != NULL)
    {
        mfFmpeg->seek(secs);
    }
}extern "C"
JNIEXPORT jint JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1duration(JNIEnv *env, jobject thiz) {
    if(mfFmpeg != NULL)
    {
        LOGD("FFMPEG->duration is %d", mfFmpeg->duration);
        return mfFmpeg->duration;
    }
    return 0;
}extern "C"
JNIEXPORT void JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1volume(JNIEnv *env, jobject thiz, jint percent) {
    if (mfFmpeg != NULL) {
        mfFmpeg->setVolume(percent);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1mute(JNIEnv *env, jobject thiz, jint mute) {
    if (mfFmpeg != NULL) {
        mfFmpeg->setMute(mute);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1pitch(JNIEnv *env, jobject thiz, jfloat pitch) {
    if (mfFmpeg != NULL) {
        mfFmpeg->setPitch(pitch);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1speed(JNIEnv *env, jobject thiz, jfloat speed) {
    if (mfFmpeg != NULL) {
        mfFmpeg->setSpeed(speed);
    }
}extern "C"
JNIEXPORT int JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1samplerate(JNIEnv *env, jobject thiz) {
    if (mfFmpeg != NULL) {
        return mfFmpeg->getSamplerate();
    }
    return 0;
}extern "C"
JNIEXPORT void JNICALL
Java_com_lzm_player_myplayer_Mplayer_n_1record(JNIEnv *env, jobject thiz, jboolean start) {
    if (mfFmpeg != NULL) {
         mfFmpeg->setRecordStatus(start);
    }
}