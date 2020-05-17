#ifndef MEDIABASE_H
#define MEDIABASE_H

#include <map>
#include <string.h>

class mediabase
{
public:
    mediabase();
};


class Properties: public std::map<std::string, std::string>
{
public:
    //下面是set函数
    void SetProperty(const std::string &key,const std::string &val)
    {
        insert(std::pair<std::string, std::string>(key,val));
    }

    void SetProperty(const char* key, const char* val)
    {
        SetProperty(std::string(key), std::string(val));
    }

    void SetProperty(const char* key, int intval)
    {
        SetProperty(std::string(key), std::to_string(intval));
    }


    //下面是get函数
    std::string GetProperty(std::string &key, char* str)
    {
        GetProperty(key);
    }

    std::string GetProperty(std::string &key, std::string &str)
    {
        GetProperty(key);
    }

    std::string GetProperty(const char* key, const std::string defaultValue)
    {

    }

    int GetProperty(const char* key, int defaultValue) const
    {
        std::string key_s(key);
        std::string str = GetProperty(key_s);

        if(str.empty()) {
            return defaultValue;
        }

        return atoi(str.c_str());
    }

    std::string GetProperty(const char* key, const char* defaultstr) const
    {
        std::string key_s(key);
        std::string str = GetProperty(key_s);

        if(str.empty()) {
            return defaultstr;
        }

        return str;
    }

    std::string GetProperty(std::string &key) const
    {
        std::string str = "";
        const_iterator it = find(key);

        if(it == end()) {
            return str;
        }

        return it->second;
    }

};




#endif // MEDIABASE_H
