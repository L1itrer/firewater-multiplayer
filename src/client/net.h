#ifndef NET_H
#define NET_H
#include <common.h>

sock_t server_connect(const char* ip, const char* port);
int server_send(sock_t serverfd, const char* buffer, size_t len);
int server_recv(sock_t serverfd, const char* buffer, size_t max_size);

#endif // !NET_H

