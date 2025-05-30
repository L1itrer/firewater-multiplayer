#include <raylib.h>
#include <common.h>
#include <game.h>
#include <stdint.h>
#include <platform.h>


void draw_rectangle(Rect r, ColorHSV c)
{
    Rectangle rec = {
        .x = r.x, .y = r.y, .width = r.width, .height = r.height
    };
    Color color = ColorFromHSV(c.hue, c.saturation, c.value);
    DrawRectangleRec(rec, color);
}

void poll_keyboard()
{
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  key_change('a', PLAYER_LOCAL, true);
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) key_change('d', PLAYER_LOCAL, true);
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_SPACE)) key_change('s', PLAYER_LOCAL, true);
    if (IsKeyUp(KEY_A)   || IsKeyUp(KEY_LEFT))    key_change('a', PLAYER_LOCAL, false);
    if (IsKeyUp(KEY_D)   || IsKeyUp(KEY_RIGHT))   key_change('d', PLAYER_LOCAL, false);
    if (IsKeyUp(KEY_S)   || IsKeyUp(KEY_SPACE))   key_change('s', PLAYER_LOCAL, false);
}


int main(void)
{
    InitWindow(800, 600, "Hello from client!");
    if (!game_init("127.0.0.1", SERVER_PORT))
    {
        printf("Error when connecting\n");
        return 1;
    }

    char buffer[128] = {0};
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GetColor(0x181818));
        DrawText("well", 380, 300, 24, GOLD);
        game_poll();
        game_update();
        game_draw();
        EndDrawing();
    }



    CloseWindow();
    return 0;
}
