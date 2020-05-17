#ifndef RTMPPLAYER_H
#define RTMPPLAYER_H

#include "rtmpbase.h"

class RtmpPlayer : public rtmpbase
{
public:
    RtmpPlayer():rtmpbase(RTMP_WORK_TYPE_PLAY){printf("rtmp player init\n");}

    int readPacket(char* buffer, int bufferSize);
};

#endif // RTMPPLAYER_H
