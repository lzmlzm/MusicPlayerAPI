//
// Created by lzm on 19-2-20.
//

#ifndef MUSIC2_MPLAYSTATUS_H
#define MUSIC2_MPLAYSTATUS_H


class MPlaystatus {

public:
    bool exit = false;
    bool load = true;
    bool seek = false;
public:
    MPlaystatus();
    ~MPlaystatus();
};


#endif //MUSIC2_MPLAYSTATUS_H
