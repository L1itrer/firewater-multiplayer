#ifndef GAME_H
#define GAME_H
#include <common.h>
#include <stdbool.h>
void game_update();
void game_draw();
void game_poll();
int game_init(const char* ip, const char* port);
void key_change(int key_code, int player, bool keydown);


typedef struct Vec2{
    float x, y;
}Vec2;

typedef struct Rect{
    float x, y, width, height;
}Rect;

typedef struct ColorHSV{
    float hue, saturation, value;
}ColorHSV;

typedef enum GameState{
    GS_AWAITING_CONNECTION,
    GS_GAME_PLAYING
}GameState;

#define MOVEMENT_SPEED 10.0

typedef struct Player{
    Vec2 position;
    bool right, left, jump;
}Player;

#define PLAYER_LOCAL 0
#define PLAYER_NETWORK 1

typedef enum Direction{
    LEFT = 'a',
    RIGHT = 'd',
    JUMP = 's'
}Direction;

typedef struct Game{
    Player player[2];
    GameState state;
}Game;

#endif //GAME_H