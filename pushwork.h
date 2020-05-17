#ifndef PUSHWORK_H
#define PUSHWORK_H

#include <iostream>
#include "mediabase.h"
#include "rtmppusher.h"
#include "flvanalyze.h"

class PushWork
{
public:
    PushWork();
    bool init(const Properties& properties);
    int test_push();


private:
    //rtmp 推流测试
    int _rtmp_debug;
    std::string _rtmp_debug_file;



    //url
    std::string _rtmp_url;

    //推流器
    RtmpPusher* _rtmp_pusher;
};

#endif // PUSHWORK_H
