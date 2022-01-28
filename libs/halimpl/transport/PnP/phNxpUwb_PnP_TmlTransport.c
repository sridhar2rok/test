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

#include "UwbCore_Types.h"

#include "phNxpUwb_SpiTransport.h"
#include "phNxpUwb_DriverInterface.h"
#include "phUwb_BuildConfig.h"
#include "phOsalUwb.h"
#include "phUwbErrorCodes.h"
#include "UWB_Evt.h"

#define PAYLOAD_LEN_MSB  0x02
#define PAYLOAD_LEN_LSB  0x03
#define UCI_HDR_LEN      0x04
#define HBCI_HEADER_SIZE 0x04

#define NORMAL_MODE_LEN_OFFSET          0x03
#define EXTND_LEN_INDICATOR_OFFSET      0x01
#define EXTND_LEN_INDICATOR_OFFSET_MASK 0x80
#define EXTENDED_LENGTH_OFFSET          0x02

//static void *mIrqWaitSem = NULL;
//static void *mSyncMutex = NULL;

#define UCI_CMD             0x01
#define HBCI_CMD            0x02
#define HBCI_LAST_CMD       0x03
#define RESET               0x04
#define HBCI_QUERY_CMD      0x05
#define MCU_RESET           0x06 //MCU nvReset will be called.
#define RCI_CMD             0x0A
#define SWITCH_PROTOCOL_CMD 0x0B

static uint8_t tx_buffer[2060];

#if UWBIOT_UWBD_SR100T
static int phNxpUwb_Hbci_Read(uint8_t *rsp, uint16_t *rspLen)
{
    uint16_t payloadLen;
    int retStatus = RETURNED_SUCCESS;

    if (rsp != NULL && rspLen != NULL) {
        phNxpUwb_SpiRead(rsp, HBCI_HEADER_SIZE);
        payloadLen = (uint16_t)((rsp[PAYLOAD_LEN_MSB] << 8) | rsp[PAYLOAD_LEN_LSB]);
        if (payloadLen != 0) {
            phNxpUwb_SpiRead(&rsp[HBCI_HEADER_SIZE], payloadLen);
        }
        *rspLen = (uint8_t)(HBCI_HEADER_SIZE + payloadLen);
    }
    return retStatus;
}

static int phNxpUwb_Hbci_Write(uint8_t *data, uint16_t len)
{
    int ret = RETURNED_FAILURE;
    int write_status;
    if (data[0] == 0x51 && data[1] == 0x1 && len == 4) {
        tx_buffer[0] = HBCI_LAST_CMD;
    }
    else {
        tx_buffer[0] = HBCI_CMD;
    }
    tx_buffer[1] = (len >> 8u * 1u) & 0xFFu;
    tx_buffer[2] = (len >> 8u * 0u) & 0xFFu;
    memcpy(&tx_buffer[3], data, len);
    write_status = phNxpUwb_SpiWrite((uint8_t *)tx_buffer, 3 + len);
    if (write_status == 0) {
        ret = RETURNED_SUCCESS;
    }
    return ret;
}
#elif UWBIOT_UWBD_SR040
uint16_t phNxpUwb_RciRead(uint8_t *rsp)
{
    uint16_t bytesRead       = 0;
    uint32_t uciPacketLength = 0;

    if (rsp != NULL) {
        if (phNxpUwb_SpiRead(rsp, UCI_HDR_LEN) == RETURNED_SUCCESS) {
            uciPacketLength = (uint32_t)(*(rsp + 3));
            if (uciPacketLength != 0) {
                phNxpUwb_SpiRead(rsp + UCI_HDR_LEN, uciPacketLength);
            }
            //TODO: For now we are reading more than 255 bytes, hence typecasting from uint32 to uint16
            bytesRead = (uint16_t)(uciPacketLength) + UCI_HDR_LEN;
        }
        else {
            bytesRead = 0;
        }
    }
    return bytesRead;
}

int phNxpUwb_RciWrite(uint8_t *data, uint16_t len)
{
    int ret      = RETURNED_FAILURE;
    tx_buffer[0] = RCI_CMD;
    tx_buffer[1] = (len >> 8u * 1u) & 0xFFu;
    tx_buffer[2] = (len >> 8u * 0u) & 0xFFu;
    memcpy(&tx_buffer[3], data, len);
    ret = phNxpUwb_SpiWrite((uint8_t *)tx_buffer, 3 + len);
    return ret;
}
void phNxpUwb_SwitchProtocol(interface_handler_t protocolType)
{
    uint8_t rx_buffer[4] = {0};
    tx_buffer[0]         = SWITCH_PROTOCOL_CMD;
    tx_buffer[1]         = 0;
    tx_buffer[2]         = 1;
    tx_buffer[3]         = (uint8_t)protocolType;
    phNxpUwb_SpiWrite((uint8_t *)tx_buffer, 4);

    do {
        phNxpUwb_SpiRead((uint8_t *)rx_buffer, 3);
    } while (rx_buffer[2] != (uint8_t)protocolType);
}
bool phNxpUwb_HeliosReset(void)
{
    /* Not used in PnP tml */
    return true;
}
#endif

int phNxpUwb_HeliosInit()
{
    int ret              = RETURNED_FAILURE;
    uint8_t rx_buffer[4] = {0};
    int write_status;
    ret = phNxpUwb_SpiInit();
    if (RETURNED_SUCCESS != ret) {
        return ret;
    }
    tx_buffer[0] = RESET;
    tx_buffer[1] = 0x00;
    tx_buffer[2] = 0x00;
    write_status = phNxpUwb_SpiWrite((uint8_t *)tx_buffer, 3);
    if (write_status == 0) {
        ret = RETURNED_SUCCESS;
    }

    /* FIXME: QN9090+SR040 takes some time */
#if UWBIOT_TML_SOCKET
    Sleep(150);
#else
    vTaskDelay(pdMS_TO_TICKS(150)); /* wait for some time for writer */
#endif

    phNxpUwb_SpiRead((uint8_t *)rx_buffer, UCI_HDR_LEN);
    return ret;
}

void phNxpUwb_HeliosDeInit()
{
    phNxpUwb_SpiDeInit();
}

#if UWBIOT_UWBD_SR100T
int phNxpUwb_HbciTransceive(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rspLen)
{
    int retStatus;
    *rspLen   = 0;
    retStatus = phNxpUwb_Hbci_Write(data, len);
    if (retStatus == RETURNED_FAILURE) {
    }
    //phOsalUwb_Delay(100);
    retStatus = phNxpUwb_Hbci_Read(rsp, rspLen);
    if (retStatus == RETURNED_FAILURE) {
    }
    return retStatus;
}
#endif

uint16_t phNxpUwb_UciRead(uint8_t *rsp)
{
    uint16_t bytesRead = 0;
    uint16_t payloadLen;

    if (rsp != NULL) {
        if (phNxpUwb_SpiRead(rsp, UCI_HDR_LEN) == RETURNED_SUCCESS) {
            payloadLen = (uint16_t)((rsp[PAYLOAD_LEN_MSB] << 8) | rsp[PAYLOAD_LEN_LSB]);
            if (payloadLen != 0) {
                phNxpUwb_SpiRead(&rsp[UCI_HDR_LEN], payloadLen);
            }
            bytesRead = (uint16_t)(UCI_HDR_LEN + payloadLen);
        }
    }
    return bytesRead;
}

int phNxpUwb_UciWrite(uint8_t *data, uint16_t len)
{
    int ret      = RETURNED_FAILURE;
    tx_buffer[0] = UCI_CMD;
    tx_buffer[1] = (len >> 8u * 1u) & 0xFFu;
    tx_buffer[2] = (len >> 8u * 0u) & 0xFFu;
    memcpy(&tx_buffer[3], data, len);
    ret = phNxpUwb_SpiWrite((uint8_t *)tx_buffer, 3 + len);
    if (ret == RETURNED_SUCCESS) {
        return len;
    }
    else {
        return ret;
    }
}

void phNxpUwb_HeliosIrqEnable(void)
{
}
