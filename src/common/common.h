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
    PLAYER_START,
	PLAYER_MOVING,
    JUMP,
    SERVER_FULL
}MessageKind;


#define SERVER_PORT "2137"

unsigned int pack(unsigned char *buf, char *format, ...);
void unpack(unsigned char *buf, char *format, ...);

#define TODO(msg) assert(0 && msg)
#endif COMMON_H
