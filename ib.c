#include "ib.h"
#include "debug.h"
#include <stdio.h>

// copied

int modifyQP2RTS (struct ibv_qp *qp, struct QPInfo *local, struct QPInfo *remote) {
    int ret = 0;

    /* change QP state to INIT */
    {
        struct ibv_qp_attr qp_attr = {
            .qp_state = IBV_QPS_INIT,
            .pkey_index = 0,
            .port_num = IB_PORT,
            .qp_access_flags = IBV_ACCESS_LOCAL_WRITE |
                    IBV_ACCESS_REMOTE_READ |
                    IBV_ACCESS_REMOTE_ATOMIC |
                    IBV_ACCESS_REMOTE_WRITE,
        };

        ret = ibv_modify_qp(qp, &qp_attr,
                IBV_QP_STATE | IBV_QP_PKEY_INDEX |
                IBV_QP_PORT  | IBV_QP_ACCESS_FLAGS);
        check(ret == 0, "Failed to modify qp to INIT.");
    }
    
    
    /* Change QP state to RTR */
    {
        struct ibv_qp_attr  qp_attr = {
            .qp_state           = IBV_QPS_RTR,
            .path_mtu           = IBV_MTU_1024,
            .dest_qp_num        = remote->qpNum,
            .rq_psn             = 0,
            .max_dest_rd_atomic = 1,
            .min_rnr_timer      = 12,
            .ah_attr.is_global  = 0,
            .ah_attr.dlid       = remote->lid,
            .ah_attr.sl         = IB_SL,
            .ah_attr.src_path_bits = 0,
            .ah_attr.port_num      = IB_PORT,
        };

        if (remote->lid == 0) {
            printf("using gid\n");
            qp_attr.ah_attr.is_global = 1;
            qp_attr.ah_attr.grh.sgid_index = local->gidIndex;
            qp_attr.ah_attr.grh.dgid = remote->gid;
            qp_attr.ah_attr.grh.hop_limit = 0xFF;
            qp_attr.ah_attr.grh.traffic_class = 0;
        }
        ret = ibv_modify_qp(qp, &qp_attr,
                    IBV_QP_STATE | IBV_QP_AV |
                    IBV_QP_PATH_MTU | IBV_QP_DEST_QPN |
                    IBV_QP_RQ_PSN |
                    IBV_QP_MAX_DEST_RD_ATOMIC |
                    IBV_QP_MIN_RNR_TIMER |
                    0);
        check (ret == 0, "Failed to change qp to rtr.");
    }
	
    /* Change QP state to RTS */
    {
        struct ibv_qp_attr  qp_attr = {
            .qp_state      = IBV_QPS_RTS,
            .timeout       = 14,
            .retry_cnt     = 7,
            .rnr_retry     = 7,
            .sq_psn        = 0,
            .max_rd_atomic = 1,
        };

        ret = ibv_modify_qp (qp, &qp_attr,
                    IBV_QP_STATE | IBV_QP_TIMEOUT |
                    IBV_QP_RETRY_CNT | IBV_QP_RNR_RETRY |
                    IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC);
        check (ret == 0, "Failed to modify qp to RTS.");
    }
	
    return 0;
 error:
    return -1;
}

void printQPInfo(const char *hint, struct QPInfo *qwq) {
    printf("%s\n", hint);
    printf("lid %d\n", qwq->lid);
    printf("qpNum %d\n", qwq->qpNum);
    printf("gidIndex %d\n", qwq->gidIndex);
}