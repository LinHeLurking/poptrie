#include "ftb.h"

uint32_t ip_str_to_uint(const char *ip_str)
{
    uint32_t ip = 0;
    uint32_t cur_part = 0;
    for (int i = 0; ip_str[i]; ++i)
    {
        if (ip_str[i] == '.')
        {
            ip = (ip << 8) + cur_part;
            cur_part = 0;
        }
        else if (ip_str[i] >= '0' && ip_str[i] <= '9')
        {
            cur_part = cur_part * 10 + ip_str[i] - '0';
        }
    }
    ip = (ip << 8) + cur_part;
    return ip;
}

uint32_t ip_str_to_uint(const std::string &ip_str)
{
    uint32_t ip = 0;
    uint32_t cur_part = 0;
    for (uint32_t i = 0; i < ip_str.size(); ++i)
    {
        if (ip_str[i] == '.')
        {
            ip = (ip << 8) + cur_part;
            cur_part = 0;
        }
        else if (ip_str[i] >= '0' && ip_str[i] <= '9')
        {
            cur_part = cur_part * 10 + ip_str[i] - '0';
        }
    }
    ip = (ip << 8) + cur_part;
    return ip;
}

std::string ip_unit_to_str(uint32_t ip)
{
    std::string ret;
    for (int i = 3; i >= 0; --i)
    {
        uint32_t seg_i;
        uint32_t mask = 255 << (i * 8);
        seg_i = ((ip & mask) >> (i * 8));
        ret += std::to_string(seg_i);
        if (i != 0)
        {
            ret += ".";
        }
    }
    return ret;
}