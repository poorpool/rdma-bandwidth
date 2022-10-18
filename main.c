#include "debug.h"
#include "config.h"
#include "setup_ib.h"
#include "cs.h"
#include <stdio.h>
#include <stdlib.h>

FILE *log_fp = NULL; // 日志文件指针

int initEnv() {
    if (configInfo.isServer) {
        log_fp = fopen("server.log", "w");
    } else {
        log_fp = fopen("client.log", "w");
    }
    check(log_fp != NULL, "failed to open log file");

    log(LOG_HEADER, configInfo.isServer ? "server" : "client");
    printConfigInfo();
    return 0;
error:
    return -1;
}

void destroyEnv() {
    if (log_fp != NULL) {
        fclose(log_fp);
    }
    log_fp = NULL;
}

int main(int argc, char *argv[]) {
    int ret = 0;

    if (argc == 6) {
        configInfo.isServer = true;
        configInfo.msgSize = atoi(argv[1]);
        configInfo.numConcurrMsgs = atoi(argv[2]);
        configInfo.devIndex = atoi(argv[3]);
        configInfo.gidIndex = atoi(argv[4]);
        configInfo.serverPort = atoi(argv[5]);
    } else if (argc == 7) {
        configInfo.isServer = false;
        configInfo.msgSize = atoi(argv[1]);
        configInfo.numConcurrMsgs = atoi(argv[2]);
        configInfo.devIndex = atoi(argv[3]);
        configInfo.gidIndex = atoi(argv[4]);
        configInfo.serverPort = atoi(argv[5]);
        configInfo.serverName = argv[6];
    } else {
        printf("usage:\n");
        printf("server: %s msg_size num_concurr_msgs dev_index gid_index listen_port\n", argv[0]);
        printf("client: %s msg_size num_concurr_msgs dev_index gid_index server_port server_name\n", argv[0]);
        return 0;
    }

    ret = initEnv();
    check(ret == 0, "failed to init env");

    ret = setupIB();

    if (configInfo.isServer) {
        ret = server_func();
    } else {
        ret = client_func();
    }

    destroyEnv();
    return 0;
error:
    // todo: fill it
    destroyEnv();
    return ret;
}

