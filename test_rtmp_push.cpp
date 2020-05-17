#include <stdio.h>
#include <librtmp/log.h>
#include "librtmp/rtmp_sys.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#define RD_SUCCESS          0
#define RD_FAILED           1
#define RD_INCOMPLETE		2

enum RTMPChannel
{
   RTMP_NETWORK_CHANNEL = 2,   ///< channel for network-related messages (bandwidth report, ping, etc)
   RTMP_SYSTEM_CHANNEL,        ///< channel for sending server control messages
   RTMP_AUDIO_CHANNEL,         ///< channel for audio data
   RTMP_VIDEO_CHANNEL   = 6,   ///< channel for video data
   RTMP_SOURCE_CHANNEL  = 8,   ///< channel for a/v invokes
};

#define HTON16(x)  ((x>>8&0xff)|(x<<8&0xff00))
#define HTON24(x)  ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00))
#define HTON32(x)  ((x>>24&0xff)|(x>>8&0xff00)|\
    (x<<8&0xff0000)|(x<<24&0xff000000))
#define HTONTIME(x) ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00)|(x&0xff000000))

static int64_t get_current_time_msec()
{
#ifdef _WIN32
    return (int64_t)GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((int64_t)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
#endif
}

/*read 1 byte*/
static int ReadU8(uint32_t *u8, FILE*fp)
{
    if (fread(u8, 1, 1, fp) != 1)
        return 0;
    return 1;
}
/*read 2 byte*/
static int ReadU16(uint32_t *u16, FILE*fp)
{
    if (fread(u16, 2, 1, fp) != 1)
        return 0;
    *u16 = HTON16(*u16);
    return 1;
}
/*read 3 byte*/
static int ReadU24(uint32_t *u24, FILE*fp)
{
    if (fread(u24, 3, 1, fp) != 1)
        return 0;
    printf("U24 0x%x\n", *u24);
    *u24 = HTON24(*u24);
    return 1;
}
/*read 4 byte*/
static int ReadU32(uint32_t *u32, FILE*fp)
{
    if (fread(u32, 4, 1, fp) != 1)
        return 0;
    *u32 = HTON32(*u32);
    return 1;
}
/*read 1 byte,and loopback 1 byte at once*/
static int PeekU8(uint32_t *u8, FILE*fp)
{
    if (fread(u8, 1, 1, fp) != 1)
        return 0;
    fseek(fp, -1, SEEK_CUR);
    return 1;
}

/*read 4 byte and convert to time format*/
static int ReadTime(uint32_t *utime, FILE*fp)
{
    if (fread(utime, 4, 1, fp) != 1)
        return 0;
    *utime = HTONTIME(*utime);
    return 1;
}

// starts sockets
static int InitSockets()
{
#ifdef WIN32
  WORD version;
  WSADATA wsaData;

  version = MAKEWORD(1, 1);
  return (WSAStartup(version, &wsaData) == 0);
#else
  return TRUE;
#endif
}

static inline void CleanupSockets()
{
#ifdef WIN32
  WSACleanup();
#endif
}

int test_rtmp_push()
{
   char *flv_name = "test.flv";
   char *url = "rtmp://192.168.1.177:1935/live/chen";

    FILE *fp = NULL;
    RTMP rtmp = { 0 };
    uint32_t preTagsize = 0;

    //packet attributes
    uint32_t type = 0;
    uint32_t datalength = 0;
    uint32_t timestamp = 0;
    uint32_t streamid = 0;


   fp = fopen(flv_name, "rb");
   if(!fp) {
        RTMP_LogPrintf("Open File LogError.\n");
        return -1;
    }

    /* set log level */
    //RTMP_LogLevel loglvl = RTMP_Log; // RTMP_LOGALL;
    //RTMP_LogSetLevel(loglvl);

    //第一步
    if (!InitSockets())
    {
        RTMP_Log(RTMP_LOGERROR, "Couldn't load sockets support on your platform, exiting!");
        return -RD_FAILED;
    }

    //第二步
    RTMP_Init(&rtmp);

    //第三步，通过url设置参数
    if (!RTMP_SetupURL(&rtmp, url))
    {
        RTMP_Log(RTMP_LOGERROR, "SetupURL Err\n");
        CleanupSockets();
        return -1;
    }
    RTMP_Log(RTMP_LOGERROR, "RTMP_SetBufferMS ---------->\n");
    RTMP_SetBufferMS(&rtmp, 10000);

    //第四步
    // RTMP推流需要EnableWrite
    RTMP_Log(RTMP_LOGERROR, "RTMP_EnableWrite ---------->\n");
    RTMP_EnableWrite(&rtmp);
    RTMP_Log(RTMP_LOGERROR, "RTMP_Connect ---------->\n");
    if (!RTMP_Connect(&rtmp, NULL))
    {
        RTMP_Log(RTMP_LOGERROR, "Connect Err\n");
        CleanupSockets();
        return -1;
    }
    RTMP_Log(RTMP_LOGERROR, "RTMP_ConnectStream ---------->\n");
    if (!RTMP_ConnectStream(&rtmp, 0))
    {
        RTMP_Log(RTMP_LOGERROR, "ConnectStream Err\n");
        RTMP_Close(&rtmp);
        CleanupSockets();
        return -1;
    }

    RTMP_Log(RTMP_LOGERROR, "RTMP_ConnectStream ok. ---------->\n");


    //第五步，申请消息内存
    RTMPPacket *packet = NULL;
    packet = (RTMPPacket*)malloc(sizeof(RTMPPacket));
    // 为包申请了buffer, 实际在内部申请的为nSize + RTMP_MAX_HEADER_SIZE
    RTMPPacket_Alloc(packet, 1024 * 1024);
    RTMPPacket_Reset(packet);

    //第六步，发送
    //jump over FLV Header
    fseek(fp, 9, SEEK_SET);			// 跳过FLV header
    //jump over previousTagSizen
    fseek(fp, 4, SEEK_CUR);			// 跳过 previousTagSizen
    packet->m_nInfoField2 = rtmp.m_stream_id;

    int continue_sleep_count = 0;
    uint32_t audio_start_timestamp = 0;
    uint32_t audio_current_timestamp = 0;
    int h264_frame_count = 0;
    uint32_t start_time = 0;
    uint32_t now_time = 0;
    start_time = (uint32_t)get_current_time_msec();
    while(1)
    {
        if ((((now_time = (uint32_t)get_current_time_msec()) - start_time)
             <(audio_current_timestamp - audio_start_timestamp)))
        {
            Sleep(30);
            if(continue_sleep_count++ > 30)
            {
//                RTMP_Log("continue_sleep_count=%d timeout, time:%ums, aud:%ums\n",
//                               continue_sleep_count,
//                               (uint32_t)get_current_time_msec() - start_time,
//                               audio_current_timestamp - audio_start_timestamp);
            }
            continue;
        }
        continue_sleep_count = 0;
        //not quite the same as FLV spec
        if (!ReadU8(&type, fp))			// 读取tag类型
        {
            RTMP_Log(RTMP_LOGERROR, "%s(%d) break %d\n", __FUNCTION__, __LINE__, type);
            break;
        }
        datalength = 0;
        if (!ReadU24(&datalength, fp))	// 负载数据长度
        {
            RTMP_Log(RTMP_LOGERROR, "%s(%d) break\n", __FUNCTION__, __LINE__);
            break;
        }
        printf("datalength = %d %d\n", datalength, type);
        if (!ReadTime(&timestamp, fp))
        {
            RTMP_Log(RTMP_LOGERROR, "%s(%d) break\n", __FUNCTION__, __LINE__);
            break;
        }
        if (!ReadU24(&streamid, fp))
        {
            RTMP_Log(RTMP_LOGERROR, "%s(%d) break\n", __FUNCTION__, __LINE__);
            break;
        }

        if (type != RTMP_PACKET_TYPE_AUDIO && type != RTMP_PACKET_TYPE_VIDEO)
        {
            RTMP_Log(RTMP_LOGERROR, "unknown type:%d", type);
            //jump over non_audio and non_video frame，
            //jump over next previousTagSizen at the same time
            fseek(fp, datalength + 4, SEEK_CUR);
            continue;
        }

        if (type == RTMP_PACKET_TYPE_AUDIO)
        {
            if(audio_start_timestamp == 0)
            {
                audio_start_timestamp = timestamp;
                start_time = (uint32_t)get_current_time_msec();
            }
            if(timestamp <  audio_current_timestamp)    //回绕？做重置
            {
                audio_start_timestamp = 0;
                start_time = (uint32_t)get_current_time_msec();
                RTMP_Log(RTMP_LOGWARNING, "%s(%d) timestamp rollback %u->%ums\n",
                         __FUNCTION__, __LINE__, audio_current_timestamp, timestamp);

            }
            audio_current_timestamp = timestamp;

        }

        size_t read_len = 0;
        if ((read_len = fread(packet->m_body, 1, datalength, fp)) != datalength)
        {
            RTMP_Log(RTMP_LOGERROR, "fread error, read_len:%d, datalength:%d\n", datalength);
            break;
        }

        if(type == RTMP_PACKET_TYPE_AUDIO)
        {
            packet->m_nChannel = RTMP_AUDIO_CHANNEL;
        }
        if(type == RTMP_PACKET_TYPE_VIDEO)
        {
            packet->m_nChannel = RTMP_VIDEO_CHANNEL;
        }

        packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
        packet->m_hasAbsTimestamp = 1;
        packet->m_nTimeStamp = timestamp;
        packet->m_packetType = type;
        packet->m_nBodySize = datalength;
        //pre_frame_time = timestamp;

        if (!RTMP_IsConnected(&rtmp))
        {
            RTMP_Log(RTMP_LOGERROR, "rtmp is not connect\n");
            break;
        }
        if (!RTMP_SendPacket(&rtmp, packet, 0))
        {
            RTMP_Log(RTMP_LOGERROR, "Send LogError\n");
            break;
        }

        if (!ReadU32(&preTagsize, fp))
        {
            RTMP_Log(RTMP_LOGERROR, "%s(%d) break\n", __FUNCTION__, __LINE__);
            break;
        }
        if (type == RTMP_PACKET_TYPE_VIDEO)
        {
            h264_frame_count++;
            if (h264_frame_count % 50 == 0)
            {
                printf("t = %d, frame_count = %d, time:%ums, aud:%ums\n",
                       timestamp, h264_frame_count,
                       (uint32_t)get_current_time_msec() - start_time,
                       audio_current_timestamp - audio_start_timestamp);
            }
        }
    }


    //第七步清场，退出
    if (fp)
        fclose(fp);

    RTMP_Close(&rtmp);

    CleanupSockets();

    return 0;
}

