#include "rtmpbase.h"

rtmpbase::rtmpbase()
{
    init();
}

rtmpbase::~rtmpbase()
{
    RTMP_Free(_rtmp);
    _rtmp = NULL;

    cleanupSockets();
}

rtmpbase::rtmpbase(RTMP_WORK_TYPE rtmp_work):_rtmp_work(rtmp_work)
{
    init();
}

bool rtmpbase::initSockets()
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

void rtmpbase::cleanupSockets()
{
#ifdef WIN32
  WSACleanup();
#endif
}



bool rtmpbase::init()
{
    int ret = -1;
    ret = initSockets();

    _rtmp = RTMP_Alloc();
    if(_rtmp == NULL) {
        ret = -1;
    }
    RTMP_Init(_rtmp);
    return ret ? FALSE : TRUE;
}


bool rtmpbase::connect()
{
    if(_url.empty()) {
        printf("url null\n");
        return FALSE;
    }
    //第三步，通过url设置参数
    if (!RTMP_SetupURL(_rtmp, (char*)_url.c_str()))
    {
        RTMP_Log(RTMP_LOGERROR, "SetupURL Err\n");
        cleanupSockets();
        return FALSE;
    }
    RTMP_Log(RTMP_LOGINFO, "RTMP_SetBufferMS ---------->\n");
    RTMP_SetBufferMS(_rtmp, 10000);

    //第四步
    if(_rtmp_work == RTMP_WORK_TYPE_PUSH)
    {
         // RTMP推流需要EnableWrite
        RTMP_Log(RTMP_LOGINFO, "RTMP_EnableWrite ---------->\n");
        RTMP_EnableWrite(_rtmp);
    }
    RTMP_Log(RTMP_LOGINFO, "RTMP_Connect ---------->\n");
    if (!RTMP_Connect(_rtmp, NULL))
    {
        RTMP_Log(RTMP_LOGERROR, "Connect Err\n");
        cleanupSockets();
        return -1;
    }
    RTMP_Log(RTMP_LOGINFO, "RTMP_ConnectStream ---------->\n");
    if (!RTMP_ConnectStream(_rtmp, 0))
    {
        RTMP_Log(RTMP_LOGERROR, "ConnectStream Err\n");
        close();
        cleanupSockets();
        return -1;
    }

    RTMP_Log(RTMP_LOGWARNING, "RTMP_ConnectStream ok. ---------->\n");
}

bool rtmpbase::connect(char* url)
{
    this->_url = url;
    return connect();
}

bool rtmpbase::connect(std::string& url)
{
    this->_url = url;
    return connect();
}

bool rtmpbase::isConnect()
{
    return RTMP_IsConnected(_rtmp);
}

void rtmpbase::close()
{
    RTMP_Close(_rtmp);
}

