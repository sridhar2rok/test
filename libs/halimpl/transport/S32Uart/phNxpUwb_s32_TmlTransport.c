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
#include "phUwb_BuildConfig.h"
#include "phOsalUwb.h"
#include "phUwbErrorCodes.h"
#include "phNxpLogApis_TmlUwb.h"

#include "UWB_Evt.h"
#include "uwb_logging.h"

#if UWBIOT_TML_SOCKET
#include <windows.h>
#endif

#define PAYLOAD_LEN_MSB  0x02
#define PAYLOAD_LEN_LSB  0x03
#define UCI_HDR_LEN      0x04
#define HBCI_HEADER_SIZE 0x04

#define NORMAL_MODE_LEN_OFFSET          0x03
#define EXTND_LEN_INDICATOR_OFFSET      0x01
#define EXTND_LEN_INDICATOR_OFFSET_MASK 0x80
#define EXTENDED_LENGTH_OFFSET          0x02

#define UCI               0x00
#define GET_BOARD_ID      0x80
#define GET_BOARD_VERSION 0x81
#define HARD_RESET        0x82
#define SET_SPI_RATE      0x83
#define GET_SPI_RATE      0x84
#define SWITCH_PROTOCOL   0x85
#define SWUP              0x05
#define ERROR_MSG         0xFF

#define USB_RESPONSE_HEADER_LEN 0x06
#define USB_WRITE_HEADER_OFFSET 0x06
#define COMMAND                 0x00
#define RESPONSE                0x01

static uint8_t tx_buffer[2060];
static uint8_t rx_buffer[2060];

#define MAX_RETRY_COUNT 9
#if UWBIOT_TML_SOCKET
#define phOsalUwb_Delay Sleep
#endif

/* Used only for HARD_RESET currently */
static void sendCommand(uint8_t commandType, uint8_t *buffer, uint32_t length)
{
    int write_status, read_status;
    uint32_t payloadLength = 0;
    uint8_t max_retry      = 0;

    tx_buffer[0] = commandType;
    tx_buffer[1] = COMMAND;
    tx_buffer[2] = (uint8_t)((length >> (8 * 0)) & 0xFF);
    tx_buffer[3] = (uint8_t)((length >> (8 * 1)) & 0xFF);
    tx_buffer[4] = (uint8_t)((length >> (8 * 2)) & 0xFF);
    tx_buffer[5] = (uint8_t)((length >> (8 * 3)) & 0xFF);

    /* Copy buffer into tx_buffer */
    if (length <= 2054) {
        memcpy(&tx_buffer[6], buffer, length);
    }
    else {
        /* Very long data */
        return;
    }

    LOG_TX("Proprietary TX ", tx_buffer, USB_WRITE_HEADER_OFFSET + length);
    write_status = phNxpUwb_SpiWrite(tx_buffer, USB_WRITE_HEADER_OFFSET + length);
    if (commandType == HARD_RESET) {
        /* Upon HardReset, if we read immediately, S32 always returns with 0xFF, inorder to
           give S32 some time for Resetting its state, this delay is added.*/
        phOsalUwb_Delay(50);
    }

    if (write_status == RETURNED_SUCCESS) {
    retry:
        phOsalUwb_SetMemory(rx_buffer, 0, sizeof(rx_buffer));
        read_status = phNxpUwb_SpiRead(rx_buffer, USB_RESPONSE_HEADER_LEN);
        if ((RETURNED_FAILURE == read_status) && (max_retry < MAX_RETRY_COUNT)) {
            max_retry++;
            goto retry;
        }
        else if (RETURNED_SUCCESS == read_status) {
            LOG_RX("Proprietary RX ", rx_buffer, USB_RESPONSE_HEADER_LEN);
            if (rx_buffer[0] == HARD_RESET && rx_buffer[1] == RESPONSE) {
                /* hard Reset Command does not have any payload
                hence return immediately */
                return;
            }
            /* If the Response is other than Hard Reset, then read payload and ignore it */
            payloadLength = (uint32_t)((rx_buffer[5] << 8u * 3u) | (rx_buffer[4] << 8u * 2u) |
                                       (rx_buffer[3] << 8u * 1u) | (rx_buffer[2] << 8u * 0u));

            if (payloadLength > 0) {
                phNxpUwb_SpiRead(rx_buffer, payloadLength);
                LOG_RX("Proprietary RX ", rx_buffer, payloadLength);
            }
        }
        else {
        }
    }
    else {
        LOG_E("ERROR! Send Failed");
    }

    return;
}

int phNxpUwb_HeliosInit()
{
    int ret = RETURNED_FAILURE;
    ret     = phNxpUwb_SpiInit();
    if (RETURNED_SUCCESS != ret) {
        return ret;
    }
    uint8_t flush_buffer[256] = {0};
    int read_len              = 0;
    do {
        read_len = phNxpUwb_SpiRead(flush_buffer, sizeof(flush_buffer));
        if (read_len > 0) {
            LOG_D("Flushed phNxpUwb_SpiRead()");
        }
    } while (read_len > 0);

    phNxpUwb_SwitchProtocol(kInterfaceModeUci);
    sendCommand(HARD_RESET, NULL, 0);
    return ret;
}

void phNxpUwb_SwitchProtocol(interface_handler_t protocolType)
{
    sendCommand(SWITCH_PROTOCOL, (uint8_t *)&protocolType, 0x01);
}

void phNxpUwb_HeliosDeInit()
{
    phNxpUwb_SpiDeInit();
}

uint16_t phNxpUwb_UciRead(uint8_t *rsp)
{
    uint16_t bytesRead       = 0;
    uint32_t uciPacketLength = 0;
    uint32_t retryCount      = 5;

    if (rsp != NULL) {
    retry:
        phOsalUwb_SetMemory(rx_buffer, 0, sizeof(rx_buffer));
        if (phNxpUwb_SpiRead(rx_buffer, USB_RESPONSE_HEADER_LEN) == RETURNED_SUCCESS) {
            uciPacketLength = (uint32_t)((rx_buffer[5] << 8u * 3u) | (rx_buffer[4] << 8u * 2u) |
                                         (rx_buffer[3] << 8u * 1u) | (rx_buffer[2] << 8u * 0u));
            if (uciPacketLength != 0) {
                if (rx_buffer[0] == ERROR_MSG) {
                    phNxpUwb_SpiRead(rx_buffer, uciPacketLength);
                    if (retryCount-- > 0) {
                        goto cleanup;
                    }
                    goto retry;
                }
                phNxpUwb_SpiRead(rsp, uciPacketLength);
            }
            //TODO: For now we are reading more than 255 bytes, hence typecasting from uint32 to uint16
            bytesRead = (uint16_t)(uciPacketLength);
        }
        else {
            bytesRead = 0;
        }
    }
cleanup:
    return bytesRead;
}

int phNxpUwb_UciWrite(uint8_t *data, uint16_t len)
{
    int write_status;
    tx_buffer[0] = UCI;
    tx_buffer[1] = COMMAND;
    tx_buffer[2] = (len >> 8u * 0u) & 0xFFu;
    tx_buffer[3] = (len >> 8u * 1u) & 0xFFu;
    tx_buffer[4] = 0x00;
    tx_buffer[5] = 0x00;
    phOsalUwb_MemCopy(&tx_buffer[USB_WRITE_HEADER_OFFSET], data, len);

    write_status = phNxpUwb_SpiWrite((uint8_t *)tx_buffer, USB_WRITE_HEADER_OFFSET + len);
    if (write_status >= 0) {
        return len;
    }
    else {
        return write_status;
    }
}

uint16_t phNxpUwb_RciRead(uint8_t *rsp)
{
    uint16_t rciFrameLength  = 0;
    int readReturn           = RETURNED_FAILURE;
    uint32_t rciPacketLength = 0;
    int retryCount           = 0;

    if (rsp != NULL) {
    retry:
        phOsalUwb_SetMemory(rx_buffer, 0, sizeof(rx_buffer));
        readReturn = phNxpUwb_SpiRead(rx_buffer, USB_RESPONSE_HEADER_LEN);
        if ((RETURNED_FAILURE == readReturn) && (retryCount < MAX_RETRY_COUNT)) {
            retryCount++;
            goto retry;
        }
        else if (RETURNED_SUCCESS == readReturn) {
            rciPacketLength = (uint32_t)((rx_buffer[5] << 8u * 3u) | (rx_buffer[4] << 8u * 2u) |
                                         (rx_buffer[3] << 8u * 1u) | (rx_buffer[2] << 8u * 0u));
            if (rciPacketLength > 0) {
                if (rx_buffer[0] == ERROR_MSG) {
                    phNxpUwb_SpiRead(rx_buffer, rciPacketLength);
                    goto retry;
                }
                /*Read complete RCI Packet*/
                phNxpUwb_SpiRead(rsp, rciPacketLength);
            }
            //TODO: For now we are reading more than 255 bytes, hence typecasting from uint32 to uint16
            rciFrameLength = (uint16_t)(rciPacketLength);
        }
        else {
            /*Received UART Frame does not contain any RCI Packet*/
            rciFrameLength = 0;
        }
    }
    return rciFrameLength;
}

int phNxpUwb_RciWrite(uint8_t *data, uint16_t len)
{
    int ret = RETURNED_FAILURE;
    int write_status;
    tx_buffer[0] = SWUP;
    tx_buffer[1] = COMMAND;
    tx_buffer[2] = (len >> 8u * 0u) & 0xFFu;
    tx_buffer[3] = (len >> 8u * 1u) & 0xFFu;
    tx_buffer[4] = 0x00;
    tx_buffer[5] = 0x00;
    phOsalUwb_MemCopy(&tx_buffer[USB_WRITE_HEADER_OFFSET], data, len);

    write_status = phNxpUwb_SpiWrite((uint8_t *)tx_buffer, USB_WRITE_HEADER_OFFSET + len);
    if (write_status >= 0) {
        ret = RETURNED_SUCCESS;
    }
    return ret;
}

void phNxpUwb_sr040Reset()
{
    /* Not used in UART tml */
}

bool phNxpUwb_HeliosReset(void)
{
    /* Not used in UART tml */
    return true;
}
