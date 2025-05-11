#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
// TODO: add posix implementation
#endif //WIN32

#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define SERVER_PORT "8080"
#define MAX_GAMES 5
#define MAX_CONNECTIONS MAX_GAMES * 2 + 1


void server_setup()
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
}

void server_end()
{
    // does the existance of function even make sense? idk
#ifdef WIN32
    WSACleanup();
#else
    TODO("posix not yet implemented");
#endif
}

int server_get_addresses(struct addrinfo** info)
{
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int adderr;
    if ((adderr = getaddrinfo(NULL, SERVER_PORT, &hints, info)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(adderr));
        return 1;
    }
    return 0;
}

sock_t server_bind(struct addrinfo *info)
{
    struct addrinfo* ptr;
    sock_t sockfd;
    int yes = 1;
    for (ptr = info;ptr != NULL;ptr = ptr->ai_next)
    {
        if ((sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1)
        {
            perror("[ERROR]: socket()");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int)) == -1)
        {
            perror("[ERROR]: setsockopt()");
            exit(1);
        }

        if (bind(sockfd, ptr->ai_addr, ptr->ai_addrlen) == -1)
        {
            socket_close(sockfd);
            perror("[ERROR]: bind()");
            continue;
        }
        break;
    }
    if (ptr == NULL)
    {
        fprintf(stderr, "[ERROR]: falied to bind\n");
        exit(1);
    }
    return sockfd;
}

int main()
{
    server_setup();
    struct addrinfo* server_info;
    if (server_get_addresses(&server_info) != 0) return 1;
    sock_t server_fd = server_bind(server_info);

    freeaddrinfo(server_info);

    if (listen(server_fd, 10) == -1)
    {
        fprintf(stderr, "[ERROR]: listen()");
        return 1;
    }

    printf("[INFO]: Server awaiting connections!");




    server_end();
    return 0;
}
