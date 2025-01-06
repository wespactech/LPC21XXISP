#include "config.h"
#include "stty.h"
#include <jni.h>
#include <string>
#include <android/log.h>
#include <thread>

#define LOG_TAG "MyISPNativeLib"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" JNIEXPORT jstring JNICALL Java_com_xysantai_myisp_MainActivity_getShowMessage(
        JNIEnv *env,
        jobject instance) {

    char *str = "NDK Test";

    return env->NewStringUTF(str);
}

extern "C" JNIEXPORT void JNICALL Java_com_xysantai_myisp_MainActivity_showAlertFromJNI(
        JNIEnv *env,
        jobject obj,
        jstring message) {

    const char *nativeMessage = env->GetStringUTFChars(message, 0);

    if (obj == NULL) {
        return;
    }

    jclass clazz = env->GetObjectClass(obj);
    if (clazz == NULL) {
        return;
    }
    jmethodID methodID = env->GetMethodID(clazz, "showAlert", "(Ljava/lang/String;)V");
    if (methodID == NULL) {
        return;
    }

    jstring jMessage = env->NewStringUTF(nativeMessage);
    env->CallVoidMethod(obj, methodID, jMessage);

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->ReleaseStringUTFChars(message, nativeMessage);
    env->DeleteLocalRef(jMessage);
}


extern "C" JNIEXPORT void JNICALL Java_com_xysantai_myisp_MainActivity_showResponseMsgFromJNI(
        JNIEnv *env,
        jobject obj,
        jstring message) {

    const char *nativeMessage = env->GetStringUTFChars(message, 0);

    jclass clazz = env->GetObjectClass(obj);

    jmethodID methodID = env->GetMethodID(clazz, "showReponseMessage", "(Ljava/lang/String;)V");
    if (methodID == nullptr) {
        return;
    }

    jstring jMessage = env->NewStringUTF(nativeMessage);
    env->CallVoidMethod(obj, methodID, jMessage);

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->ReleaseStringUTFChars(message, nativeMessage);
    env->DeleteLocalRef(jMessage);
    env->DeleteLocalRef(clazz);

}


extern "C" JNIEXPORT void JNICALL Java_com_xysantai_myisp_MainActivity_showChipTypeFromJNI(
        JNIEnv *env,
        jobject obj,
        jstring message) {

    const char *nativeMessage = env->GetStringUTFChars(message, 0);

    jclass clazz = env->GetObjectClass(obj);

    jmethodID methodID = env->GetMethodID(clazz, "showChipTypeMessage", "(Ljava/lang/String;)V");
    if (methodID == nullptr) {
        return ;
    }

    jstring jMessage = env->NewStringUTF(nativeMessage);
    env->CallVoidMethod(obj, methodID, jMessage);

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->ReleaseStringUTFChars(message, nativeMessage);
    env->DeleteLocalRef(jMessage);
    env->DeleteLocalRef(clazz);
}


char LpcProgramFlash(JNIEnv *env,jobject obj,TTY_INFO *ptty, unsigned char *Buf, unsigned int len);

extern "C" JNIEXPORT jint JNICALL Java_com_xysantai_myisp_MainActivity_programFlashFromJNI(
        JNIEnv *env,
        jobject obj,
        jstring filePath,
        jstring ttySidStr,
        jstring baudrate) {


    TTY_INFO *ptty;
    FILE *stream;
    unsigned int fileLength, tmp;
    unsigned char *bBuffer;
    char errorType = -1;
    char filepath[128];
    char messageBuffer[100];

    jstring message = env->NewStringUTF(">> Initialization ......\r\n");
    jclass clazz = env->GetObjectClass(obj);
    jmethodID methodID = env->GetMethodID(clazz, "showReponseMessage", "(Ljava/lang/String;)V");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);


    const char *nativeFilePath = env->GetStringUTFChars(filePath, 0);
    const char *nativeTtySid = env->GetStringUTFChars(ttySidStr, 0);
    int ttySid = atoi(nativeTtySid);
    const char *nativeBaudrate = env->GetStringUTFChars(baudrate, 0);
    int baudrateValue = atoi(nativeBaudrate);


    strncpy(filepath, nativeFilePath,sizeof(filepath) - 1);
    filepath[sizeof(filepath) - 1] = '\0';


    stream = fopen(filepath, "r");
    if (nullptr == stream) {
        env->ReleaseStringUTFChars(filePath, nativeFilePath);
        env->ReleaseStringUTFChars(ttySidStr, nativeTtySid);
        env->ReleaseStringUTFChars(baudrate, nativeBaudrate);
        env->DeleteLocalRef(clazz);
        return OPEN_UPDATE_FILE_ERR;
    }

    fseek(stream, 0, SEEK_END);
    fileLength = ftell(stream);
    fseek(stream, 0, SEEK_SET);
    bBuffer = (unsigned char *) malloc((fileLength + 5) * sizeof(char));
    fread(bBuffer, sizeof(char), fileLength, stream);
    fclose(stream);

    std::string messageStr = ">> File name：" + std::string(filepath);
    message = env->NewStringUTF(messageStr.c_str());
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);


    sprintf(messageBuffer, ">> File length：%u", fileLength);
    message = env->NewStringUTF(messageBuffer);
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);

    unsigned int fileLengthKB = fileLength / 1024;
    sprintf(messageBuffer, ">> File size：%u KB", fileLengthKB);
    message = env->NewStringUTF(messageBuffer);
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);

    message = env->NewStringUTF("Set serial port (baud rate, 8 data bits, no parity check, 1 stop bit)\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);


    ptty = readyTTY(ttySid);
    if (nullptr == ptty) {
        fprintf(stderr, "Failed to open the Serial Port\n");
        free(bBuffer);
        env->ReleaseStringUTFChars(filePath, nativeFilePath);
        env->ReleaseStringUTFChars(ttySidStr, nativeTtySid);
        env->ReleaseStringUTFChars(baudrate, nativeBaudrate);
        env->DeleteLocalRef(clazz);
        return OPEN_TTYS1_ERR;
    }


    if(0!=lockTTY(ptty)){
        free(bBuffer);
        cleanTTY(ptty);
        env->ReleaseStringUTFChars(filePath, nativeFilePath);
        env->ReleaseStringUTFChars(ttySidStr, nativeTtySid);
        env->ReleaseStringUTFChars(baudrate, nativeBaudrate);
        env->DeleteLocalRef(clazz);
        return LOCK_TTYS1_FAIL;
    }
    if(setTTYSpeed(ptty,baudrateValue)>0){
        free(bBuffer);
        cleanTTY(ptty);
        env->ReleaseStringUTFChars(filePath, nativeFilePath);
        env->ReleaseStringUTFChars(ttySidStr, nativeTtySid);
        env->ReleaseStringUTFChars(baudrate, nativeBaudrate);
        env->DeleteLocalRef(clazz);
        return SET_TTYS1_BAUDRATE_FAIL;
    }
    if(setTTYParity(ptty,8,'N',1)>0){
        free(bBuffer);
        cleanTTY(ptty);
        env->ReleaseStringUTFChars(filePath, nativeFilePath);
        env->ReleaseStringUTFChars(ttySidStr, nativeTtySid);
        env->ReleaseStringUTFChars(baudrate, nativeBaudrate);
        env->DeleteLocalRef(clazz);
        return SET_TTYS1_PARITY_FAIL;
    }

    message = env->NewStringUTF("Start Programing Flash ......\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);


    errorType = LpcProgramFlash(env,obj,ptty, bBuffer, fileLength);
    if (errorType != 0) {
        message = env->NewStringUTF("Programing Flash Failed.\r\n");
        env->CallVoidMethod(obj, methodID, message);
        env->DeleteLocalRef(message);
        usleep(500000);
        message = env->NewStringUTF("Programing Flash Failed.\r\n");
        Java_com_xysantai_myisp_MainActivity_showAlertFromJNI(env, obj, message);
        env->DeleteLocalRef(message);

    }else{
        message = env->NewStringUTF("Programing Flash Successed.\r\n");
        env->CallVoidMethod(obj, methodID, message);
        env->DeleteLocalRef(message);
        usleep(500000);
        message = env->NewStringUTF("Programing Flash Successed.\r\n");
        Java_com_xysantai_myisp_MainActivity_showAlertFromJNI(env, obj, message);
        env->DeleteLocalRef(message);
    }


    message = env->NewStringUTF("Clean up resources\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);


    free(bBuffer);
    cleanTTY(ptty);
    env->ReleaseStringUTFChars(filePath, nativeFilePath);
    env->ReleaseStringUTFChars(ttySidStr, nativeTtySid);
    env->ReleaseStringUTFChars(baudrate, nativeBaudrate);

    message = env->NewStringUTF("Clean up resources finished\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    env->DeleteLocalRef(clazz);
    usleep(500000);

    return 0;
}

char chipflag;

char CmdSendandRecvCheck(TTY_INFO *ptty, char *sentbuf, char *recvAckbuf) {
    unsigned int rCnt, sentLen, recLen, realRecvLen;
    char realRecvBuf[100];
    rCnt = 0;
    sentLen = 0;
    recLen = 0;
    realRecvLen = 0;

    clearDeviceBuffer(ptty);

    sentLen = strlen(sentbuf);
    if (sentLen != sendnTTY(ptty, sentbuf, sentLen)) {
        if ((0x3F == sentbuf[0]) && (1 == sentLen)) {
            syslog(LOG_INFO, "BMCIsp-->%s:SendnTTY(?) err\n\r", __func__);
            return SENT_UPDATE_CMD_ERR;
        }
        sentbuf[sentLen - 2] = '\0';
        syslog(LOG_INFO, "BMCIsp-->%s:SendnTTY(%s) err\n\r", __func__, sentbuf);
        return SENT_UPDATE_CMD_ERR;
    }

    usleep(100000);


    recLen = strlen(recvAckbuf);
    memset(realRecvBuf, 0, sizeof(realRecvBuf));
    realRecvLen = recvnTTY(ptty, realRecvBuf, recLen);


    char *start = realRecvBuf;
    while (*start == '\n') {
        start++;
    }
    if (start != realRecvBuf) {
        memmove(realRecvBuf, start, realRecvLen - (start - realRecvBuf));
        realRecvBuf[realRecvLen - (start - realRecvBuf)] = '\0';
    }

    if (realRecvLen < recLen) {
        return RECEIVEE_CMD_ACK_TIMOUT;
    }

    realRecvBuf[realRecvLen] = '\0';

    if (0 == strcmp(sentbuf, "J\r\n")) {
        if (0 == strcmp(realRecvBuf, "J\r\n0\r\n196369\r\n")) {
            chipflag = CHIPLPC2132;
            return 0;
        } else if (0 == strcmp(realRecvBuf, "J\r\n0\r\n196389\r\n")) {
            chipflag = CHIPLPC2138;
            return 0;
        } else if (0 == strcmp(realRecvBuf, "J\r\n0\r\n196387\r\n")) {
            chipflag = CHIPLPC2136;
            return 0;
        } else {
            return RECEIVEE_CMD_UNMATCH;
        }
    } else {
        if (0 != strcmp(realRecvBuf, recvAckbuf)) {
            syslog(LOG_INFO, "BMCIsp-->%s:RecvnTTY(%s)ACK err\n\r", __func__, sentbuf);
            return RECEIVEE_CMD_UNMATCH;
        }
        return 0;
    }
}

char SendDataToFlash(TTY_INFO *ptty, char *databuf, unsigned int len) {
    unsigned int i, rCnt, realRecvLen;
    char realRecvBuf[100];

    clearDeviceBuffer(ptty);

    if (len != sendnTTY(ptty, databuf, len)) {
        syslog(LOG_INFO, "BMCIsp-->%s:SendnTTY(ENCODE DATA) err\n\r", __func__);
        return SENT_ENCODE_DATA_ERR;
    }

    usleep(100000); // 100毫秒

    memset(realRecvBuf, 0, sizeof(realRecvBuf));
    realRecvLen = recvnTTY(ptty, realRecvBuf, len);
    if (realRecvLen < len) {
        return RECEIVEE_CMD_ACK_TIMOUT;
    }

    realRecvBuf[realRecvLen] = '\0';

    char *start = realRecvBuf;
    while (*start == '\n') {
        start++;
    }
    if (start != realRecvBuf) {
        int skipped = start - realRecvBuf;
        memmove(realRecvBuf, start, realRecvLen - skipped); // 移动数据
        realRecvLen -= skipped;
        realRecvBuf[realRecvLen] = '\0';
    }

    for (i = 0; i < len; i++) {
        if (realRecvBuf[i] != databuf[i]) {
            syslog(LOG_INFO, "BMCIsp-->%s:RecvnTTY(DATA)ACK er·r\n\r", __func__);
            return RECEIVEE_ACK_DATA_UNMATCH;
        }
    }
    return 0;
}

char LpcProgramFlash(JNIEnv *env,jobject obj,TTY_INFO *ptty,unsigned char *Buf,unsigned int len) {

    unsigned int xLen;
    unsigned char i, blockNum = 0;
    unsigned char dataCnt;
    unsigned char *sBuf;
    int linelen;
    int linecnt;
    int iSect = 0;
    int tmpSect = 0;
    int stringlen;
    float tmp = 0;
    char messageBuffer[256];
    char sentStr[100];
    char ackStr[100];
    char outbuf[512];
    unsigned long iPos = 0;
    unsigned long xCrc = 0;
    int oPos = 0;


    xLen = len;
    sBuf = Buf;
    dataCnt = 0;

    printf("Synchronizing with bmc/bcb.\r");
    fflush(stdout);

    // Send:?
    // Receive:Synchronized
    sentStr[0] = 0x3F;
    sentStr[1] = '\0';
    strcpy(ackStr, "Synchronized\r\n");
    if (CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
        syslog(LOG_INFO, "BMCIsp-->%s:Bmc/Bcb no ACK\n\r", __func__);
        printf("Synchronizing with bmc/bcb.........[Failed]\n\r");
        return CMD_SEND_NO_RECIVE_ACK;
    }
    jstring message = env->NewStringUTF("?");
    jclass clazz = env->GetObjectClass(obj);
    jmethodID methodID = env->GetMethodID(clazz, "showReponseMessage", "(Ljava/lang/String;)V");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);
    message = env->NewStringUTF("Synchronized");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);


    printf("Synchronizing with bmc/bcb..\r");
    fflush(stdout);
    // Send:Synchronized\r\n
    // Receive:Synchronized<CR><LF>OK<CR><LF>
    strcpy(sentStr, "Synchronized\r\n");
    strcpy(ackStr, "Synchronized\r\nOK\r\n");
    if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
        return CMD_SEND_AND_RECIVE_CHECK_ERR;
    }
    message = env->NewStringUTF("Synchronized");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);
    message = env->NewStringUTF("Synchronized\r\nOK");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);

    // usleep(800000);
    printf("Synchronizing with bmc/bcb...\r");
    fflush(stdout);
    // Send: oscillator 11MHz,(25000<CR><LF>)
    // Receive: (25000<CR><LF>OK<CR><LF>)
    strcpy(sentStr, "110592\r\n");           //oscillator 25000 to 110592
    strcpy(ackStr, "110592\r\nOK\r\n");
    if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
        return CMD_SEND_AND_RECIVE_CHECK_ERR;
    }
    message = env->NewStringUTF("110592");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);
    message = env->NewStringUTF("110592\r\nOK");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);



    // usleep(800000);
    printf("Synchronizing with bmc/bcb....\r");
    fflush(stdout);
    // Send:U 23130 unlock,(U 23130<CR><LF>)
    // Receive: (U 23130<CR><LF>0<CR><LF>)
    strcpy(sentStr, "U 23130\r\n");
    strcpy(ackStr, "U 23130\r\n0\r\n");
    if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
        return CMD_SEND_AND_RECIVE_CHECK_ERR;
    }
    message = env->NewStringUTF("U 23130");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);
    message = env->NewStringUTF("U 23130\r\n0");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);


    // usleep(800000);
    printf("Synchronizing with bmc/bcb.....\r");
    fflush(stdout);
    // send:J ,(J<CR><LF>)
    // receive:(J<CR><LF>0<CR><LF>X<CR><LF>)
    strcpy(sentStr, "J\r\n");
    strcpy(ackStr, "J\r\n0\r\n196369\r\n");     //LPC2132
    if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
        return CMD_SEND_AND_RECIVE_CHECK_ERR;
    }
    strcpy(ackStr, "J\r\n0\r\n196387\r\n");     //LPC2136
    if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
        return CMD_SEND_AND_RECIVE_CHECK_ERR;
    }
    strcpy(ackStr, "J\r\n0\r\n196389\r\n");     //LPC2138
    if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
        return CMD_SEND_AND_RECIVE_CHECK_ERR;
    }


    /*lpc2132 flash ram is 64k,lpc2136 flash ram is 256k,lpc2138 flash ram is 512k*/
    /*get file size*/
    if (CHIPLPC2132 == chipflag) {
        if ((xLen / 1024) > 64) {
            syslog(LOG_INFO, "BMCIsp-->%s:Over flash volume\n\r", __func__);
            DEBUG_OUT("\n\rOver flash volume");
            return UPDATE_FILE_OVER_FLASH_VOLUME;
        }
        message = env->NewStringUTF("Chip type：LPC2132");
        env->CallVoidMethod(obj, methodID, message);
        env->DeleteLocalRef(message);
        usleep(500000);
        message = env->NewStringUTF("LPC2132");
        jmethodID methodID1 = env->GetMethodID(clazz, "showChipTypeMessage", "(Ljava/lang/String;)V");
        env->CallVoidMethod(obj, methodID1, message);
        env->DeleteLocalRef(message);
        usleep(500000);
    } else if (CHIPLPC2136 == chipflag) {
        if ((xLen / 1024) > 256) {
            syslog(LOG_INFO, "BMCIsp-->%s:Over flash volume\n\r", __func__);
            DEBUG_OUT("\n\rOver flash volume");
            return UPDATE_FILE_OVER_FLASH_VOLUME;
        }
        message = env->NewStringUTF("Chip type：LPC2136");
        env->CallVoidMethod(obj, methodID, message);
        env->DeleteLocalRef(message);
        usleep(500000);
        message = env->NewStringUTF("LPC2136");
        jmethodID methodID1 = env->GetMethodID(clazz, "showChipTypeMessage", "(Ljava/lang/String;)V");
        env->CallVoidMethod(obj, methodID1, message);
        env->DeleteLocalRef(message);
        usleep(500000);
    } else if (CHIPLPC2138 == chipflag) {
        if ((xLen / 1024) > 512) {
            syslog(LOG_INFO, "BMCIsp-->%s:Over flash volume\n\r", __func__);
            DEBUG_OUT("\n\rOver flash volume");
            return UPDATE_FILE_OVER_FLASH_VOLUME;
        }
        message = env->NewStringUTF("Chip type：LPC2138");
        env->CallVoidMethod(obj, methodID, message);
        env->DeleteLocalRef(message);
        usleep(500000);
        message = env->NewStringUTF("LPC2138");
        jmethodID methodID1 = env->GetMethodID(clazz, "showChipTypeMessage", "(Ljava/lang/String;)V");
        env->CallVoidMethod(obj, methodID1, message);
        env->DeleteLocalRef(message);
        usleep(500000);
    }


    if (CHIPLPC2132 == chipflag) {
        blockNum = (xLen) / 4096;
        printf("Update need program [%d] blocks\n\r", blockNum + 1);
        if (blockNum > 7) {
            blockNum = 8;
        }
    } else if (CHIPLPC2136 == chipflag) {
        blockNum = (xLen) / 4096;
        printf("Update need program [%d] blocks\n\r", blockNum + 1);
        if ((blockNum > 7) && (blockNum < 62.5))            //4096*62.5=256000
        {
            blockNum = blockNum / 8 + 7;
        }
    } else if (CHIPLPC2138 == chipflag) {
        blockNum = (xLen) / 4096;
        printf("Update need program [%d] blocks\n\r", blockNum + 1);
        if ((blockNum > 7) && (blockNum < 125)) {
            if (blockNum < 120) {
                blockNum = blockNum / 8 + 7;
            } else {
                blockNum = blockNum - 120 + 22;
            }
        }

    }
    sprintf(messageBuffer, "Update need program [%d] blocks :", blockNum);
    message = env->NewStringUTF(messageBuffer);
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);



    // usleep(800000);
    printf("Synchronizing with bmc/bcb......\r");
    fflush(stdout);
    // Send： Wait to Write FLASH P 0 X<CR><LF>
    // Receive：P 0 X<CR><LF>0<CR><LF>
    // Wait to Write FLASH  P 0 X

    message = env->NewStringUTF("Prepare sector(s) for write operation\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);

    // usleep(800000);
    printf("Synchronizing with bmc/bcb.......\r");
    fflush(stdout);
    sprintf(sentStr, "P 0 %d\r\n", blockNum);
    strcpy(ackStr, sentStr);
    strcat(ackStr, "0\r\n");
    if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
        return CMD_SEND_AND_RECIVE_CHECK_ERR;
    }
    message = env->NewStringUTF(sentStr);
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);
    message = env->NewStringUTF(ackStr);
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);



    // usleep(800000);
    printf("Synchronizing with bmc/bcb........\r");
    fflush(stdout);

    message = env->NewStringUTF("Erase sector(s) operation\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);

    // Send： Erase FLASH,(E 0 X<CR><LF>
    // Receive：E 0 X<CR><LF>0<CR><LF>
    // Erase FLASH Sector E  0  X
    sprintf(sentStr, "E 0 %d\r\n", blockNum);
    strcpy(ackStr, sentStr);
    strcat(ackStr, "0\r\n");
    if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
        return CMD_SEND_AND_RECIVE_CHECK_ERR;
    }
    message = env->NewStringUTF(sentStr);
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);
    message = env->NewStringUTF(ackStr);
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);



    message = env->NewStringUTF("Write data to RAM\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);

    // usleep(800000);
    printf("Synchronizing with bmc/bcb........[Ok]\n\r");
    // Send： W 1073742336 4096<CR><LF>
    // Receive： W 40000200H 4096<CR><LF>0<CR><LF>
    // Write RAM W 40000200H 4096
    strcpy(sentStr, "W 1073742336 4096\r\n");
    strcpy(ackStr, "W 1073742336 4096\r\n0\r\n");
    if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
        return CMD_SEND_AND_RECIVE_CHECK_ERR;
    }
    message = env->NewStringUTF("W 1073742336 4096\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);
    message = env->NewStringUTF("W 1073742336 4096\r\n0\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);


    message = env->NewStringUTF("Starting ......");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);


/*
    jmethodID setMaxMethod = env->GetMethodID(clazz, "setProgressMax", "(I)V");
    env->CallVoidMethod(obj, setMaxMethod, xLen);
    jmethodID updateMethod = env->GetMethodID(clazz, "showProgressBar", "(I)V");
*/

    iPos = 0;
    while (iPos < xLen) {
        if (xLen - iPos > MAX_LINELEN) {
            linelen = MAX_LINELEN;
        } else {
            linelen = xLen - iPos;
        }

/*      env->CallVoidMethod(obj, updateMethod, iPos);
        usleep(500000);*/

        if ((iPos % 4096) == 4095) {
            outbuf[oPos++] = ENCODE_BYTE(1);
            outbuf[oPos++] = ENCODE_BYTE((sBuf[iPos] & 0xFC) >> 2);
            outbuf[oPos++] = ENCODE_BYTE((sBuf[iPos] & 0x03) << 4);
            outbuf[oPos++] = ENCODE_BYTE(0);
            outbuf[oPos++] = ENCODE_BYTE(0);

            xCrc += sBuf[iPos];
            iPos++;
            outbuf[oPos++] = '\r';
            outbuf[oPos++] = '\n';
            outbuf[oPos++] = '\0';

            stringlen = strlen(outbuf);
            if (0 != SendDataToFlash(ptty, outbuf, stringlen)) {
                return SEND_DATA_TO_FLASH_ERR2;
            }
            message = env->NewStringUTF(outbuf);
            env->CallVoidMethod(obj, methodID, message);
            env->DeleteLocalRef(message);
            usleep(500000);

            //	CRC + OK
            sprintf(sentStr, "%d\r\n", xCrc);
            sprintf(ackStr, "%d\r\nOK\r\n", xCrc);
            if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
                return CMD_SEND_AND_RECIVE_CHECK_ERR;
            }
            message = env->NewStringUTF(sentStr);
            env->CallVoidMethod(obj, methodID, message);
            env->DeleteLocalRef(message);
            usleep(500000);
            message = env->NewStringUTF(ackStr);
            env->CallVoidMethod(obj, methodID, message);
            env->DeleteLocalRef(message);
            usleep(500000);



            message = env->NewStringUTF("Prepare Sector(s) for Write Operation\r\n");
            env->CallVoidMethod(obj, methodID, message);
            env->DeleteLocalRef(message);
            usleep(500000);

            //	P X X
            //  sprintf(outbuf, "P %d %d\r\n", iSect / 2, iSect / 2);
            xCrc = 0;
            oPos = 0;
            if (CHIPLPC2132 == chipflag) {
                if (iSect >= 8) {
                    // >32k
                    sprintf(sentStr, "P %d %d\r\n", 8, 8);
                } else {
                    // <32k
                    sprintf(sentStr, "P %d %d\r\n", iSect, iSect);
                }
            } else if (CHIPLPC2136 == chipflag) {
                tmpSect = iSect;
                if ((tmpSect > 7) && (tmpSect < 62.5)) {
                    tmpSect = tmpSect / 8 + 7;
                }
                sprintf(sentStr, "P %d %d\r\n", tmpSect, tmpSect);
            } else if (CHIPLPC2138 == chipflag) {
                tmpSect = iSect;
                if ((tmpSect > 7) && (tmpSect < 125)) {
                    if (tmpSect < 120) {
                        tmpSect = tmpSect / 8 + 7;
                    } else {
                        tmpSect = tmpSect - 120 + 22;
                    }
                }
                sprintf(sentStr, "P %d %d\r\n", tmpSect, tmpSect);
            }

            strcpy(ackStr, sentStr);
            strcat(ackStr, "0\r\n");
            if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
                return CMD_SEND_AND_RECIVE_CHECK_ERR;
            }
            message = env->NewStringUTF(sentStr);
            env->CallVoidMethod(obj, methodID, message);
            env->DeleteLocalRef(message);
            usleep(500000);
            message = env->NewStringUTF(ackStr);
            env->CallVoidMethod(obj, methodID, message);
            env->DeleteLocalRef(message);
            usleep(500000);



            message = env->NewStringUTF("Copy data from RAM to Flash(4K+CRC)\r\n");
            env->CallVoidMethod(obj, methodID, message);
            env->DeleteLocalRef(message);
            usleep(500000);

            // C Y 1073742336 4096
            sprintf(sentStr, "C %d 1073742336 4096\r\n", iSect * 4096);
            strcpy(ackStr, sentStr);
            strcat(ackStr, "0\r\n");
            if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
                return CMD_SEND_AND_RECIVE_CHECK_ERR;
            }
            message = env->NewStringUTF(sentStr);
            env->CallVoidMethod(obj, methodID, message);
            env->DeleteLocalRef(message);
            usleep(500000);
            message = env->NewStringUTF(ackStr);
            env->CallVoidMethod(obj, methodID, message);
            env->DeleteLocalRef(message);
            usleep(500000);

            iSect++;
            if (xLen - iPos > 4096) {
                // "W 1073742336 4096\r\n"
                strcpy(sentStr, "W 1073742336 4096\r\n");
                strcpy(ackStr, sentStr);
                strcat(ackStr, "0\r\n");
                if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
                    return CMD_SEND_AND_RECIVE_CHECK_ERR;
                }
                message = env->NewStringUTF(sentStr);
                env->CallVoidMethod(obj, methodID, message);
                env->DeleteLocalRef(message);
                usleep(500000);
                message = env->NewStringUTF(ackStr);
                env->CallVoidMethod(obj, methodID, message);
                env->DeleteLocalRef(message);
                usleep(500000);

            } else {
                // "W 1073742336 X\r\n"
                sprintf(sentStr, "W 1073742336 %d\r\n", xLen - iPos);
                strcpy(ackStr, sentStr);
                strcat(ackStr, "0\r\n");
                if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
                    return CMD_SEND_AND_RECIVE_CHECK_ERR;
                }
                message = env->NewStringUTF(sentStr);
                env->CallVoidMethod(obj, methodID, message);
                env->DeleteLocalRef(message);
                usleep(500000);
                message = env->NewStringUTF(ackStr);
                env->CallVoidMethod(obj, methodID, message);
                env->DeleteLocalRef(message);
                usleep(500000);
            }

        } else {
            outbuf[oPos++] = ENCODE_BYTE(linelen);

            for (linecnt = linelen; linecnt > 0; linecnt -= 3) {
                outbuf[oPos++] = ENCODE_BYTE((sBuf[iPos] & 0xFC) >> 2);
                outbuf[oPos++] = ENCODE_BYTE(
                        ((sBuf[iPos] & 0x03) << 4) + ((sBuf[iPos + 1] & 0xF0) >> 4));
                outbuf[oPos++] = ENCODE_BYTE(
                        ((sBuf[iPos + 1] & 0x0F) << 2) + ((sBuf[iPos + 2] & 0xC0) >> 6));
                outbuf[oPos++] = ENCODE_BYTE(sBuf[iPos + 2] & 0x3F);
                xCrc += (sBuf[iPos] + sBuf[iPos + 1] + sBuf[iPos + 2]);
                iPos += 3;
            }

            outbuf[oPos++] = '\r';
            outbuf[oPos++] = '\n';
            outbuf[oPos++] = '\0';
            stringlen = strlen(outbuf);
            if (0 != SendDataToFlash(ptty, outbuf, stringlen)) {
                return SEND_DATA_TO_FLASH_ERR1;
            }
            message = env->NewStringUTF(outbuf);
            env->CallVoidMethod(obj, methodID, message);
            env->DeleteLocalRef(message);
            usleep(500000);


            oPos = 0;
            if (((iPos % 4096) % 900) == 0) {
                dataCnt++;
                tmp = (dataCnt * 900 * 100) / (float) xLen;
                printf("Programming flash.................[%d%%]\r", (unsigned int) tmp);
                fflush(stdout);

                sprintf(sentStr, "%d\r\n", xCrc);
                sprintf(ackStr, "%d\r\nOK\r\n", xCrc);
                if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
                    return CMD_SEND_AND_RECIVE_CHECK_ERR;
                }
                message = env->NewStringUTF(sentStr);
                env->CallVoidMethod(obj, methodID, message);
                env->DeleteLocalRef(message);
                usleep(500000);
                message = env->NewStringUTF(ackStr);
                env->CallVoidMethod(obj, methodID, message);
                env->DeleteLocalRef(message);
                usleep(500000);

                xCrc = 0;
            }
        }
    }

    message = env->NewStringUTF("CRC\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);

    if (0 != ((iPos % 4096) % 900)) {
        sprintf(sentStr, "%d\r\n", xCrc);
        sprintf(ackStr, "%d\r\nOK\r\n", xCrc);
        if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
            return CMD_SEND_AND_RECIVE_CHECK_ERR;
        }

        sprintf(messageBuffer, "%d\r\n", xCrc);
        message = env->NewStringUTF(messageBuffer);
        env->CallVoidMethod(obj, methodID, message);
        env->DeleteLocalRef(message);
        usleep(500000);
        sprintf(messageBuffer, "%d\r\nOK\r\n", xCrc);
        message = env->NewStringUTF(messageBuffer);
        env->CallVoidMethod(obj, methodID, message);
        env->DeleteLocalRef(message);
        usleep(500000);
    }


    message = env->NewStringUTF("Prepare Sector(s) for Write Operation\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);

    if (CHIPLPC2132 == chipflag) {
        if (iSect >= 8) {
            // >32k
            sprintf(sentStr, "P %d %d\r\n", 8, 8);
        } else {
            // <32k
            sprintf(sentStr, "P %d %d\r\n", iSect, iSect);
        }
    } else if (CHIPLPC2136 == chipflag) {
        tmpSect = iSect;
        if ((tmpSect > 7) && (tmpSect < 62.5)) {
            tmpSect = tmpSect / 8 + 7;
        }
        sprintf(sentStr, "P %d %d\r\n", tmpSect, tmpSect);
    } else if (CHIPLPC2138 == chipflag) {
        tmpSect = iSect;
        if ((tmpSect > 7) && (tmpSect < 125)) {
            if (tmpSect < 120) {
                tmpSect = tmpSect / 8 + 7;
            } else {
                tmpSect = tmpSect - 120 + 22;
            }
        }
        sprintf(sentStr, "P %d %d\r\n", tmpSect, tmpSect);
    }

    strcpy(ackStr, sentStr);
    strcat(ackStr, "0\r\n");
    if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
        return CMD_SEND_AND_RECIVE_CHECK_ERR;
    }
    message = env->NewStringUTF(sentStr);
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);
    message = env->NewStringUTF(ackStr);
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);


    message = env->NewStringUTF("Copy data from RAM to Flash(4K+CRC)\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);

    sprintf(sentStr, "C %d 1073742336 4096\r\n", iSect * 4096);
    strcpy(ackStr, sentStr);
    strcat(ackStr, "0\r\n");
    if (0 != CmdSendandRecvCheck(ptty, sentStr, ackStr)) {
        return CMD_SEND_AND_RECIVE_CHECK_ERR;
    }
    sprintf(messageBuffer, "C %d 1073742336 4096\r\n", iSect * 4096);
    message = env->NewStringUTF(messageBuffer);
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);


    message = env->NewStringUTF("Programming Flash.................[Done]\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);

    message = env->NewStringUTF("Finished.\r\n");
    env->CallVoidMethod(obj, methodID, message);
    env->DeleteLocalRef(message);
    usleep(500000);

    env->DeleteLocalRef(clazz);

    printf("Finished.\r");

    return BMC_UPDATE_SUCCESS;


}



