#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "ftb.h"

uint32_t popcnt(uint64_t k);

inline uint32_t popcnt(uint64_t k)
{
    uint64_t ret;
    __asm__ volatile(
        "popcnt %0, %1"
        : "=g"(ret)
        : "r"(k)
        :);
    return (uint32_t)ret;
}

class node
{
public:
    uint32_t base0 = 0xffffffff; // used for indexing leaves
    uint32_t base1 = 0xffffffff; // used for indexing nodes
    uint64_t vector = 0;
    uint64_t leafvec = 0;
};

class leaf
{
public:
    uint32_t port = 0xffffffff;
    uint32_t prefix_len = 0;
};

class node_tmp
{
public:
    uint64_t vector = 0;
    node_tmp *father = nullptr;
    node_tmp *child[64];
    leaf *leaves[64];
    node_tmp()
    {
        std::fill(child, child + 64, nullptr);
        for (int i = 0; i < 64; ++i)
        {
            this->leaves[i] = new leaf;
        }
    }
    ~node_tmp() {}
};

void poptrie_build_init();

void build_poptrie(const std::vector<forward_table_entry_t> &ftb);

int32_t poptrie_find(uint32_t ip);

int32_t poptrie_find(const char *ip_str);

int32_t poptrie_find(const std::string &ip_str);

void leaf_compress_optimize();

void direct_pointing_optimize();

uint32_t extract(uint32_t val, uint32_t offset, uint32_t len);

inline uint32_t extract(uint32_t val, uint32_t offset, uint32_t len)
{
    uint32_t tail_len = 32 - offset - len;
    uint32_t mask = ((1UL << len) - 1) << tail_len;
    return ((val & mask) >> tail_len);
}

void poptrie_tmp_style_flattern();
void poptrie_node_flattern();
void poptrie_tmp_free();

void poptire_finalize();

void poptrie_correctness_test(const std::vector<forward_table_entry_t> ftb);

uint32_t poptrie_mem_usage();