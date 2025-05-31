#include <common.h>
#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
int add(int a, int b)
{
  return a + b;
}





#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

int socket_close(sock_t sockfd)
{
    return closesocket(sockfd);
}
int socket_setblocking(sock_t sockfd, int noblock)
{
    u_long mode = noblock;
    return ioctlsocket(sockfd, FIONBIO, &mode);
}
int socket_poll(Pollfd fds[], unsigned int count, int timeout)
{
    return WSAPoll(fds, count, timeout);
}

#else
int socket_close(sock_t sockfd)
{
    return close(sockfd);
}
int socket_poll(Pollfd fds[], unsigned int count, int timeout)
{
    return poll(fds, count, timeout);
}
#endif
/*
** packi16() -- store a 16-bit int into a char buffer (like htons())
*/ 
static void packi16(unsigned char *buf, uint16_t i)
{
    *buf++ = i>>8; *buf++ = i;
}

/*
** packi32() -- store a 32-bit int into a char buffer (like htonl())
*/ 
static void packi32(unsigned char *buf, uint32_t i)
{
    *buf++ = i>>24; *buf++ = i>>16;
    *buf++ = i>>8;  *buf++ = i;
}

/*
** packi64() -- store a 64-bit int into a char buffer (like htonl())
*/ 
static void packi64(unsigned char *buf, uint64_t i)
{
    *buf++ = i>>56; *buf++ = i>>48;
    *buf++ = i>>40; *buf++ = i>>32;
    *buf++ = i>>24; *buf++ = i>>16;
    *buf++ = i>>8;  *buf++ = i;
}

/*
** unpacki16() -- unpack a 16-bit int from a char buffer (like ntohs())
*/ 
static int16_t unpacki16(unsigned char *buf)
{
    uint16_t i2 = ((uint16_t)buf[0]<<8) | buf[1];
    int i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffu) { i = i2; }
    else { i = -1 - (uint16_t)(0xffffu - i2); }

    return i;
}

/*
** unpacku16() -- unpack a 16-bit unsigned from a char buffer (like ntohs())
*/ 
static uint16_t unpacku16(unsigned char *buf)
{
    return ((uint16_t)buf[0]<<8) | buf[1];
}

/*
** unpacki32() -- unpack a 32-bit int from a char buffer (like ntohl())
*/ 
static int32_t unpacki32(unsigned char *buf)
{
    uint32_t i2 = ((uint32_t)buf[0]<<24) |
                           ((uint32_t)buf[1]<<16) |
                           ((uint32_t)buf[2]<<8)  |
                           buf[3];
    uint32_t i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffffffu) { i = i2; }
    else { i = -1 - (int32_t)(0xffffffffu - i2); }

    return i;
}

/*
** unpacku32() -- unpack a 32-bit unsigned from a char buffer (like ntohl())
*/ 
static uint32_t unpacku32(unsigned char *buf)
{
    return ((uint32_t)buf[0]<<24) |
           ((uint32_t)buf[1]<<16) |
           ((uint32_t)buf[2]<<8)  |
           buf[3];
}

/*
** unpacki64() -- unpack a 64-bit int from a char buffer (like ntohl())
*/ 
static uint64_t unpacki64(unsigned char *buf)
{
    uint64_t i2 = ((uint64_t)buf[0]<<56) |
                                ((uint64_t)buf[1]<<48) |
                                ((uint64_t)buf[2]<<40) |
                                ((uint64_t)buf[3]<<32) |
                                ((uint64_t)buf[4]<<24) |
                                ((uint64_t)buf[5]<<16) |
                                ((uint64_t)buf[6]<<8)  |
                                buf[7];
    long long int i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffffffffffffffu) { i = i2; }
    else { i = -1 -(int64_t)(0xffffffffffffffffu - i2); }

    return i;
}

/*
** unpacku64() -- unpack a 64-bit unsigned from a char buffer (like ntohl())
*/ 
static uint64_t unpacku64(unsigned char *buf)
{
    return ((uint64_t)buf[0]<<56) |
           ((uint64_t)buf[1]<<48) |
           ((uint64_t)buf[2]<<40) |
           ((uint64_t)buf[3]<<32) |
           ((uint64_t)buf[4]<<24) |
           ((uint64_t)buf[5]<<16) |
           ((uint64_t)buf[6]<<8)  |
           buf[7];
}

/*
** pack() -- store data dictated by the format string in the buffer
*/ 

extern unsigned int pack(unsigned char *buf, char *format, ...)
{
    va_list ap;

    int8_t c;              // 8-bit
    uint8_t C;

    int16_t h;                      // 16-bit
    uint16_t H;

    int32_t l;                 // 32-bit
    uint32_t L;

    int64_t q;            // 64-bit
    uint64_t Q;

    char *s;                    // strings
    unsigned int len;

    unsigned int size = 0;

    va_start(ap, format);

    for(; *format != '\0'; format++) {
        switch(*format) {
        case 'c': // 8-bit
            size += 1;
            c = (int8_t)va_arg(ap, int); // promoted
            *buf++ = c;
            break;

        case 'C': // 8-bit unsigned
            size += 1;
            C = (uint8_t)va_arg(ap, unsigned int); // promoted
            *buf++ = C;
            break;

        case 'h': // 16-bit
            size += 2;
            h = va_arg(ap, int);
            packi16(buf, h);
            buf += 2;
            break;

        case 'H': // 16-bit unsigned
            size += 2;
            H = va_arg(ap, unsigned int);
            packi16(buf, H);
            buf += 2;
            break;

        case 'l': // 32-bit
            size += 4;
            l = va_arg(ap, int32_t);
            packi32(buf, l);
            buf += 4;
            break;

        case 'L': // 32-bit unsigned
            size += 4;
            L = va_arg(ap, uint32_t);
            packi32(buf, L);
            buf += 4;
            break;

        case 'q': // 64-bit
            size += 8;
            q = va_arg(ap, int64_t);
            packi64(buf, q);
            buf += 8;
            break;

        case 'Q': // 64-bit unsigned
            size += 8;
            Q = va_arg(ap, uint64_t);
            packi64(buf, Q);
            buf += 8;
            break;

        case 's': // string
            s = va_arg(ap, char*);
            len = strlen(s);
            size += len + 2;
            packi16(buf, len);
            buf += 2;
            memcpy(buf, s, len);
            buf += len;
            break;
        }
    }

    va_end(ap);

    return size;
}

/*
** unpack() -- unpack data dictated by the format string into the buffer
*/
extern void unpack(unsigned char *buf, char *format, ...)
{
    va_list ap;

    int8_t *c;              // 8-bit
    uint16_t *C;

    int16_t *h;                      // 16-bit
    uint16_t *H;

    int32_t *l;                 // 32-bit
    uint32_t *L;

    int64_t *q;            // 64-bit
    uint64_t *Q;

    char *s;
    unsigned int len, maxstrlen=0, count;

    va_start(ap, format);

    for(; *format != '\0'; format++) {
        switch(*format) {
        case 'c': // 8-bit
            c = va_arg(ap, signed char*);
            if (*buf <= 0x7f) { *c = *buf;} // re-sign
            else { *c = -1 - (unsigned char)(0xffu - *buf); }
            buf++;
            break;

        case 'C': // 8-bit unsigned
            C = va_arg(ap, unsigned char*);
            *C = *buf++;
            break;

        case 'h': // 16-bit
            h = va_arg(ap, int16_t*);
            *h = unpacki16(buf);
            buf += 2;
            break;

        case 'H': // 16-bit unsigned
            H = va_arg(ap, uint16_t*);
            *H = unpacku16(buf);
            buf += 2;
            break;

        case 'l': // 32-bit
            l = va_arg(ap, int32_t*);
            *l = unpacki32(buf);
            buf += 4;
            break;

        case 'L': // 32-bit unsigned
            L = va_arg(ap, uint32_t*);
            *L = unpacku32(buf);
            buf += 4;
            break;

        case 'q': // 64-bit
            q = va_arg(ap, int64_t*);
            *q = unpacki64(buf);
            buf += 8;
            break;

        case 'Q': // 64-bit unsigned
            Q = va_arg(ap, uint64_t*);
            *Q = unpacku64(buf);
            buf += 8;
            break;

        case 's': // string
            s = va_arg(ap, char*);
            len = unpacku16(buf);
            buf += 2;
            if (maxstrlen > 0 && len >= maxstrlen) count = maxstrlen - 1;
            else count = len;
            memcpy(s, buf, count);
            s[count] = '\0';
            buf += len;
            break;

        default:
            if (isdigit(*format)) { // track max str len
                maxstrlen = maxstrlen * 10 + (*format-'0');
            }
        }

        if (!isdigit(*format)) maxstrlen = 0;
    }

    va_end(ap);
}

void debug_buffer_print(unsigned char* buffer, int len)
{
    int i = 0;
    printf("[");
    for (;i < len;++i)
    {
        printf("%x, ", buffer[i]);
    }
    printf("]: len: %d\n", i);
}
