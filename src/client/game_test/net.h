#ifndef NET_H
#define NET_H

typedef unsigned long long sock_t;

sock_t server_connect(const char* ip, const char* port);
int server_send(const char* buffer, size_t len);
int server_recv(const char* buffer, size_t max_size);
int server_is_data_available();

#endif