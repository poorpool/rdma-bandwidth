#ifndef POORPOOL_SETUP_IB_H
#define POORPOOL_SETUP_IB_H

#include <infiniband/verbs.h>

struct IBRes {
    struct ibv_context *ctx;
    struct ibv_pd *pd; // protect domain
    struct ibv_mr *mr; // memory region
    struct ibv_cq *cq;
    struct ibv_qp *qp;
    struct ibv_port_attr portAttr; // 连接 qp 用
    struct ibv_device_attr devAttr; // 创建 cq 用

    uint8_t gidIndex;

    char *ibBuf;
    size_t ibBufSize;
};

extern struct IBRes ibRes; // 理解为一个统一的信息存放中心

extern int setupIB(); // 设置 rdma 基本信息

#endif