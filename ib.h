#ifndef POORPOOL_IB_H
#define POORPOOL_IB_H

#include <inttypes.h>
#include <infiniband/verbs.h>

#define IB_PORT 1
#define IB_SL 0

struct QPInfo {
    uint16_t lid;
    uint32_t qpNum;
    union ibv_gid gid;
    uint8_t gidIndex;
};

extern void printQPInfo(const char *hint, struct QPInfo *qwq);

extern int modifyQP2RTS (struct ibv_qp *qp, struct QPInfo *local, struct QPInfo *remote);

extern int post_send (uint32_t req_size, uint32_t lkey, uint64_t wr_id,
	       uint32_t imm_data, struct ibv_qp *qp, char *buf);

extern int post_recv (uint32_t req_size, uint32_t lkey, uint64_t wr_id, 
	       struct ibv_qp *qp, char *buf);

#endif