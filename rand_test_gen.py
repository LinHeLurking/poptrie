#!/usr/bin/env python3

from random import randint


def ip_i_to_str(ip):
    ret = ""
    i = 3
    while i >= 0:
        mask = ((1 << 8)-1) << (8*i)
        ip_msk = (ip & mask) >> (8*i)
        ret += str(ip_msk)
        if i != 0:
            ret += "."
        i -= 1
    return ret


def gen(size=50):
    with open("ftb_test.txt", 'w') as f:
        for _ in range(size):
            test_ip = randint(1, 0xffffffff-1)
            plen = randint(6, 24)
            port = randint(1, 50)
            ip_str = ip_i_to_str(test_ip)
            f.write("{} {} {}\n".format(ip_str, plen, port))


if __name__ == "__main__":
    gen()
