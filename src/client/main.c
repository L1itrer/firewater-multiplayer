#include <raylib.h>
#include <common.h>
#include "net.h"

int main()
{
    //InitWindow(800, 600, "Hello from client!");
    //
    //while (!WindowShouldClose())
    //{
    //    BeginDrawing();
    //    ClearBackground(GetColor(0x181818));
    //    DrawText("well", 380, 300, 24, GOLD);
    //    EndDrawing();
    //}
    sock_t serverfd = server_connect("127.0.0.1", SERVER_PORT);
    char buffer[128] = { 0 };
    if (server_recv(serverfd, buffer, 128) == -1)
    {
        printf("error reaceving data\n");
        exit(1);
    }
    printf("%s\n", buffer);


    //CloseWindow();
    return 0;
}
