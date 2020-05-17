#ifndef RTMPPUSHER_H
#define RTMPPUSHER_H

#include "rtmpbase.h"

class RtmpPusher: public rtmpbase
{
public:
    RtmpPusher():rtmpbase(RTMP_WORK_TYPE_PUSH){printf("rtmp pusher init\n");}

    int sendPacket(unsigned int packet_type, unsigned char* data, unsigned int size, unsigned int timestamp);
};

#endif // RTMPPUSHER_H
