#ifndef POORPOOL_CONFIG_H
#define POORPOOL_CONFIG_H

#include <stdbool.h>

struct ConfigInfo {
    bool isServer;
    int msgSize;
    int numConcurrMsgs;
    int devIndex; // ibv_devinfo 查看有多少个设备，为设备的序号
    int gidIndex; // show_gids 查看
    char *serverName; // 对于客户端来说，它的服务端的名字
    int serverPort; // 对于客户端是服务端端口，对于服务端是监听端口
};

extern struct ConfigInfo configInfo;

void printConfigInfo();

#endif