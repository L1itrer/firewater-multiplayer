#ifndef COMMON_H
#define COMMON_H
#include <assert.h>
int add(int a, int b);

#ifdef WIN32
typedef unsigned long long sock_t;


#else

typedef int sock_t;

#endif //WIN32

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif //INVALID_SOCKET

typedef struct Pollfd{
    sock_t fd;
    short events;
    short revents;
}Pollfd;

int socket_close(sock_t sockfd);
int socket_setblocking(sock_t sockfd, int noblock);
int socket_poll(Pollfd fds[], unsigned int count, int timeout);

typedef enum MessageKind{
    PLAYER_QUEUE,
	PLAYER_MOVING,
    GAME_START,
    SERVER_FULL
}MessageKind;


#define SERVER_PORT "2137"
#ifdef __cplusplus
extern "C" {
#endif
unsigned int pack(unsigned char *buf, char *format, ...);
void unpack(unsigned char *buf, char *format, ...);
void debug_buffer_print(unsigned char* buffer, int max_len);
#ifdef __cplusplus
}
#endif
#define TODO(msg) assert(0 && msg)
#define UNREACHABLE() assert(0 && "Unreachable!\n")
#endif COMMON_H
