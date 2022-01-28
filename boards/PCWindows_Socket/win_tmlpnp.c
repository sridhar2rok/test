/* Copyright 2020 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */

#include <stdio.h>
#include <assert.h>
#include <board.h>
#include <string.h>
#include <stdlib.h>

#include <winsock2.h>
#include <Ws2tcpip.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "UWB_GpioIrq.h"

#define UWBIOT_DEFAULT_SOCKET "127.0.0.1:1025"
#define PRINT_ENTRY()         printf("DBG:TODO:%s\n", __FUNCTION__)
#define DEBUG_MSG             0

#pragma comment(lib, "Ws2_32.lib")

const char *gboard_configuration = "WINDOWS - TML - SOCKET";

xTaskHandle readerTaskHandle;
int stopreaderTask = 0;
SemaphoreHandle_t binSemphr;
char receiveBuffer[4096] = {
    0,
};
int receiveDatalen  = 0;
SOCKET ClientSocket = INVALID_SOCKET;

void readerTask(void *arg);

bool UWB_GpioSet(UWB_GpioComponent_t cp, bool on)
{
    PRINT_ENTRY();
    return true;
}

#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

void board_SerialGetSocket(char *SocketName, size_t *pInOutSocketNameLen)
{
    const char *socketName;
    const char defaultSocketName[] = UWBIOT_DEFAULT_SOCKET;
    const char *socketName_env     = getenv(UWBIOT_ENV_COM);
    if (socketName_env == NULL) {
        socketName = defaultSocketName;
    }
    else {
        socketName = socketName_env;
        printf("Using SocketName='%s' (ENV: %s=%s)\n", socketName, UWBIOT_ENV_COM, socketName);
    }

    strncpy(SocketName, socketName, MIN(strlen(socketName), *pInOutSocketNameLen));
}

static int getSocketParameters(const char *arg, char *szServer, int szServerLen, unsigned int *port)
{
    // the IP address is in format a.b.c.d:port, e.g. 10.0.0.1:8080
    int nSuccess;

    // First attempt at parsing: server IP-address passed, sscanf will return 2 upon successfull parsing
    nSuccess = sscanf(arg, "%15[0-9.]:%5u[0-9]", szServer, (unsigned int *)port);

    if (nSuccess == 2) {
        return 0; // Success
    }
    else {
        // Second attempt at parsing: server name passed instead of IP-address
        unsigned int i;
        int fColonFound = 0;

        for (i = 0; i < strlen(arg); i++) {
            if (arg[i] == ':') {
                szServer[i] = 0;
                fColonFound = 1;
                break;
            }
            else {
                szServer[i] = arg[i];
            }
        }

        if ((fColonFound == 1) && (i != 0)) {
            nSuccess = sscanf(&arg[i], ":%5u[0-9]", (unsigned int *)port);
            if ((nSuccess == 1)) {
                return 0; // Success
            }
        }
    }

    return -1; //FAIL
}

int board_opensocket(char *SocketName, size_t socketNameLen)
{
    int ret = 0;
    WSADATA wsaData;
    SOCKADDR_IN ServerAddress;
    char szServer[128];
    int szServerLen   = sizeof(szServer);
    unsigned int port = 0;

    ret = getSocketParameters(SocketName, szServer, szServerLen, (unsigned int *)&port);
    if (ret == -1) {
        printf("Error in connection string \n");
        return -1;
    }

    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0) {
        printf("Error in WSAStartup \n");
        return -1;
    }

    ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ClientSocket == INVALID_SOCKET) {
        printf("Could not open socket '%s' \n", SocketName);
        printf("NOTE: You can set '%s' to uwb_com_proxy address at run Time \n", UWBIOT_ENV_COM);
        return -1;
    }

    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port   = htons(port);
    InetPton(AF_INET, (szServer), &ServerAddress.sin_addr.s_addr);

    ret = connect(ClientSocket, (SOCKADDR *)&ServerAddress, sizeof(ServerAddress));
    if (ret != 0) {
        printf("\n Error in connecting to server.\n Start uwb_com_proxy.exe before running this example \n");
        return -1;
    }

    binSemphr = xSemaphoreCreateBinary();
    xSemaphoreGive(binSemphr);
    xSemaphoreTake(binSemphr, portMAX_DELAY);

    if (xTaskCreate(readerTask, "readerTask", 1024, NULL, 1, &readerTaskHandle) != TRUE) {
        printf("readerTask create Failed \n");
    }

    return 0;
}

void readerTask(void *arg)
{
    int bytestRead = 0;
    while (!stopreaderTask) {
        bytestRead = recv(ClientSocket, receiveBuffer, sizeof(receiveBuffer), 0);
#if DEBUG_MSG
        printf("Received bytes frok socket  %d \n", bytestRead);
#endif
        if (bytestRead > 0) {
            receiveDatalen = bytestRead;
            xSemaphoreGive(binSemphr);
        }
    }
    return;
}

void board_closesocket()
{
    closesocket(ClientSocket);
    shutdown(ClientSocket, SD_SEND);
    stopreaderTask = 1;
}

int board_socket_writeData(uint8_t *input, uint16_t inLen, uint8_t *output, uint16_t *outLen)
{
    int bytes_sent = 0; // Total bytes sent on socket,
    int ret        = -1;

    bytes_sent = send(ClientSocket, (const char *)input, inLen, 0);
    if (inLen != bytes_sent) {
        printf("board_socket_writeData: Error in sending data \n");
        goto exit;
    }

    if (output != NULL) {
        xSemaphoreTake(binSemphr, portMAX_DELAY);
        if (receiveDatalen > 4) {
            if (receiveBuffer[0] == 'E' && receiveBuffer[1] == 'R' && receiveBuffer[2] == 'R' &&
                receiveBuffer[3] == 'O' && receiveBuffer[4] == 'R') {
                printf("Error in reading data \n");
                *outLen = 0;
                return ret;
            }
        }
        *outLen = receiveDatalen;
        memcpy(output, receiveBuffer, receiveDatalen);
    }
    ret = 0;

exit:
    return ret;
}
