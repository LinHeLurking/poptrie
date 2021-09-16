#pragma once

#include <vector>
#include <queue>
#include <cstdint>

#include "ftb.h"

extern std::vector<forward_table_entry_t> forward_table;

class prefix_tree_node_t
{
public:
    bool is_match_node = false;
    prefix_tree_node_t *next[4] = {};
    int32_t ftb_idx = -1;
};

class prefix_tree_t
{
public:
    prefix_tree_node_t *root;

    void add_node(uint32_t subnet_ip, uint32_t prefix_len, uint32_t port);
    int32_t find(uint32_t ip);

    prefix_tree_t()
    {
        root = new prefix_tree_node_t;
    }
    ~prefix_tree_t()
    {
        std::queue<prefix_tree_node_t *> clean_queue;
        clean_queue.push(root);
        while (!clean_queue.empty())
        {
            auto cur = clean_queue.front();
            clean_queue.pop();
            for (int i = 0; i < 4; ++i)
            {
                if (cur->next[i] != nullptr)
                {
                    clean_queue.push(cur->next[i]);
                }
            }
            delete cur;
        }
    }
};

extern prefix_tree_t prefix_tree;

void build_prefix_tree();

uint32_t prefix_tree_mem_usage();
