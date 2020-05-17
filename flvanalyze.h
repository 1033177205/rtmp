#ifndef FLVANALYZE_H
#define FLVANALYZE_H

#include <iostream>
#include <stdio.h>

enum FLV_TAG_TYPE_E
{
    FLV_TAG_SCRIPT_DATA = 0x12,
    FLV_TAG_AUDIO_DATA = 0x08,
    FLV_TAG_VIDEO_DATA = 0x09,
};


class FLVHearder
{
public:
    unsigned char name[3];       //表示FLV格式
    unsigned char version;       //flv版本
    unsigned char typeFlag;      //前5位为保留位，必须为0，第6位表示是否存在音频tag，第7位为保留位，必须为0，第8位表示是否存在视频tag。
    unsigned int  dataOffest;    //4个字节，表示Header长度，版本1的长度固定为9
};


class FLVTagHearder
{
public:
    FLV_TAG_TYPE_E type;
    unsigned int  datasize;     //3个字节，表示tag的data部分的大小
    unsigned int  time;         //4个字节，单位ms，相对值，第一个时间戳为0
    unsigned int  stream_id;    //Stream ID：3个字节，总为0
};

class FLVTag
{
public:
    FLVTagHearder heard;
    unsigned char* data;
};

class FLVAnalyze
{
public:
    FLVAnalyze(std::string& file);
    ~FLVAnalyze();
    int get_FLVHearder(FLVHearder& hear);
    int get_FLVTag(FLVTag& tag, unsigned int mode = 0);
    void free_FLVTag_data(FLVTag& tag);

private:
    std::string _file;
    FILE* _fp;
};

#endif // FLVANALYZE_H
