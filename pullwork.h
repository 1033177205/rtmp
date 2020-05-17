#ifndef PULLWORK_H
#define PULLWORK_H

#include "mediabase.h"
#include "rtmpplayer.h"

class PullWork
{
public:
    PullWork();
    int init(const Properties& properties);
    int test_pull();

private:
    //rtmp 推流测试
    int _rtmp_debug;
    std::string _rtmp_debug_file;



    //url
    std::string _rtmp_url;

    //拉流器
    RtmpPlayer* _rtmp_player;
};

#endif // PULLWORK_H
