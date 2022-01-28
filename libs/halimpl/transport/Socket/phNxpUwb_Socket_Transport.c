/*
 * Copyright 2019,2020 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif
#include "UwbCore_Types.h"

#include "phNxpUwb_SpiTransport.h"
#include "phNxpUwb_DriverInterface.h"
#include "phNxpUwb_Socket_const.h"
#include "phUwb_BuildConfig.h"
#include "phOsalUwb.h"
#include "UWB_Evt.h"
#include "phUwbErrorCodes.h"

void board_closesocket();
void board_SerialGetSocket(char *SocketName, size_t *pInOutSocketNameLen);
int board_opensocket(char *SocketName, size_t socketNameLen);
int board_socket_writeData(uint8_t *input, uint16_t inLen, uint8_t *output, uint16_t *outLen);

static uint8_t tx_buffer[2060];

int phNxpUwb_HeliosInit()
{
    int ret              = 0;
    char socketName[32]  = {0};
    size_t socketNameLen = sizeof(socketName) - 1;

    board_SerialGetSocket(socketName, &socketNameLen);

    ret = board_opensocket(socketName, socketNameLen);
    if (ret == -1) {
        board_closesocket();
        return RETURNED_FAILURE;
    }

    tx_buffer[0] = UWB_SKT_INIT_FUNC;
    tx_buffer[1] = 0;
    tx_buffer[2] = 0;
    tx_buffer[3] = 0;
    tx_buffer[4] = 0;

    ret = board_socket_writeData(tx_buffer, 5, NULL, 0);
    return ret;
}

void phNxpUwb_HeliosDeInit()
{
    tx_buffer[0] = UWB_SKT_DEINIT_FUNC;
    tx_buffer[1] = 0;
    tx_buffer[2] = 0;
    tx_buffer[3] = 0;
    tx_buffer[4] = 0;

    board_socket_writeData(tx_buffer, 5, NULL, 0);
    board_closesocket();
}

int phNxpUwb_HbciTransceive(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rspLen)
{
    int ret = 0;

    tx_buffer[0] = UWB_SKT_HBCI_TRANSRECEIVE_FUNC;
    tx_buffer[1] = len & 0xFF;
    tx_buffer[2] = (len >> 8) & 0xFF;
    tx_buffer[3] = (*rspLen) & 0xFF;
    tx_buffer[4] = ((*rspLen) >> 8) & 0xFF;

    memcpy(&tx_buffer[5], data, len);

    ret = board_socket_writeData(tx_buffer, len + 5, rsp, rspLen);
    return ret;
}

uint16_t phNxpUwb_UciRead(uint8_t *rsp)
{
    uint16_t readBytes = 0;

    tx_buffer[0] = UWB_SKT_UCI_READ_FUNC;
    tx_buffer[1] = 0;
    tx_buffer[2] = 0;
    tx_buffer[3] = 0;
    tx_buffer[4] = 0;

    (void)board_socket_writeData(tx_buffer, 5, rsp, &readBytes);

    return readBytes;
}

int phNxpUwb_UciWrite(uint8_t *data, uint16_t len)
{
    int ret = 0;

    tx_buffer[0] = UWB_SKT_UCI_WRITE_FUNC;
    tx_buffer[1] = len & 0xFF;
    tx_buffer[2] = (len >> 8) & 0xFF;
    tx_buffer[3] = 0;
    tx_buffer[4] = 0;

    memcpy(&tx_buffer[5], data, len);

    ret = board_socket_writeData(tx_buffer, len + 5, NULL, 0);
    return ret;
}

uint16_t phNxpUwb_RciRead(uint8_t *rsp)
{
    int ret            = 0;
    uint16_t readBytes = 0;

    tx_buffer[0] = UWB_SKT_RCI_READ_FUNC;
    tx_buffer[1] = 0;
    tx_buffer[2] = 0;
    tx_buffer[3] = 0;
    tx_buffer[4] = 0;

    ret = board_socket_writeData(tx_buffer, 5, rsp, &readBytes);
    return ret;
}

int phNxpUwb_RciWrite(uint8_t *data, uint16_t len)
{
    int ret = 0;

    tx_buffer[0] = UWB_SKT_RCI_WRITE_FUNC;
    tx_buffer[1] = len & 0xFF;
    tx_buffer[2] = (len >> 8) & 0xFF;
    tx_buffer[3] = 0;
    tx_buffer[4] = 0;

    memcpy(&tx_buffer[5], data, len);

    ret = board_socket_writeData(tx_buffer, len + 5, NULL, 0);
    return ret;
}

void phNxpUwb_HeliosIrqEnable(void)
{
    return;
}

void phNxpUwb_sr040Reset()
{
    /* Not used in Socket TML */
    return;
}

void phNxpUwb_SwitchProtocol(interface_handler_t protocolType)
{
    tx_buffer[0] = UWB_SKT_SWITCH_PROTOCOL_FUNC;
    tx_buffer[1] = *(uint8_t *)&protocolType;
    tx_buffer[2] = 1;
    tx_buffer[3] = 0;
    tx_buffer[4] = 0;

    board_socket_writeData(tx_buffer, 5, NULL, 0);
}

bool phNxpUwb_HeliosReset(void)
{
    /* Not used in Socket tml */
    return true;
}
