#include "debug.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

int sock_create_bind(int port) {
    int ret = 0;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    check(fd > 0, "failed to create socket");
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    ret = bind(fd, (struct sockaddr *)&address, sizeof(address));
    check(ret == 0, "failed to bind socket");
    return fd;
error:
    if (fd > 0) {
        close(fd);
    }
    return ret == 0 ? -1 : ret;
}

int sock_create_connect(char *addr, int port) {
    int ret = 0;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    check(fd > 0, "failed to create socket");
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(addr);
    ret = connect(fd, (struct sockaddr *)&address, sizeof(address));
    check(ret == 0, "failed to connect socket");
    return fd;
error:
    if (fd > 0) {
        close(fd);
    }
    return ret == 0 ? -1 : ret;
}

int sock_read(int fd, void *buf, size_t len) {
    int ret = 0;
    char *buffer = buf;
    while (len != 0) {
        ssize_t real_len = read(fd, buffer, len);
        if (real_len == 0) {
            break;
        } else if (real_len < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                return ret;
            }
        } else {
            len -= real_len;
            buffer += real_len;
            ret += real_len;
        }
    }
    return ret;
}

int sock_write(int fd, void *buf, size_t len) {
    int ret = 0;
    char *buffer = buf;
    while (len != 0) {
        ssize_t real_len = write(fd, buffer, len);
        if (real_len <= 0) {
            if (errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        } else {
            len -= real_len;
            buffer += real_len;
            ret += real_len;
        }
    }
    return ret;
}
