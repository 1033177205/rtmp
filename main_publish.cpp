/*
 *  这是一个推流的主程序
*/
#include <stdio.h>
#include "mediabase.h"
#include "PushWork.h"
#include <iostream>
#include <thread>
#include <chrono>

#undef main


//srs docker 启动命令：
// systemctl start docker
// docker run -it -d -p 1935:1935 1033177205/srs:v0


int main(int argc, char *argv[])
{
//接收rtmp的地址
    printf("hello rtmp \n%d %s\n", argc, argv[1]);

    if(argc != 2) {
        printf("参数错误\n");
        return -1;
    }

//    char *url = "rtmp://192.168.1.177:1935/live/chen";

    //初始化参数
    Properties properties;

    //设置视频调试参数
    properties.SetProperty("rtmp_debug", 1);
    properties.SetProperty("rtmp_debug_file", "test.flv");



    //设置url
    properties.SetProperty("rtmp_url", "rtmp://192.168.1.177:1935/live/chen");

    //推流类
    PushWork pushwork;
    if(pushwork.init(properties) != 0) {
        printf("push work init fail\n");
        return 0;
    }

    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    printf("结束\n");
    return 0;
}
