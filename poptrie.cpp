#include <queue>
#include <algorithm>
#include <string>
#include <cstring>
#include <iostream>
#include <random>
#include "poptrie.h"

leaf *leaves = nullptr;
uint32_t __leaf_used = 0;
uint32_t __leaf_max = 0;

node *nodes = nullptr;
uint32_t __node_used = 0;
uint32_t __node_max = 0;

node_tmp *tmp_root;

uint32_t *direct_point = nullptr;

void poptrie_build_init()
{
    tmp_root = new node_tmp;
    direct_point = new uint32_t[4096];
}

void build_poptrie(const std::vector<forward_table_entry_t> &ftb)
{
    for (uint32_t i = 0; i < ftb.size(); ++i)
    {
        uint32_t ip = ftb[i].subnet_ip & ftb[i].mask;
        uint32_t prefix_len = ftb[i].prefix_len;
        uint32_t port = ftb[i].port;
        uint32_t remain_prefix = prefix_len;
        uint32_t offset = 0;
        node_tmp *cur = tmp_root;
        // Follow or create internal nodes
        while (remain_prefix > 6)
        {
            uint32_t v = extract(ip, offset, 6);
            if (cur->vector & (1ULL << v))
            {
            }
            else
            {
                cur->vector |= (1ULL << v);
                cur->child[v] = new node_tmp;
                cur->child[v]->father = cur;
                // Copy current exsisting leaves down
                uint32_t cur_leaf_port = cur->leaves[v]->port;
                uint32_t cur_prefix_len = cur->leaves[v]->prefix_len;
                for (int k = 0; k < 64; ++k)
                {
                    cur->child[v]->leaves[k]->port = cur_leaf_port;
                    cur->child[v]->leaves[k]->prefix_len = cur_prefix_len;
                }
            }
            cur = cur->child[v];
            offset += 6;
            remain_prefix -= 6;
        }
        // Create leaf
        uint32_t v = extract(ip, offset, 6);
        uint32_t st = v;
        uint32_t ed = v + (1 << (6 - remain_prefix));
        for (uint32_t j = st; j < ed; ++j)
        {
            if (cur->leaves[j]->prefix_len > prefix_len)
            {
                continue;
            }
            cur->leaves[j]->port = port;
            cur->leaves[j]->prefix_len = prefix_len;

            if (cur->child[j] != nullptr)
            {
                // Copy current exsisting leaves down
                std::queue<node_tmp *> q;
                q.push(cur->child[j]);
                while (!q.empty())
                {
                    node_tmp *_cur = q.front();
                    q.pop();
                    for (int k = 0; k < 64; ++k)
                    {
                        if (_cur->child[k] != nullptr)
                        {
                            q.push(_cur->child[k]);
                        }
                        else if (_cur->leaves[k]->port == 0xffffffff)
                        {
                            _cur->leaves[k]->port = port;
                            _cur->leaves[k]->prefix_len = prefix_len;
                        }
                    }
                }
            }
        }
    }
    poptrie_tmp_style_flattern();
    leaf_compress_optimize();
    direct_pointing_optimize();
}

void leaf_compress_optimize()
{
    std::queue<uint32_t> q;
    q.push(0);
    while (!q.empty())
    {
        node *cur = &nodes[q.front()];
        q.pop();
        uint32_t last_port = 0xfefefefe;
        uint32_t last_prefix_len = 0;
        cur->leafvec = ~(cur->vector);
        for (uint32_t i = 0; i < 64; ++i)
        {
            if (cur->vector & (1ULL << i))
            {
                uint32_t bc = popcnt(cur->vector & ((2ULL << i) - 1));
                uint32_t idx = cur->base1 + bc - 1;
                q.push(idx);
            }
            else
            {
                uint32_t bc = popcnt((~(cur->vector)) & ((2ULL << i) - 1));
                uint32_t idx = cur->base0 + bc - 1;
                if (last_port == 0xfefefefe)
                {
                    continue;
                }
                if (leaves[idx].port == last_port &&
                    leaves[idx].prefix_len == last_prefix_len)
                {
                    cur->leafvec &= ~(1ULL << i);
                }
                else
                {
                    last_port = leaves[idx].port;
                    last_prefix_len = leaves[idx].prefix_len;
                }
            }
        }
    }
}

void direct_pointing_optimize()
{
    for (uint32_t key = 0; key < 4096; ++key)
    {
        node *cur = &nodes[0];
        uint32_t ip = key << 20;
        uint32_t v = extract(ip, 0, 6);
        if (cur->vector & (1ULL << v))
        {
            uint32_t bc = popcnt(cur->vector & ((2ULL << v) - 1));
            cur = &nodes[cur->base1 + bc - 1];
            v = extract(ip, 6, 6);
        }
        if (cur->vector & (1ULL << v))
        {
            // Add an internal node to dp
            uint32_t bc = popcnt(cur->vector & ((2ULL << v) - 1));
            direct_point[key] = (cur->base1 + bc - 1);
        }
        else
        {
            // Add a leaf to dp
            uint32_t bc = popcnt(cur->leafvec & ((2ULL << v) - 1));
            direct_point[key] = (cur->base0 + bc - 1) | (1UL << 31);
        }
    }
}

// Match th ip in poptrie, return corresponding port.
// -1 is returned if there's no match.
int32_t poptrie_find(uint32_t ip)
{
    uint32_t direct_key = ip >> 20;
    uint32_t dp = direct_point[direct_key];
    if (dp & (1UL << 31))
    {
        return leaves[dp & ((1ULL << 31) - 1)].port;
    }
    node *cur = &nodes[dp];
    uint32_t offset = 12;
    uint32_t v = extract(ip, offset, 6);
    while (cur->vector & (1ULL << v))
    {
        uint32_t bc = popcnt(cur->vector & ((2ULL << v) - 1));
        cur = &nodes[cur->base1 + bc - 1];
        offset += 6;
        v = extract(ip, offset, 6);
    }
    uint32_t bc = popcnt(cur->leafvec & ((2ULL << v) - 1));
    return leaves[cur->base0 + bc - 1].port;
}

int32_t poptrie_find(const std::string &ip_str)
{
    return poptrie_find(ip_str_to_uint(ip_str));
}

int32_t poptrie_find(const char *ip_str)
{
    return poptrie_find(ip_str_to_uint(ip_str));
}

void poptrie_tmp_style_flattern()
{
    std::queue<node_tmp *> q;
    q.push(tmp_root);
    uint32_t internal_node_cnt = 1;
    uint32_t leaf_cnt = 0;
    while (!q.empty())
    {
        node_tmp *cur = q.front();
        q.pop();
        for (int i = 0; i < 64; ++i)
        {
            if (cur->child[i] != nullptr)
            {
                q.push(cur->child[i]);
                internal_node_cnt += 1;
            }
            else
            {
                leaf_cnt += 1;
            }
        }
    }
    __node_max = internal_node_cnt;
    __leaf_max = leaf_cnt;
    nodes = new node[__node_max];
    leaves = new leaf[__leaf_max];
    __node_used = __leaf_used = 0;
    poptrie_node_flattern();
    poptrie_tmp_free();
}

void poptrie_node_flattern()
{
    std::queue<node_tmp *> node_q;
    std::queue<uint32_t> idx_q;
    node_q.push(tmp_root);
    idx_q.push(__node_used);
    __node_used += 1;
    while (!node_q.empty())
    {
        node_tmp *cur = node_q.front();
        uint32_t idx = idx_q.front();
        node_q.pop();
        idx_q.pop();
        nodes[idx].base1 = __node_used;
        nodes[idx].base0 = __leaf_used;
        nodes[idx].vector = cur->vector;
        for (int i = 0; i < 64; ++i)
        {
            if (cur->child[i] != nullptr)
            {
                node_q.push(cur->child[i]);
                idx_q.push(__node_used);
                __node_used += 1;
            }
            else
            {
                leaves[__leaf_used] = *(cur->leaves[i]);
                __leaf_used += 1;
            }
        }
    }
}

void poptrie_tmp_free()
{
    std::queue<node_tmp *> q;
    q.push(tmp_root);
    while (!q.empty())
    {
        node_tmp *cur = q.front();
        q.pop();
        for (int i = 0; i < 64; ++i)
        {
            if (cur->leaves[i] != nullptr)
            {
                delete (cur->leaves[i]);
            }
            if (cur->child[i] != nullptr)
            {
                q.push(cur->child[i]);
            }
        }
        delete cur;
    }
}

void poptire_finalize()
{
    delete[] nodes;
    delete[] leaves;
    delete[] direct_point;
}

void poptrie_correctness_test(const std::vector<forward_table_entry_t> ftb)
{
    std::cout << "Testing poptrie using random generated ip" << std::endl;
    std::default_random_engine gen;
    std::uniform_int_distribution<uint32_t> dist(1, 0xffffffff);
    uint32_t correct = 0;
    uint32_t wrong = 0;
    for (int i = 0; i < 1000; ++i)
    {
        uint32_t test_ip = dist(gen);
        int32_t result = poptrie_find(test_ip);
        int32_t ans = -1;
        uint32_t last_prefix_len = 0;
        for (auto &it : ftb)
        {
            if ((it.subnet_ip & it.mask) == (test_ip & it.mask))
            {
                if (it.prefix_len > last_prefix_len)
                {
                    last_prefix_len = it.prefix_len;
                    ans = it.port;
                }
            }
        }
        if (ans != result)
        {
            std::cerr << "Error with IP "
                      << ip_unit_to_str(test_ip)
                      << ", found "
                      << result
                      << ", expected "
                      << ans
                      << std::endl;
            wrong += 1;
        }
        else
        {
            correct += 1;
        }
    }
    std::cout << "Poptrie test end. Checked: "
              << correct + wrong
              << ", Correct: "
              << correct
              << ", Wrong: "
              << wrong
              << std::endl;
}

uint32_t poptrie_mem_usage()
{
    return (__node_used * sizeof(node) + __leaf_used * sizeof(leaf));
}
