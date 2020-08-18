//
// Created by lzm on 2020/8/18.
//

#ifndef VIDEOAPP_MPCMBEAN_H
#define VIDEOAPP_MPCMBEAN_H
#include <SoundTouch.h>

using namespace soundtouch;

class MPcmBean {

public:
    char *buffer;
    int buffsize;

public:
    MPcmBean(SAMPLETYPE *buffer, int size);
    ~MPcmBean();


};

#endif //VIDEOAPP_MPCMBEAN_H
