/*
 *  这是一个拉流的主程序
*/
#include <stdio.h>
#include "mediabase.h"
#include <iostream>
#include <thread>
#include <chrono>
#include "pullwork.h"

#undef main

#define RTMP_URL "rtmp://192.168.1.177:1935/live/chen"

int main(int argc, char *argv[])
{
    //初始化参数
    Properties properties;

    //设置视频调试参数
    properties.SetProperty("rtmp_debug", 1);
    properties.SetProperty("rtmp_debug_file", "test_me.flv");



    //设置url
    properties.SetProperty("rtmp_url", RTMP_URL);

    //推流类
    PullWork pullwork;
    if(pullwork.init(properties) != 0) {
        printf("push work init fail\n");
        return 0;
    }

    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    printf("结束\n");
    return 0;
}
