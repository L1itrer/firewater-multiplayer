#include <raylib.h>
#include <common.h>
#include <game.h>
#include <platform.h>
#include "net.h"


void draw_rectangle(Rect r, ColorHSV c)
{
    Rectangle rec = {
        .x = r.x, .y = r.y, .width = r.width, .height = r.height
    };
    Color color = ColorFromHSV(c.hue, c.saturation, c.value);
    DrawRectangleRec(rec, color);
}

int main(void)
{
    InitWindow(800, 600, "Hello from client!");
    
    game_init();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GetColor(0x181818));
        DrawText("well", 380, 300, 24, GOLD);
        game_update();
        game_draw();
        EndDrawing();
    }



    CloseWindow();
    return 0;
}
