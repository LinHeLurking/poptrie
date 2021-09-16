#pragma once

#include <string>
#include <cstdint>

class forward_table_entry_t
{
public:
    uint32_t subnet_ip = 0;
    uint32_t mask = 0;
    uint32_t port = 0;
    uint32_t prefix_len = 0;

    forward_table_entry_t() {}
    forward_table_entry_t(uint32_t _subnet_ip, uint32_t _prefix_len, uint32_t _port) : subnet_ip(_subnet_ip), port(_port), prefix_len(_prefix_len)
    {
        uint32_t tail = (1 << (32 - _prefix_len)) - 1;
        mask = ~tail;
    }
};

uint32_t ip_str_to_uint(const char *ip_str);
uint32_t ip_str_to_uint(const std::string &ip_str);

std::string ip_unit_to_str(uint32_t ip);
