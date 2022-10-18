#include "debug.h"
#include "config.h"

struct ConfigInfo configInfo;

void printConfigInfo() {
    log(LOG_SUB_HEADER, "configuration");

    log("isServer = %s", configInfo.isServer ? "true" : "false");
    log("msgSize = %d", configInfo.msgSize);
    log("numConcurrMsgs = %d", configInfo.numConcurrMsgs);
    log("devIndex = %d", configInfo.devIndex);
    log("gidIndex = %d", configInfo.gidIndex);
    if (configInfo.isServer) {
        log("listenPort = %d", configInfo.serverPort);
    } else {
        log("serverPort = %d", configInfo.serverPort);
        log("serverName = %s", configInfo.serverName);
    }
    
    log(LOG_SUB_HEADER, "end of configuration");
}