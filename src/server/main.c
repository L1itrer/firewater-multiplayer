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
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_GAMES 5
#define MAX_CONNECTIONS (MAX_GAMES * 2 + 1)

typedef struct Pollfds {
    Pollfd fds[MAX_CONNECTIONS];
    int game_index[MAX_CONNECTIONS];
    int count;
}Pollfds;

void pollfds_add(Pollfds* pollfds, sock_t fd, int game_index)
{
    Pollfd temp = { 0 };
    temp.fd = fd;
    temp.events = POLLIN | POLLHUP;
    pollfds->game_index[pollfds->count] = game_index;
    pollfds->fds[pollfds->count] = temp;
    pollfds->count += 1;
}

void pollfds_remove(Pollfds* pollfds, int index)
{
    pollfds->fds[index] = pollfds->fds[pollfds->count - 1];
    pollfds->count -= 1;
}

typedef struct SingleGameState {
    int player_socket_index, other_socket_index;
}SingleGameState;

typedef struct Games {
    SingleGameState game[MAX_GAMES];
    int count;
}Games;

void server_setup(void)
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

void server_end(void)
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
    if ((adderr = getaddrinfo("0.0.0.0", SERVER_PORT, &hints, info)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(adderr));
        return 1;
    }
    return 0;
}

sock_t server_bind(struct addrinfo *info)
{
    struct addrinfo* ptr;
    sock_t sockfd = 0;
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

        if (bind(sockfd, ptr->ai_addr, (int)ptr->ai_addrlen) == -1)
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

void game_start()
{

}

void handle_new_connection(sock_t serverfd, Pollfds* sockets, Games* games)
{
    struct sockaddr addr = { 0 };
    int addrlen = sizeof(addr);
    sock_t connfd;
    if ((connfd = accept(serverfd, &addr, &addrlen)) == -1)
    {
        fprintf(stderr, "[ERROR]: accept()");
    }

    // i don't know what has to happen for the count to be higher than max connections
    // but i don't wanna tempt fate
    if (sockets->count >= MAX_CONNECTIONS)
    {
        char buffer[8] = { 0 };
        send(connfd, buffer, pack(buffer, "l", (uint32_t)SERVER_FULL), 0);
        socket_close(connfd);
        return;
    }
    printf("[INFO]: New client connected!\n");

    for (int i = 0;i < sockets->count;++i)
    {
        sock_t sock = sockets->fds[i].fd;
        if (sock == serverfd) continue;
        send(sock, "someone joined!", 15, 0);
    }

    // even connections == someone is waiting for a game
    // 
    //if (sockets->count % 2 == 0)
    //{
    //    pollfds_add(sockets, connfd, games->count);
    //    games->game[games->count].other_socket_index = sockets->count - 1; // we just added one
    //    game_start(); // TODO
    //}
    //else
    //{

    //}
}

void handle_client_msg()
{

}

int main(void)
{
    server_setup();
    struct addrinfo* server_info;
    struct sockaddr addr = { 0 };
    Games games = { 0 };
    Pollfds sockets = { 0 };
    if (server_get_addresses(&server_info) != 0) return 1;
    sock_t server_fd = server_bind(server_info);


    if (listen(server_fd, 10) == -1)
    {
        fprintf(stderr, "[ERROR]: listen()");
        return 1;
    }
    freeaddrinfo(server_info);
    socket_setblocking(server_fd, 1);
    pollfds_add(&sockets, server_fd, -1);
    sockets.fds[0].events = POLLIN;

    printf("[INFO]: Server awaiting connections!\n");

    int addrlen = sizeof(addr);
    for (;;)
    {
        int ready_to_read_count = socket_poll(sockets.fds, sockets.count, 50);
        if (ready_to_read_count == SOCKET_ERROR)
        {
            int error_code = WSAGetLastError();

            // Buffer to hold the error message
            char* error_message = NULL;

            // Format the error message
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                error_code,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR)&error_message,
                0,
                NULL
            );
            fprintf(stderr, "%s: %d - %s", "error", error_code, error_message);

            // Free the buffer allocated by FormatMessage
            LocalFree(error_message);
            return 1;
        }
        for (int i = 0;i < sockets.count && ready_to_read_count > 0;++i)
        {
            sock_t sock = sockets.fds[i].fd;
            if ((sockets.fds[i].revents & POLLIN ) || (sockets.fds[i].revents & POLLHUP))
            {
                if (sock == server_fd)
                {
                    // a new available connection
                    handle_new_connection(server_fd, &sockets, &games); //TODO
                }
                else
                {
                    handle_client_msg(); // TODO
                }
                ready_to_read_count -= 1;
            }
        }
    }

    socket_close(server_fd);
    server_end();
    return 0;
}
