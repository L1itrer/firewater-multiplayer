#ifndef PLATFORM_H
#define PLATFORM_H
#include <game.h>

void draw_rectangle(Rect r, ColorHSV c);


#include <common.h>

/**  @brief Connect to a server given an ip
 *   @return A valid sock_t on success, an INVALID_SOCKET on failure
*/
sock_t server_connect(const char* ip, const char* port);

int server_send(const char* buffer, size_t len);
int server_recv(const char* buffer, size_t max_size);
int server_is_data_available();

#endif // PLATFORM_H