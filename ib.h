#ifndef POORPOOL_IB_H
#define POORPOOL_IB_H

#include <inttypes.h>
#include <infiniband/verbs.h>

struct QPInfo {
    uint16_t lid;
    uint32_t qpNum;
    union ibv_gid gid;
    uint8_t gidIndex;
};

extern void printQPInfo(const char *hint, struct QPInfo *qwq);

#endif