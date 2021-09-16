#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <iomanip>
#include <sys/time.h>
#include "prefix_tree.h"
#include "poptrie.h"

using namespace std;

void read_ftb(vector<forward_table_entry_t> &ftb)
{
    string fname("forwarding-table.txt");
    ifstream ifs(fname);
    if (ifs.is_open())
    {
        char ip_str[16];
        uint32_t ip_uint;
        uint32_t prefix_len;
        uint32_t port;
        while (ifs >> ip_str >> prefix_len >> port)
        {
            ip_uint = ip_str_to_uint(ip_str);
            ftb.emplace_back(forward_table_entry_t(ip_uint, prefix_len, port));
        }
    }
    else
    {
        cerr << "Open " << fname << " failed." << endl;
    }
}

void performance_check(uint32_t round = 1000000)
{
    vector<uint32_t> test_ips;
    default_random_engine gen;
    uniform_int_distribution<uint32_t> dist(1, 0xffffffff);
    for (uint32_t i = 0; i < round; ++i)
    {
        test_ips.emplace_back(dist(gen));
    }
    struct timeval start, end;
    double lookup_cost_in_ns;
    uint32_t mem_usage = poptrie_mem_usage();
    uint32_t mem_B = mem_usage % 1024;
    uint32_t mem_KB = (mem_usage / 1024) % 1024;
    uint32_t mem_MB = (mem_usage / 1024 / 1024) % 1024;
    gettimeofday(&start, NULL);
    for (uint32_t i = 0; i < round; ++i)
    {
        poptrie_find(test_ips[i]);
    }
    gettimeofday(&end, NULL);
    lookup_cost_in_ns = ((double)(end.tv_sec - start.tv_sec)) * 1e9;
    lookup_cost_in_ns += ((double)(end.tv_usec - start.tv_usec)) * 1e3;
    lookup_cost_in_ns /= round;
    cout
        << "Performance of poptrie: "
        << endl
        << "memory usage: "
        << mem_MB
        << " MB "
        << mem_KB
        << " KB "
        << mem_B
        << " B "
        << endl
        << "Time cost per lookup: "
        << setprecision(7)
        << lookup_cost_in_ns
        << " ns"
        << endl;
    mem_usage = prefix_tree_mem_usage();
    mem_B = mem_usage % 1024;
    mem_KB = (mem_usage / 1024) % 1024;
    mem_MB = (mem_usage / 1024 / 1024) % 1024;
    gettimeofday(&start, NULL);
    for (uint32_t i = 0; i < round; ++i)
    {
        prefix_tree.find(test_ips[i]);
    }
    gettimeofday(&end, NULL);
    lookup_cost_in_ns = ((double)(end.tv_sec - start.tv_sec)) * 1e9;
    lookup_cost_in_ns += ((double)(end.tv_usec - start.tv_usec)) * 1e3;
    lookup_cost_in_ns /= round;
    cout
        << "Performance of naive prefix tree: "
        << endl
        << "memory usage: "
        << mem_MB
        << " MB "
        << mem_KB
        << " KB "
        << mem_B
        << " B "
        << endl
        << "Time cost per lookup: "
        << setprecision(7)
        << lookup_cost_in_ns
        << " ns"
        << endl;
}

int main()
{
    cout << "Building poptrie" << endl;
    poptrie_build_init();
    vector<forward_table_entry_t> ftb;
    read_ftb(ftb);
    build_poptrie(ftb);
    cout << "Poptrie built" << endl;

    cout << "Building naive prefix tree" << endl;
    build_prefix_tree();
    cout << "Naive prefix tree built" << endl;

    // poptrie_correctness_test(ftb);

    performance_check();

    poptire_finalize();

    return 0;
}