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
    assert((pollfds->count + 1) <= MAX_CONNECTIONS && "pollfds_add(): trying to add over capacity\n");
    Pollfd temp = { 0 };
    temp.fd = fd;
    temp.events = POLLIN; //| POLLHUP;
    pollfds->game_index[pollfds->count] = game_index;
    pollfds->fds[pollfds->count] = temp;
    pollfds->count += 1;
}

void reset_pollfd(Pollfd* pfd)
{
    pfd->fd = -1;
    pfd->events = 0;
    pfd->revents = 0;
}

void pollfds_remove(Pollfds* pollfds, int index)
{
    assert(index >= 0 && "pollfds_remove(): index lower than 0\n");
    socket_close(pollfds->fds[index].fd);
    int game_index = pollfds->game_index[index];
    pollfds->fds[index] = pollfds->fds[pollfds->count - 1];
    pollfds->game_index[index] = pollfds->game_index[pollfds->count - 1];
    reset_pollfd(&pollfds->fds[pollfds->count-1]);
    pollfds->count -= 1;
}

typedef struct SingleGameState {
    int player_socket_index, other_socket_index;
}SingleGameState;

typedef struct Games {
    SingleGameState game[MAX_GAMES];
    int count;
}Games;

typedef struct ServerState {
    Games games;
    Pollfds pollfds;
}ServerState;

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

void game_start(Pollfds* fds, SingleGameState game)
{
    char buffer[8] = { 0 };
    MessageKind msg = GAME_START;
    pack(buffer, "l", msg);
    int player1 = game.player_socket_index;
    int player2 = game.other_socket_index;
    sock_t player1_sock = fds->fds[player1].fd;
    sock_t player2_sock = fds->fds[player2].fd;
    send(player1_sock, buffer, 4, 0);
    send(player2_sock, buffer, 4, 0);
}

void games_add_player(Pollfds* sockets, Games* games, sock_t connfd)
{
    //we don't count the listening socket
    if ((sockets->count-1) % 2 == 0)
    {
        // even connections == no one is waiting - put the player into the queue
        pollfds_add(sockets, connfd, games->count);
        games->game[games->count].player_socket_index = sockets->count - 1;
        games->game[games->count].other_socket_index = -1;
        games->count += 1;

    }
    else
    {
        // even connections == someone is waiting for a game
        pollfds_add(sockets, connfd, games->count-1);
        games->game[games->count - 1].other_socket_index = sockets->count - 1;
        game_start(sockets, games->game[games->count - 1]);
    }
}

void game_reset(SingleGameState* game)
{
    game->other_socket_index = -1;
    game->player_socket_index = -1;
}

// this goddamn function should:
// 
void games_remove_game(Pollfds* sockets, Games* games, int game_index)
{
    // hey me from the future if you're wondering how this works, don't bother, i don't know
    games->game[game_index] = games->game[games->count - 1];
    sockets->game_index[games->game[game_index].player_socket_index] = game_index;
    sockets->game_index[games->game[game_index].other_socket_index] = game_index;
    game_reset(&games->game[games->count - 1]);
    games->count -= 1;
}

void games_restore_state(Pollfds* pfd, Games* games)
{
    for (int i = 1;i < pfd->count;++i)
    {
        int game_index = pfd->game_index[i];
        if (i % 2 != 0) games->game[game_index].player_socket_index = i;
        else games->game[game_index].other_socket_index = i;
    }
}


void games_remove_player(Pollfds* sockets, Games* games, int index)
{
    int player_game_index = sockets->game_index[index];
    int player_index = games->game[player_game_index].player_socket_index;
    pollfds_remove(sockets, player_index);
    int other_index = games->game[player_game_index].other_socket_index;
    games_remove_game(sockets, games, player_game_index);
    if (other_index != -1) 
    {
        // remove the same index twice because other player index just landed there
        pollfds_remove(sockets, player_index);
    }
    games_restore_state(sockets, games);
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
    //TODO: print some info about the player
    printf("[INFO]: New client connected!\n");

    games_add_player(sockets, games, connfd);


}

int get_other_player_index(Pollfds* sockets, Games* games, int index)
{
    int player_game_index = sockets->game_index[index];
    int player_socket_index = games->game[player_game_index].player_socket_index;
    sock_t player_socket = sockets->fds[player_socket_index].fd;
    sock_t searched_player = sockets->fds[index].fd;
    if (player_socket == searched_player)
    {
        return games->game[player_game_index].other_socket_index;
    }
    else
    {
        return games->game[player_game_index].player_socket_index;
    }
}

void handle_client_msg(Pollfds* sockets, Games* games, int index)
{
    short revents = sockets->fds[index].revents;
    if (revents & POLLHUP)
    {
        printf("[INFO]: Client disconnected!\n");
        //int other_player_index = get_other_player_index(sockets, games, index);
        //pollfds_remove(sockets, index);
        games_remove_player(sockets, games, index);
        
    }
    if (revents & POLLIN)
    {
        // TODO some actual error checking
        char buffer[128] = { 0 };
        sock_t sending_sock = sockets->fds[index].fd;
        int count = recv(sending_sock, buffer, 128, 0);
        int recv_index = get_other_player_index(sockets, games, index);
        sock_t receiving_sock = sockets->fds[recv_index].fd;
        send(receiving_sock, buffer, count, 0);
    }
}

void games_init(Games* games)
{
    for (int i = 0;i < MAX_GAMES;++i)
    {
        games->game[i].other_socket_index = -1;
        games->game[i].player_socket_index = -1;
    }
    games->count = 0;
    
}

int main(void)
{
    server_setup();
    struct addrinfo* server_info;
    struct sockaddr addr = { 0 };
    Games games;
    games_init(&games);
    memset(&games, -1, sizeof(games));
    games.count = 0;
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
        int ready_to_read_count = socket_poll(sockets.fds, sockets.count, 10);
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
                    handle_client_msg(&sockets, &games, i); // TODO
                }
                ready_to_read_count -= 1;
            }
        }
    }

    socket_close(server_fd);
    server_end();
    return 0;
}
