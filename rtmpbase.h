#ifndef RTMPBASE_H
#define RTMPBASE_H

//介绍：这是对librtmp封装
#include "librtmp/rtmp_sys.h"
#include "librtmp/rtmp.h"
#include <iostream>
#include <librtmp/log.h>

//不懂这个通道是怎么赋值的
enum RTMPChannel
{
   RTMP_NETWORK_CHANNEL = 2,   ///< channel for network-related messages (bandwidth report, ping, etc)
   RTMP_SYSTEM_CHANNEL,        ///< channel for sending server control messages
   RTMP_AUDIO_CHANNEL,         ///< channel for audio data
   RTMP_VIDEO_CHANNEL   = 6,   ///< channel for video data
   RTMP_SOURCE_CHANNEL  = 8,   ///< channel for a/v invokes
};

enum RTMP_WORK_TYPE
{
    RTMP_WORK_TYPE_UNKNOW,
    RTMP_WORK_TYPE_PLAY,
    RTMP_WORK_TYPE_PUSH
};

class rtmpbase
{
public:
    rtmpbase();
    rtmpbase(RTMP_WORK_TYPE rtmp_work);
    ~rtmpbase();
    bool connect();
    bool connect(std::string& url);
    bool connect(char* url);
    bool isConnect();
    void close();

private:
    bool init();
    bool initSockets();
    void cleanupSockets();
    RTMP_WORK_TYPE _rtmp_work;

protected:
    RTMP *_rtmp;
    std::string _url;
};

#endif // RTMPBASE_H
