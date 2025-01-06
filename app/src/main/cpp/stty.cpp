#include "config.h"
#include <sys/file.h>

TTY_INFO *readyTTY(int id) {
    TTY_INFO *ptty;
    ptty = (TTY_INFO *) malloc(sizeof(TTY_INFO));
    if (NULL == ptty) {
        return NULL;
    }
    memset(ptty, 0, sizeof(TTY_INFO));
    pthread_mutex_init(&ptty->mt, NULL);
    snprintf(ptty->name, sizeof(ptty->name), "/dev/ttyS%d", id);

    DEBUG_OUT("\n\rptty->name:%s", ptty->name);
    ptty->fd = open(ptty->name, O_RDWR | O_NOCTTY);
    if (ptty->fd < 0) {
        perror("Failed to open serial port");
        free(ptty);
        return NULL;
    }
    if (tcgetattr(ptty->fd, &ptty->otm) != 0) {
        perror("Failed to get serial port attributes");
        close(ptty->fd);
        free(ptty);
        return NULL;
    }

    return ptty;
}

int cleanTTY(TTY_INFO *ptty) {
    if (ptty->fd > 0) {
        tcsetattr(ptty->fd, TCSANOW, &ptty->otm);
        close(ptty->fd);
        ptty->fd = -1;
        free(ptty);
        ptty = NULL;
    }
    return 0;
}

int setTTYSpeed(TTY_INFO *ptty, int speed) {
    int i;

    if (tcgetattr(ptty->fd, &ptty->ntm) != 0) {
        perror("tcgetattr error");
        return -1;
    }
    ptty->ntm.c_cflag &= ~CBAUD;
    ptty->ntm.c_cflag |= (CLOCAL | CREAD | CS8);
    switch (speed) {
        case 300:
            ptty->ntm.c_cflag |= B300;
            break;
        case 1200:
            ptty->ntm.c_cflag |= B1200;
            break;
        case 2400:
            ptty->ntm.c_cflag |= B2400;
            break;
        case 4800:
            ptty->ntm.c_cflag |= B4800;
            break;
        case 9600:
            ptty->ntm.c_cflag |= B9600;
            break;
        case 19200:
            ptty->ntm.c_cflag |= B19200;
            break;
        case 38400:
            ptty->ntm.c_cflag |= B38400;
            break;
        case 115200:
            ptty->ntm.c_cflag |= B115200;
            break;
    }
    ptty->ntm.c_iflag = IGNPAR;
    ptty->ntm.c_oflag = 0;
    tcflush(ptty->fd, TCIFLUSH);
    if (tcsetattr(ptty->fd, TCSANOW, &ptty->ntm) != 0) {
        perror("tcsetattr error");
        return -3;
    }
    return 0;
}


int setTTYParity(TTY_INFO *ptty, int databits, int parity, int stopbits) {
    DEBUG_OUT("setTTYParity enter\n");

    if (tcgetattr(ptty->fd, &ptty->ntm) != 0) {
        printf("SetupSerial [%s]\n", ptty->name);
        return 1;
    }
    bzero(&ptty->ntm, sizeof(ptty->ntm));
    ptty->ntm.c_cflag = CS8 | CLOCAL | CREAD;
    ptty->ntm.c_iflag = IGNPAR;
    ptty->ntm.c_oflag = 0;
    ptty->ntm.c_cflag &= ~CSIZE;

    switch (databits) {
        case 7:
            ptty->ntm.c_cflag |= CS7;
            break;
        case 8:
            ptty->ntm.c_cflag |= CS8;
            break;
        default:
            printf("Unsupported data size\n");
            return 5;
    }

    switch (parity) {
        case 'n':
        case 'N':
            ptty->ntm.c_cflag &= ~PARENB;
            ptty->ntm.c_iflag &= ~INPCK;
            break;
        case 'o':
        case 'O':
            ptty->ntm.c_cflag |= (PARODD | PARENB);
            ptty->ntm.c_iflag |= INPCK;
            break;
        case 'e':
        case 'E':
            ptty->ntm.c_cflag |= PARENB;
            ptty->ntm.c_cflag &= ~PARODD;
            ptty->ntm.c_iflag |= INPCK;
            break;
        case 'S':
        case 's':
            ptty->ntm.c_cflag &= ~PARENB;
            ptty->ntm.c_cflag &= ~CSTOPB;
            break;
        default:
            printf("Unsupported parity\n");
            return 2;
    }

    ptty->ntm.c_cflag &= ~CSTOPB;
    ptty->ntm.c_cc[VTIME] = 0;
    ptty->ntm.c_cc[VMIN] = 1;
    tcflush(ptty->fd, TCIFLUSH);

    if (tcsetattr(ptty->fd, TCSANOW, &ptty->ntm) != 0) {
        perror("SetupSerial");
        return 4;
    }
    return 0;
}


int recvnTTY(TTY_INFO *ptty, char *pbuf, int size) {
    int ret, left, bytes;
    int timeout = 0;

    left = size;
    while (left > 0 && timeout < 10000) {
        ret = 0;
        bytes = 0;
        pthread_mutex_lock(&ptty->mt);
        if (ioctl(ptty->fd, FIONREAD, &bytes) < 0) {
            perror("ioctl error");
            pthread_mutex_unlock(&ptty->mt);
            return -1;
        }

        if (bytes > 0) {
            ret = read(ptty->fd, pbuf, left);   // 读取数据
            if (ret < 0) {
                perror("read error");
                pthread_mutex_unlock(&ptty->mt);
                return -1;
            }
            timeout = 0;
        } else {
            timeout++;
        }
        pthread_mutex_unlock(&ptty->mt);    // 解锁

        if (ret > 0) {
            left -= ret;
            pbuf += ret;
        }
        usleep(100);
    }
    if (timeout >= 10000) {
        syslog(LOG_INFO, "recvnTTY timeout=%d\n\r", timeout);
        return -2;
    }
    return size - left;
}

int sendnTTY(TTY_INFO *ptty, char *pbuf, int size) {
    int ret, nleft, timeout;
    char *ptmp;

    ret = 0;
    timeout = 0;
    nleft = size;
    ptmp = pbuf;
    while ((nleft > 0) && (timeout < 5000)) {
        pthread_mutex_lock(&ptty->mt);
        ret = write(ptty->fd, ptmp, nleft);
        pthread_mutex_unlock(&ptty->mt);

        if (ret > 0) {
            nleft -= ret;
            ptmp += ret;
            timeout = 0;
        } else if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                timeout++;
            } else {
                perror("Write error");
                return -1;
            }
        }
        usleep(1000);
    }
    if (timeout >= 5000) {
        syslog(LOG_INFO, "sendnTTY timeout=%d\n\r", timeout);
        return -2;
    }
    return size - nleft;
}


int lockTTY(TTY_INFO *ptty) {
    if (ptty->fd < 0) {
        return 1;
    }
    return flock(ptty->fd, LOCK_EX);
}

int unlockTTY(TTY_INFO *ptty) {
    if (ptty->fd < 0) {
        return 1;
    }
    return flock(ptty->fd, LOCK_UN);
}

void clearDeviceBuffer(TTY_INFO *ptty) {
    if (tcflush(ptty->fd, TCIOFLUSH) < 0) {
        perror("Failed to flush the serial port");
    }
}
