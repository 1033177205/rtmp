#include "rtmppusher.h"
#include "stdlib.h"
#include "librtmp/rtmp.h"
















int RtmpPusher::sendPacket(unsigned int packet_type, unsigned char* data, unsigned int size, unsigned int timestamp)
{
    if(_rtmp == NULL) {
        printf("sendPacket _rtmp is null\n");
        return -1;
    }

    //发送的packet包
    RTMPPacket packet;
    // 为包申请了buffer, 实际在内部申请的为nSize + RTMP_MAX_HEADER_SIZE
    RTMPPacket_Alloc(&packet, size);
    RTMPPacket_Reset(&packet);

    packet.m_packetType = packet_type;
    if(packet_type == RTMP_PACKET_TYPE_AUDIO)
    {
        packet.m_nChannel = RTMP_AUDIO_CHANNEL;
    }
    if(packet_type == RTMP_PACKET_TYPE_VIDEO)
    {
        packet.m_nChannel = RTMP_VIDEO_CHANNEL;
    }

    packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet.m_hasAbsTimestamp = 1;
    packet.m_nTimeStamp = timestamp;
    packet.m_nInfoField2 = _rtmp->m_stream_id;
    packet.m_nBodySize = size;

    memcpy(packet.m_body, data, size);

    if (!isConnect())
    {
        RTMP_Log(RTMP_LOGERROR, "rtmp is not connect\n");
        return -2;
    }
    if (!RTMP_SendPacket(_rtmp, &packet, 0))
    {
        RTMP_Log(RTMP_LOGERROR, "Send LogError\n");
        return -3;
    }

    RTMPPacket_Free(&packet);
    return 0;
}
