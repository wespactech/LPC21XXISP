#ifndef __STTY_H__
#define __STTY_H__

#include <pthread.h>
#include <termios.h>

typedef struct tty_info_t {
    int fd;
    pthread_mutex_t mt;
    char name[48];
    struct termios ntm;
    struct termios otm;
} TTY_INFO;

TTY_INFO *readyTTY(int id);
int setTTYSpeed(TTY_INFO *ptty, int speed);
int setTTYParity(TTY_INFO *ptty, int databits, int parity, int stopbits);
int cleanTTY(TTY_INFO *ptty);
int sendnTTY(TTY_INFO *ptty, char *pbuf, int size);
int recvnTTY(TTY_INFO *ptty, char *pbuf, int size);
int lockTTY(TTY_INFO *ptty);
int unlockTTY(TTY_INFO *ptty);

void clearDeviceBuffer(TTY_INFO *ptty);

#endif
