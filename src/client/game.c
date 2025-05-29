#include <game.h>
#include <common.h>
#include <platform.h>
// TODO: REMOVE ANY C LIB FUNCTIONS FROM THIS FILE
#include <stdio.h> // for testing purposes

GameState g_state = {0};

int game_init(const char* ip, const char* port)
{
    // we don't use the connection here, as it's part of the platform
    // we just check if the result is fine
    sock_t conn = server_connect(ip, port);
    if (conn == INVALID_SOCKET) return 1;
    printf("Connected!\n");
    char buffer[128] = {0};
    server_recv(buffer, 128);
    printf("%s\n", buffer);
    return 0;
}

void game_update()
{

}

void game_draw()
{

}
