#include "pullwork.h"

PullWork::PullWork()
{

}


int PullWork::init(const Properties& properties)
{
    _rtmp_debug = properties.GetProperty("rtmp_debug", 0);
    printf("_rtmp_debug %d\n", _rtmp_debug);

    _rtmp_debug_file = properties.GetProperty("rtmp_debug_file", "dd.flv");
    printf("_rtmp_debug   %s\n", _rtmp_debug_file.c_str());

    _rtmp_url = properties.GetProperty("rtmp_url", "");
    printf("_url   %s\n", _rtmp_url.c_str());
    if(_rtmp_url.empty()) {
        printf("rtmp is null\n");
        return -1;
    }

    _rtmp_player = new RtmpPlayer();       //构造函数里面在初始化
    if(_rtmp_player == NULL) {
        printf("_rtmp_player is null\n");
        return -2;
    }

    _rtmp_player->connect(_rtmp_url);

    if(_rtmp_debug == 1)  {
        test_pull();                    //内部死循环，一直发送推流数据
    }

    return 0;
}

int PullWork::test_pull()
{
    FILE* file;
    int bufferSize = 64 * 1024;
    char *buffer = (char *) malloc(bufferSize);
    int nRead = 0;
    off_t size = 0;

    file = fopen(_rtmp_debug_file.c_str(), "w+b");
    if (file == 0)
    {
        printf("Failed to open file! %s\n", _rtmp_debug_file.c_str());
        return -1;
    }

    do
    {
        nRead = _rtmp_player->readPacket(buffer, bufferSize);
        printf("nRead: %d\n", nRead);
        if (nRead > 0)
        {
            if (fwrite(buffer, sizeof(unsigned char), nRead, file) != (size_t) nRead)
            {
                printf("%s: Failed writing, exiting!", __FUNCTION__);
                free(buffer);
                return -2;
            }
            size += nRead;
        }
    }
    while ( nRead > -1 && _rtmp_player->isConnect());

    fclose(file);
    return 0;
}
