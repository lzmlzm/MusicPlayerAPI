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
public:
    MCallJava(JavaVM *javaVM, JNIEnv *env, jobject *obj);

    ~MCallJava();

    void onCallPrepared(int type);

    void onCallLoad(int type, bool load);

    void onCallTimeInfo(int type, int cur, int total);

    void onCallError(int type,int code, char *msg);

    void onCallValueDB(int type, int db);

    void onCallPcmToAAC(int type, int size, void *buffer);

};


#endif //MUSIC2_MCALLJAVA_H
