//
// Created by lzm on 19-2-18.
//

#ifndef MUSIC2_ANDROIDLOG_H
#define MUSIC2_ANDROIDLOG_H

#include "android/log.h"

#define LOG_DEBUG true

#define LOGD(FORMAT,...)  __android_log_print(ANDROID_LOG_DEBUG,"LZM",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...)  __android_log_print(ANDROID_LOG_ERROR,"LZM",FORMAT,##__VA_ARGS__);

#endif //MUSIC2_ANDROIDLOG_H
