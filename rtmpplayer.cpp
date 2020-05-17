#include "rtmpplayer.h"
#include <iostream>

int RtmpPlayer::readPacket(char* buffer, int bufferSize)
{
    return RTMP_Read(_rtmp, buffer, bufferSize);
}
