#include <raylib.h>
//#include <common.h>

int main()
{
    InitWindow(800, 600, "Hello from client!");
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GetColor(0x181818));
        DrawText("well", 380, 300, 24, GOLD);
        EndDrawing();
    }


    CloseWindow();
    return 0;
}
