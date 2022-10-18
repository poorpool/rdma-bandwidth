#ifndef POORPOOL_SOCK_H
#define POORPOOL_SOCK_H

extern int sock_create_bind(int port);

extern int sock_create_connect(char *addr, int port);

extern int sock_read(int fd, void *buf, size_t len);

extern int sock_write(int fd, void *buf, size_t len);

#endif