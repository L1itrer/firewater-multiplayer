#ifndef GAME_H
#define GAME_H

void game_update();
void game_draw();
void game_init();

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