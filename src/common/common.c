#include <common.h>
int add(int a, int b)
{
  return a + b;
}





#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

int socket_close(sock_t sockfd)
{
    return closesocket(sockfd);
}

#else
int socket_close(sock_t sockfd)
{
    return close(sockfd);
}

#endif