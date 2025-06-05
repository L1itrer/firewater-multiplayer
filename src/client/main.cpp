#include <raylib.h>
extern "C" {
    #include <common.h>
}
#include <game.h>
#include <stdint.h>
#include <stdio.h>
#include <platform.h>

#define COLOR_FROM_HSV(c) ColorFromHSV(c.hue, c.saturation, c.value)

void platform_draw_rectangle(Rect r, ColorHSV c)
{
    Rectangle rec = {
        r.x, r.y, r.width, r.height
    };
    Color color = COLOR_FROM_HSV(c);
    DrawRectangleRec(rec, color);
}

void platform_poll_keyboard()
{
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  local_key_change(ACTION_LEFT, true);
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) local_key_change(ACTION_RIGHT, true);
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_SPACE)) local_key_change(ACTION_JUMP, true);
    if (IsKeyUp(KEY_A)   && IsKeyUp(KEY_LEFT))    local_key_change(ACTION_LEFT, false);
    if (IsKeyUp(KEY_D)   && IsKeyUp(KEY_RIGHT))   local_key_change(ACTION_RIGHT, false);
    if (IsKeyUp(KEY_S)   && IsKeyUp(KEY_SPACE))   local_key_change(ACTION_JUMP, false);
}

void platform_draw_text(const char* text, int x, int y, int font_size, ColorHSV color)
{
    Color c = COLOR_FROM_HSV(color);
    DrawText(text, x, y, font_size, c);
}


int main(int argc, const char* const* argv)
{
    const char* ip = argc >= 2 ? argv[1] : "127.0.0.1";
    InitWindow(800, 600, "Hello from client!");
    SetTargetFPS(60);
    if (!game_init(ip, SERVER_PORT))
    {
        printf("Error when connecting\n");
        return 1;
    }

    char buffer[128] = {0};
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GetColor(0x181818));
        game_poll();
        game_update();
        game_draw();
        EndDrawing();
    }



    CloseWindow();
    return 0;
}
