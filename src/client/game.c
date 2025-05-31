#include <game.h>
#include <common.h>
#include <platform.h>
#include <stdint.h>
// TODO: REMOVE ANY C LIB FUNCTIONS FROM THIS FILE
#include <stdio.h> // for testing purposes

Game g_game = {0};
int local_player = 0; // TODO: it does not have to be always 0!

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
        if (player->actions[ACTION_LEFT])
        {
            player->position.x -= MOVEMENT_SPEED;
        }
        if (player->actions[ACTION_RIGHT])
        {
            player->position.x += MOVEMENT_SPEED;
        }
        if (player->actions[ACTION_JUMP])
        {
            player->position.y += 5.0f;
            player->actions[ACTION_JUMP] = false;
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

void game_draw_server_full()
{
     platform_draw_text("Server full", 100, 200, 56, 
                (ColorHSV){.hue=0.0f, .saturation=0.0f, .value=0.9f});
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
        case GS_SERVER_FULL:
            game_draw_server_full();
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
        int32_t packet_size = 0;
        MessageKind message;
        server_recv(buffer, 128);
        unpack(buffer, "ll", &packet_size, &message);
        printf("Received %d bytes\n", packet_size);
        debug_buffer_print(buffer, packet_size);
        switch (message)
        {
            case GAME_START:
                game_start();
                break;
            case PLAYER_MOVING:
                Direction direction;
                bool keydown;
                unpack(buffer+8, "lc", &direction, &keydown);
                key_change(direction, PLAYER_NETWORK, keydown);
                break;
            case SERVER_FULL:
                g_game.state = GS_SERVER_FULL;
                break;
            default:
                UNREACHABLE();
                break;
        }
    }
}


void send_key_change(int key_code, bool keydown)
{
    MessageKind msg = PLAYER_MOVING;
    Direction dir = key_code;
    char buffer[128] = {0};
    int32_t packet_size = pack(buffer, "lllc", 0, msg, dir, keydown);
    pack(buffer, "l", packet_size);
    printf("Sending %d bytes\n", packet_size);
    debug_buffer_print(buffer, packet_size);
    server_send(buffer, packet_size);
}

void local_key_change(int key_code, bool keydown)
{
    if (g_game.player[local_player].actions[key_code] != keydown)
    {
        send_key_change(key_code, keydown);
    }
    key_change(key_code, local_player, keydown);
}

void key_change(int key_code, int player, bool keydown)
{
    assert(player == PLAYER_LOCAL || player == PLAYER_NETWORK && "Unexpected player!\n");
    assert(key_code < 4 && key_code >= 0 && "Invalid key code!\n");
    g_game.player[local_player].actions[key_code] = keydown;
}