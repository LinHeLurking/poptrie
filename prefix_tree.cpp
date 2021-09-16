#include <iostream>
#include <fstream>

#include "prefix_tree.h"

std::vector<forward_table_entry_t> forward_table(0);

prefix_tree_t prefix_tree;

uint32_t __node_cnt = 1;

void prefix_tree_t::add_node(uint32_t subnet_ip, uint32_t prefix_len, uint32_t port)
{
    uint32_t seg[16];
    for (int i = 0; i < 16; ++i)
    {
        uint32_t mask = 0b11 << (2 * i);
        seg[i] = (subnet_ip & mask) >> (2 * i);
    }

    uint32_t remain = prefix_len;
    prefix_tree_node_t *cur = root;
    for (int i = 15; i >= 0 && remain > 0; --i)
    {
        if (remain == 1)
        {
            uint32_t a = seg[i] | 0b1;
            uint32_t b = a ^ 0b1;
            // If there's existing node, you should not cover it with shorter prefix
            if (cur->next[a] == nullptr)
            {
                cur->next[a] = new prefix_tree_node_t;
                __node_cnt += 1;
            }
            else
            {
                a = 0xffffffff;
            }
            if (cur->next[b] == nullptr)
            {
                cur->next[b] = new prefix_tree_node_t;
                __node_cnt += 1;
            }
            else
            {
                b = 0xffffffff;
            }
            if (a != 0xffffffff || b != 0xffffffff)
            {
                auto entry = forward_table_entry_t(subnet_ip, prefix_len, port);
                forward_table.push_back(entry);
                if (a != 0xffffffff)
                {
                    cur->next[a]->is_match_node = true;
                    cur->next[a]->ftb_idx = forward_table.size() - 1;
                }
                if (b != 0xffffffff)
                {
                    cur->next[b]->is_match_node = true;
                    cur->next[b]->ftb_idx = forward_table.size() - 1;
                }
            }

            remain -= 1;
        }
        else if (remain == 2)
        {
            uint32_t id = seg[i];
            if (cur->next[id] == nullptr)
            {
                cur->next[id] = new prefix_tree_node_t;
                __node_cnt += 1;
            }
            cur->next[id]->is_match_node = true;
            auto entry = forward_table_entry_t(subnet_ip, prefix_len, port);
            forward_table.push_back(entry);
            cur->next[id]->ftb_idx = forward_table.size() - 1;
            remain -= 2;
        }
        else
        {
            uint32_t id = seg[i];
            if (cur->next[id] == nullptr)
            {
                cur->next[id] = new prefix_tree_node_t;
                __node_cnt += 1;
            }
            cur = cur->next[id];
            remain -= 2;
        }
    }
}

int32_t prefix_tree_t::find(uint32_t ip)
{
    uint32_t seg[16];
    for (int i = 0; i < 16; ++i)
    {
        uint32_t mask = 0b11 << (2 * i);
        seg[i] = (ip & mask) >> (2 * i);
    }

    int32_t ret = -1;
    prefix_tree_node_t *cur = root;
    for (int i = 15; i >= 0; --i)
    {
        uint32_t id = seg[i];
        if (cur->is_match_node)
        {
            ret = cur->ftb_idx;
        }
        if (cur->next[id] == nullptr)
        {
            break;
        }
        cur = cur->next[id];
    }
    return ret;
}

void build_prefix_tree()
{
    char ip_str[16];
    uint32_t prefix_len, port;
    std::ifstream ifs("forwarding-table.txt");
    if (ifs.is_open())
    {
        while (ifs >> ip_str >> prefix_len >> port)
        {
            uint32_t ip = ip_str_to_uint(ip_str);
            prefix_tree.add_node(ip, prefix_len, port);
        }
    }
    else
    {
        std::cerr << "Open file failed" << std::endl;
    }
}

uint32_t prefix_tree_mem_usage()
{
    return (__node_cnt * sizeof(prefix_tree_node_t));
}
