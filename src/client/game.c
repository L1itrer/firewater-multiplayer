#include <game.h>
#include <common.h>
#include <platform.h>

GameState g_state = {0};

int game_init(const char* ip, const char* port)
{
    // we don't use the connection here, as it's part of the platform
    // we just check if the result is fine
    sock_t conn = server_connect(ip, port);
    if (conn == INVALID_SOCKET) return 1;
    return 0;
}

void game_update()
{

}

void game_draw()
{

}
