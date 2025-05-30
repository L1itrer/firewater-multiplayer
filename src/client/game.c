#include <game.h>
#include <common.h>
#include <platform.h>
// TODO: REMOVE ANY C LIB FUNCTIONS FROM THIS FILE
#include <stdio.h> // for testing purposes

Game g_game = {0};

int game_init(const char* ip, const char* port)
{
    // we don't use the connection here, as it's part of the platform
    // we just check if the result is fine
    sock_t conn = server_connect(ip, port);
    if (conn == INVALID_SOCKET) return 0;
    printf("Connected!\n");
    return 1;
}

void game_update()
{
    for (int i = 0;i < 2;++i)
    {
        Player* player = &g_game.player[i];
        if (player->left)
        {
            player->position.x -= MOVEMENT_SPEED;
        }
        if (player->right)
        {
            player->position.x += MOVEMENT_SPEED;
        }
        if (player->jump)
        {
            player->position.y += 20.0;
            player->jump = false;
        }
    }
}

void game_draw()
{
    draw_rectangle(
        (Rect)
        {
            .x = g_game.player[PLAYER_LOCAL].position.x,
            .y = g_game.player[PLAYER_LOCAL].position.y,
            .width = 50.0,
            .height = 50.0
        },
        (ColorHSV)
        {
            .hue = 0.0,
            .saturation = 0.8,
            .value = 0.8
        }
    );
}

void game_poll()
{
    char buffer[128] = {0};
    poll_keyboard();
    if (false && server_is_data_available())
    {
        server_recv(buffer, 128);
        MessageKind message;
        unpack(buffer, "l", &message);
        switch (message)
        {
            case GAME_START:
                break;
            case PLAYER_MOVING:
                break;
            default:
                UNREACHABLE();
                break;
        }
    }
}

void key_change(int key_code, int player, bool keydown)
{
    assert(player == PLAYER_LOCAL || player == PLAYER_NETWORK && "Unexpected player!\n");
    switch (key_code)
    {
        case 'a':
            g_game.player[player].left = keydown;
            break;
        case 'd':
            g_game.player[player].right = keydown;
            break;
        case 's':
            g_game.player[player].jump = keydown;
            break;
        default:
            UNREACHABLE();
            break;
    }
}