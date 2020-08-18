//
// Created by lzm on 2020/8/18.
//

#include "MPcmBean.h"

MPcmBean::MPcmBean(SAMPLETYPE *buffer, int size) {

    this->buffer = (char *) malloc(size);
    this->buffsize = size;
    memcpy(this->buffer, buffer, size);

}

MPcmBean::~MPcmBean() {
    free(buffer);
    buffer = NULL;
}