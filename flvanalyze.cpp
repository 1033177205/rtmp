#include "flvanalyze.h"
#include "stdlib.h"

//flv格式解析
#define HTON16(x)  ((x>>8&0xff)|(x<<8&0xff00))
#define HTON24(x)  ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00))
#define HTON32(x)  ((x>>24&0xff)|(x>>8&0xff00)|\
    (x<<8&0xff0000)|(x<<24&0xff000000))
#define HTONTIME(x) ((x>>16&0xff)|(x<<16&0xff0000)|(x&0xff00)|(x&0xff000000))

/*read 1 byte*/
int ReadU8(uint32_t *u8, FILE*fp)
{
    if (fread(u8, 1, 1, fp) != 1)
        return 0;
    return 1;
}
/*read 2 byte*/
int ReadU16(uint32_t *u16, FILE*fp)
{
    if (fread(u16, 2, 1, fp) != 1)
        return 0;
    *u16 = HTON16(*u16);
    return 1;
}
/*read 3 byte*/
int ReadU24(uint32_t *u24, FILE*fp)
{
    if (fread(u24, 3, 1, fp) != 1)
        return 0;
    printf("U24 0x%x\n", *u24);
    *u24 = HTON24(*u24);
    return 1;
}
/*read 4 byte*/
int ReadU32(uint32_t *u32, FILE*fp)
{
    if (fread(u32, 4, 1, fp) != 1)
        return 0;
    *u32 = HTON32(*u32);
    return 1;
}
/*read 1 byte,and loopback 1 byte at once*/
int PeekU8(uint32_t *u8, FILE*fp)
{
    if (fread(u8, 1, 1, fp) != 1)
        return 0;
    fseek(fp, -1, SEEK_CUR);
    return 1;
}

/*read 4 byte and convert to time format*/
int ReadTime(uint32_t *utime, FILE*fp)
{
    if (fread(utime, 4, 1, fp) != 1)
        return 0;
    *utime = HTONTIME(*utime);
    return 1;
}

FLVAnalyze::FLVAnalyze(std::string& file) : _file(file)
{
    _fp = fopen(_file.c_str(), "rb");
}

FLVAnalyze::~FLVAnalyze()
{
    fclose(_fp);
}

int FLVAnalyze::get_FLVTag(FLVTag& tag, unsigned int mode)  //记得调用释放内存函数
{
    static int one = 1;
    uint32_t type = 0;
    uint32_t datalength = 0;
    uint32_t timestamp = 0;
    uint32_t streamid = 0;
    uint32_t preTagsize = 0;
    static int file_end = 0;

    if(one == 1) {
        one = 0;
        fseek(_fp, 9, SEEK_SET);
        fseek(_fp, 4, SEEK_CUR);
    }

    if(mode == 0) {   //mode = 0 说明是循环读取flv tag数据
        if (!ReadU8(&type, _fp))			// 读取tag类型
        {
            printf("%s(%d) break %d\n", __FUNCTION__, __LINE__, type);
            file_end++;
            if(file_end >= 10) {
                one = 0;
                return -100;
            }
            return -1;
        }
        file_end = 0;
        datalength = 0;
        if (!ReadU24(&datalength, _fp))	// 负载数据长度
        {
            printf("%s(%d) break\n", __FUNCTION__, __LINE__);
            return -2;
        }
        printf("datalength = %d %d\n", datalength, type);
        if (!ReadTime(&timestamp, _fp))
        {
            printf("%s(%d) break\n", __FUNCTION__, __LINE__);
            return -3;
        }
        if (!ReadU24(&streamid, _fp))
        {
            printf("%s(%d) break\n", __FUNCTION__, __LINE__);
            return -4;
        }

        tag.heard.type = (FLV_TAG_TYPE_E)type;
        tag.heard.datasize = datalength;
        tag.heard.time = timestamp;
        tag.heard.stream_id = streamid;

        if (type != FLV_TAG_AUDIO_DATA && type != FLV_TAG_VIDEO_DATA)
        {
            printf("unknown type:%d", type);
            //jump over non_audio and non_video frame，
            //jump over next previousTagSizen at the same time
            fseek(_fp, datalength + 4, SEEK_CUR);
            return -5;
        }


        tag.data = (unsigned char*)calloc(sizeof(char), datalength);
        size_t read_len = 0;
        if ((read_len = fread(tag.data, 1, datalength, _fp)) != datalength)
        {
            printf("fread error, read_len:%d, datalength:%d\n", datalength);
            return -6;
        }

        if (!ReadU32(&preTagsize, _fp))
        {
            printf("%s(%d) break\n", __FUNCTION__, __LINE__);
            return -7;
        }
    }

    return 0;
}

void FLVAnalyze::free_FLVTag_data(FLVTag& tag)
{
    free(tag.data);
    tag.data = NULL;
}
