#ifndef GAME_H
#define GAME_H
#include <net.h>
#include <common.h>

void game_update();
void game_draw();
int game_init(const char* ip, const char* port);

typedef struct Vec2{
    float x, y;
}Vec2;

typedef struct Rect{
    float x, y, width, height;
}Rect;

typedef struct ColorHSV{
    float hue, saturation, value;
}ColorHSV;

typedef struct GameState{
    Vec2 player_pos, other_player_pos;
}GameState;

#endif //GAME_H