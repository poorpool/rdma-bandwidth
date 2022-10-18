#define _GNU_SOURCE
#include "debug.h"
#include "cs.h"
#include "config.h"
#include "ib.h"
#include "setup_ib.h"
#include <malloc.h>
#include <infiniband/verbs.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>

int client_func() {
    int ret = 0;
    int last_n = 0;
    int num_wc = 20; // work complete num
    struct ibv_wc *wc = NULL;
    
    // pre-post recv
    wc = (struct ibv_wc *)calloc (num_wc, sizeof(struct ibv_wc));
    check(wc != NULL, "failed to alloc wc array");

    char *bufPtr = ibRes.ibBuf;
    int bufOffset = 0;
    for (int i = 0; i < configInfo.numConcurrMsgs; i++) {
        ret = post_recv(configInfo.msgSize, ibRes.mr->lkey, (uint64_t)bufPtr, ibRes.qp, bufPtr);
        check(ret == 0, "failed to post recv");
        bufOffset = (bufOffset + configInfo.msgSize) % ibRes.ibBufSize;
        bufPtr = ibRes.ibBuf + bufOffset;
    }

    sprintf(ibRes.ibBuf, "message %d", 0);
    post_send (configInfo.msgSize, ibRes.mr->lkey, 0, 0, ibRes.qp, ibRes.ibBuf);

    while (1) {
        int n = ibv_poll_cq(ibRes.cq, num_wc, wc);
        if (n < 0) {
            check(0, "failed to poll cq");
        }
        for (int i = 0; i < n; i++) {
            if (wc[i].status != IBV_WC_SUCCESS) {
                if (wc[i].opcode == IBV_WC_SEND) {
                    check(0, "send failed status: %s", ibv_wc_status_str(wc[i].status));
                } else {
                    check(0, "recv failed status: %s", ibv_wc_status_str(wc[i].status));
                }
            }
            if (wc[i].opcode == IBV_WC_RECV) {
                char *msg_ptr = (char *)wc[i].wr_id;
                printf("client receive %s\n", msg_ptr);
                sscanf(msg_ptr, "message %d", &last_n);
                sprintf(msg_ptr, "message %d", last_n+1);
                sleep(1);
		        post_send (configInfo.msgSize, ibRes.mr->lkey, 0, 0, ibRes.qp, msg_ptr);

                post_recv (configInfo.msgSize, ibRes.mr->lkey, wc[i].wr_id, ibRes.qp, msg_ptr);
	        }
        }
    }

    return 0;
error:
    if (wc != NULL) {
        free(wc);
    }
    return -1;
}
