//
// Created by lzm on 19-2-18.
//

#ifndef MUSIC2_MCALLJAVA_H
#define MUSIC2_MCALLJAVA_H

#include <cwchar>
#include "jni.h"
#include "AndroidLog.h"

#define MAIN_THREAD 0
#define CHILD_THREAD 1
class MCallJava {


public:


    JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj;
    jmethodID jmid_prepared;
    jmethodID jmid_load;
    jmethodID jmid_timeinfo;
    jmethodID jmid_error;
    jmethodID jmid_db;
    jmethodID jmid_callpcmtoaac;
    jmethodID jmid_callcomplete;
    jmethodID jmid_retpcm;
    jmethodID jmid_retpcmRate;
    jmethodID jmid_renderyuv;
    jmethodID jmid_supportMediacodec;
    jmethodID jmid_initMediaCodec;
    jmethodID jmid_decodeavpacket;
    jmethodID jmid_clearlastframe;
public:
    MCallJava(JavaVM *javaVM, JNIEnv *env, jobject *obj);

    ~MCallJava();

    void onCallPrepared(int type);

    void onCallLoad(int type, bool load);

    void onCallTimeInfo(int type, int cur, int total);

    void onCallError(int type,int code, char *msg);

    void onCallValueDB(int type, int db);

    void onCallPcmToAAC(int type, int size, void *buffer);

    void onCallComplete(int type);

    void onCallPcmInfo(void *buffer, int size);

    void onCallPcmRate(int samplerate);

    void onCallRenderYUV(int width,int height,uint8_t *frame_y,uint8_t *frame_u,uint8_t *frame_v);

    bool onCallisSupportMediaCodec(const char *codecName);

    void onCallInitMediaCodec(const char *mime, int width,int height,int csd0_size,int csd1_size, uint8_t *csd0, uint8_t *csd1);

    void onCallDecodeAvPacket(int datasize, uint8_t *pdata);


};


#endif //MUSIC2_MCALLJAVA_H
