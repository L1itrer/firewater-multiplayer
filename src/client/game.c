#include <game.h>
#include <common.h>
#include <platform.h>
#include <stdint.h>
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
    g_game.player[PLAYER_LOCAL].position = (Vec2){.x = 100.0f, .y = 100.0f};
    g_game.player[PLAYER_NETWORK].position = (Vec2){.x = 100.0f, .y = 500.0f};

    return 1;
}

void game_play()
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
            player->position.y += 5.0f;
            player->jump = false;
        }
    }
}


void game_update()
{
    switch (g_game.state)
    {
        case GS_AWAITING_CONNECTION:
            break;
        case GS_GAME_PLAYING:
            game_play();
            break;
    }
}

void game_draw_players()
{
    platform_draw_rectangle(
        (Rect)
        {
            .x = g_game.player[PLAYER_LOCAL].position.x,
            .y = g_game.player[PLAYER_LOCAL].position.y,
            .width = 50.0f,
            .height = 50.0f
        },
        (ColorHSV)
        {
            .hue = 0.0f,
            .saturation = 0.8f,
            .value = 0.8f
        }
    );
        platform_draw_rectangle(
        (Rect)
        {
            .x = g_game.player[PLAYER_NETWORK].position.x,
            .y = g_game.player[PLAYER_NETWORK].position.y,
            .width = 50.0f,
            .height = 50.0f
        },
        (ColorHSV)
        {
            .hue = 210.0f,
            .saturation = 0.8f,
            .value = 0.8f
        }
    );
}

void game_draw()
{
    switch (g_game.state)
    {
        case GS_AWAITING_CONNECTION:
            platform_draw_text("Awaiting Connection", 100, 200, 56, 
                (ColorHSV){.hue=0.0f, .saturation=0.0f, .value=0.9f});
            break;
        case GS_GAME_PLAYING:
            game_draw_players();
            break;
    }


}
void game_start()
{
    g_game.state = GS_GAME_PLAYING;
}

void game_poll()
{
    if (g_game.state != GS_AWAITING_CONNECTION) platform_poll_keyboard();
    if (server_is_data_available())
    {
        char buffer[128] = {0};
        server_recv(buffer, 128);
        MessageKind message;
        unpack(buffer, "l", &message);
        switch (message)
        {
            case GAME_START:
                game_start();
                break;
            case PLAYER_MOVING:
                Direction direction;
                bool keydown;
                unpack(buffer, "llc", &message, &direction, &keydown);
                key_change(direction, PLAYER_NETWORK, keydown);
                break;
            default:
                // UNREACHABLE();
                break;
        }
    }
}

void send_key_change(int key_code, bool keydown)
{
    MessageKind msg = PLAYER_MOVING;
    Direction dir = key_code;
    char buffer[128] = {0};
    int count = pack(buffer, "llc", msg, dir, keydown);
    server_send(buffer, count);
}

void key_change(int key_code, int player, bool keydown)
{
    assert(player == PLAYER_LOCAL || player == PLAYER_NETWORK && "Unexpected player!\n");
    switch (key_code)
    {
        case 'a':
            g_game.player[player].left = keydown;
            send_key_change(key_code, keydown);
            break;
        case 'd':
            g_game.player[player].right = keydown;
            send_key_change(key_code, keydown);
            break;
        case 's':
            g_game.player[player].jump = keydown;
            send_key_change(key_code, keydown);
            break;
        default:
            UNREACHABLE();
            break;
    }
}