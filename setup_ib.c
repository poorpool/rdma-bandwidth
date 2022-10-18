#include "setup_ib.h"
#include "debug.h"
#include "config.h"
#include "sock.h"
#include "ib.h"
#include <malloc.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <infiniband/verbs.h>

struct IBRes ibRes;

int PORT_NUM = 1; // 这啥啊？？


int connectQPAsServer() {
    int ret = 0, peer_fd = 0;
    int fd = sock_create_bind(configInfo.serverPort);
    check(fd > 0, "failed to create and bind socket");
    listen(fd, 5);

    socklen_t sock_len = sizeof(struct sockaddr_in);
    struct sockaddr_in peer_address;
    peer_fd = accept(fd, (struct sockaddr *)&peer_address, &sock_len);
    check(peer_fd > 0, "failed to create peer fd");

    struct QPInfo localQPInfo = {
        .lid = ibRes.portAttr.lid,
        .qpNum = ibRes.qp->qp_num,
        .gidIndex = ibRes.gidIndex,
    }, remoteQPInfo;
    if (localQPInfo.lid == 0 && ibRes.portAttr.link_layer == IBV_LINK_LAYER_ETHERNET) {
        ret = ibv_query_gid(ibRes.ctx, PORT_NUM, localQPInfo.gidIndex, &localQPInfo.gid);
	    check (ret == 0, "failed to get gid");
    }
    ret = sock_read(peer_fd, &remoteQPInfo, sizeof(struct QPInfo));
    check(ret == sizeof(struct QPInfo), "server read qp info from client failed");
    ret = sock_write(peer_fd, &localQPInfo, sizeof(struct QPInfo));
    check(ret == sizeof(struct QPInfo), "server write qp info to client failed");
    
    printQPInfo("local from server", &localQPInfo);
    printQPInfo("remote from server", &remoteQPInfo);

    close(peer_fd);
    close(fd);
    return 0;
error:
    if (peer_fd > 0) {
        close(peer_fd);
    }
    if (fd > 0) {
        close(fd);
    }
    return ret == 0 ? -1 : ret;
}

int connectQPAsClient() {
    int ret = 0;
    int fd = sock_create_connect(configInfo.serverName, configInfo.serverPort);
    check(fd > 0, "failed to create and connect socket");

    struct QPInfo localQPInfo = {
        .lid = ibRes.portAttr.lid,
        .qpNum = ibRes.qp->qp_num,
        .gidIndex = ibRes.gidIndex,
    }, remoteQPInfo;
    if (localQPInfo.lid == 0 && ibRes.portAttr.link_layer == IBV_LINK_LAYER_ETHERNET) {
        ret = ibv_query_gid(ibRes.ctx, PORT_NUM, localQPInfo.gidIndex, &localQPInfo.gid);
	    check (ret == 0, "failed to get gid");
    }
    ret = sock_write(fd, &localQPInfo, sizeof(struct QPInfo));
    check(ret == sizeof(struct QPInfo), "client write qp info to server failed");
    ret = sock_read(fd, &remoteQPInfo, sizeof(struct QPInfo));
    check(ret == sizeof(struct QPInfo), "client read qp info from server failed");
    
    printQPInfo("local from client", &localQPInfo);
    printQPInfo("remote from client", &remoteQPInfo);

    close(fd);
    return 0;
error:
    if (fd > 0) {
        close(fd);
    }
    return ret == 0 ? -1 : ret;
}

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
    if (configInfo.isServer) {
        ret = connectQPAsServer();
    } else {
        ret = connectQPAsClient();
    }
    check(ret == 0, "failed to connect qp");

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