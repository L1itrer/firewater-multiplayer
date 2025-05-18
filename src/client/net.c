#include <common.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#ifndef WIN32
#define INVALID_SOCKET -1
#endif

sock_t server_connect(const char* ip, const char* port)
{
   #ifdef WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
    {
        fprintf(stderr, "WSAStartup: failure\n");
        exit(1);
    }
    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
    {
        fprintf(stderr, "WSAStartup: version 2.2 unavailable. somehow.\n");
        WSACleanup();
        exit(1);
    }

#else
    TODO("posix not yet implemented");
#endif
    struct addrinfo hints = { 0 };
    struct addrinfo* serverinfo, * ptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    sock_t serverfd = 0;
    int rv, yes=1;
    if (rv = getaddrinfo(ip, port, &hints, &serverinfo))
    {
        fprintf(stderr, "[ERROR]: getaddrinfo(): %s\n", gai_strerror(rv));
        exit(1);
    }
    for (ptr = serverinfo;ptr != NULL; ptr = ptr->ai_next)
    {
        if ((serverfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == INVALID_SOCKET)
        {
            perror("[ERROR]: socket()");
            continue;
        }
        //if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int)) == -1)
        //{
        //    perror("[ERROR]: setsockopt()");
        //    continue;
        //}
        if (connect(serverfd, ptr->ai_addr, ptr->ai_addrlen) == -1)
        {
            int errorCode = WSAGetLastError();
            socket_close(serverfd);
            serverfd = 0;
            perror("[ERROR]: connect()");
            continue;
        }
    }
    if (serverfd == 0)
    {
        fprintf(stdout, "[INFO]: Could not connect to server");
        exit(1);
    }
    return serverfd;

}
int server_send(sock_t serverfd, const char* buffer, size_t len)
{
  return send(serverfd, buffer, len, 0);
}
int server_recv(sock_t serverfd, const char* buffer, size_t max_size)
{
  return recv(serverfd, buffer, (int) max_size, 0);
}

