#ifndef _CONFIG_H__
#define _CONFIG_H__

#define DEBUG_OUT //printf
#define DEBUG_TTYS 0
#define DEBUG_UPDATE 0

#define CHIPLPC2132 1
#define CHIPLPC2138 2
#define CHIPLPC2136 3

#define MAX_LINELEN 45
#define ENCODE_BYTE(b) (((b) == 0) ? 0x60 : ((b) + 0x20))

typedef enum bmc_update_err {
    BMC_UPDATE_SUCCESS = 0,
    OPEN_UPDATE_FILE_ERR,
    MOVE_POINTER_TO_FILE_END_ERR,
    GET_FLIE_LENGTH_ERR,
    MOVE_POINTER_TO_FILE_BEGIN_ERR,
    GET_MEMORY_BUFFER_ERR,
    READ_UPDATE_FLIE_ERR,
    OPEN_TTYS1_ERR,
    LOCK_TTYS1_FAIL,
    SET_TTYS1_BAUDRATE_FAIL,
    SET_TTYS1_PARITY_FAIL,
    UPDATE_FILE_OVER_FLASH_VOLUME,
    CMD_SEND_NO_RECIVE_ACK,
    CMD_SEND_AND_RECIVE_CHECK_ERR,
    SEND_DATA_TO_FLASH_ERR1,
    SEND_DATA_TO_FLASH_ERR2,
    SENT_UPDATE_CMD_ERR,
    RECEIVEE_CMD_ACK_TIMOUT,
    RECEIVEE_CMD_UNMATCH,
    SENT_ENCODE_DATA_ERR,
    RECEIVEE_ACK_DATA_TIMEOUT,
    RECEIVEE_ACK_DATA_UNMATCH,
} bmc_update_err_e;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include "stty.h"
#include <sys/ioctl.h>

#endif
