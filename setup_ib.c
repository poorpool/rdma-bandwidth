#include "setup_ib.h"
#include "debug.h"
#include "config.h"
#include <malloc.h>

struct IBRes ibRes;

int PORT_NUM = 1; // 这啥啊？？

int setupIB() {
    int ret = 0;
    struct ibv_device **dev_list;
    
    // 1. Get IB context
    dev_list = ibv_get_device_list(NULL);
    check(dev_list != NULL, "failed to get device list");
    ibRes.ctx = ibv_open_device(dev_list[configInfo.devIndex]);
    check(ibRes.ctx != NULL, "failed to open ib device")

    printf("device idx %d, name %s, dev_name %s\n", configInfo.devIndex, ibRes.ctx->device->name,
            ibRes.ctx->device->dev_name);

    // 2. Allocate IB protection domain
    ibRes.pd = ibv_alloc_pd(ibRes.ctx);
    check(ibRes.pd != NULL, "failed to allocate pd")

    // 3. Register IB memory region
    ibRes.ibBufSize = configInfo.msgSize * configInfo.numConcurrMsgs;
    ibRes.ibBuf = (char *)memalign(4096, ibRes.ibBufSize);
    check(ibRes.ibBuf != NULL, "failed to allocate buffer");
    ibRes.mr = ibv_reg_mr(ibRes.pd, (void *)ibRes.ibBuf, ibRes.ibBufSize,
            IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE);
    check(ibRes.mr != NULL, "failed to register mr");

    // 4. Create Completion Queue
    ret = ibv_query_device(ibRes.ctx, &ibRes.devAttr); // query device attr
    check(ret == 0, "failed to query device attr");
    ibRes.cq = ibv_create_cq(ibRes.ctx, ibRes.devAttr.max_cqe, NULL, NULL, 0);
    check(ibRes.cq != NULL, "failed to create cq");

    // 5. Create Queue Pair
    struct ibv_qp_init_attr qp_init_attr = {
        .send_cq = ibRes.cq,
        .recv_cq = ibRes.cq,
        .cap = {
            .max_recv_wr = 1024,
            .max_send_wr = 1024,
            .max_send_sge = 1,
            .max_recv_sge = 1
        },
        .qp_type = IBV_QPT_RC,
    };
    ibRes.qp = ibv_create_qp(ibRes.pd, &qp_init_attr);
    check(ibRes.qp != NULL, "failed to create qp");

    // 6. Query IB port attribute
    ret = ibv_query_port(ibRes.ctx, PORT_NUM, &ibRes.portAttr);
    check(ret == 0, "failed to query port attr");
    ibRes.gidIndex = configInfo.gidIndex;
    printf("device %s has %d gids\n", ibRes.ctx->device->name, ibRes.portAttr.gid_tbl_len);
    printf("max mtu %d\n", ibRes.portAttr.max_mtu);

    // 7. Connect QP

    // 8. free
    ibv_free_device_list(dev_list);
    return 0;
error:
    if (dev_list != NULL) {
        ibv_free_device_list(dev_list);
        dev_list = NULL;
    }
    return ret == 0 ? -1 : ret;
}