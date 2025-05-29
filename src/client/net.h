#ifndef NET_H
#define NET_H
#include <common.h>

/**  @brief Connect to a server given an ip
 *   @return A valid sock_t on success, an INVALID_SOCKET on failure
*/
sock_t server_connect(const char* ip, const char* port);

int server_send(const char* buffer, size_t len);
int server_recv(const char* buffer, size_t max_size);

#endif // !NET_H

