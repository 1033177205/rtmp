#include "pushwork.h"


PushWork::PushWork()
{

}

bool PushWork::init(const Properties& properties)
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

    _rtmp_pusher = new RtmpPusher();       //构造函数里面在初始化
    if(_rtmp_pusher == NULL) {
        printf("rtmppusher is null\n");
        return -2;
    }

    _rtmp_pusher->connect(_rtmp_url);

    if(_rtmp_debug == 1)  {
        test_push();            //内部死循环，一直发送推流数据
    }


    return 0;
}

int PushWork::test_push()
{
    FLVAnalyze flv_analyze(_rtmp_debug_file);
    FLVTag tag;
    int ret = -1;

    while(1)
    {
        if((ret = flv_analyze.get_FLVTag(tag, 0)) != 0) {
            if(ret == -100)
                return -1;
            continue;
        }

        _rtmp_pusher->sendPacket(tag.heard.type, tag.data, tag.heard.datasize, tag.heard.time);

        flv_analyze.free_FLVTag_data(tag);
    }
    return 0;
}
