#ifndef COMMON_H
#define COMMON_H
#include <assert.h>
int add(int a, int b);

#ifdef WIN32
typedef unsigned long long sock_t;

#else
typedef int sock_t;


#endif //WIN32

int socket_close(sock_t sockfd);


typedef enum MessageKind{
	PLAYER_JOINED,
}MessageKind;


typedef struct Position{
	float x, y;
}Position;

typedef struct Game
{
	Position player_position, other_player_position;
}Game;

#define SERVER_PORT "2137"


#define TODO(msg) assert(0 && msg)
#endif COMMON_H
