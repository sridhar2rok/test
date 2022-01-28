/*
  * Copyright (C) 2012-2020 NXP Semiconductors
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

#include "phNxpUwb_DriverInterface.h"
#include "phNxpUwb_SpiTransport.h"
#include "phOsalUwb.h"
#include "phUwb_BuildConfig.h"
#include "phNxpLogApis_TmlUwb.h"
#include "phUwbErrorCodes.h"

#define PAYLOAD_LEN_MSB 0x02
#define PAYLOAD_LEN_LSB 0x03

#define UCI_RX_HDR_LEN             0x04
#define NORMAL_MODE_LEN_OFFSET     0x03
#define EXTND_LEN_INDICATOR_OFFSET 0x01
#define EXTENDED_LENGTH_OFFSET     0x02

#define UCI_HDR_LEN      0x04
#define HBCI_HEADER_SIZE 0x04

#define EXTND_LEN_INDICATOR_OFFSET_MASK 0x80

static void *mIrqWaitSem = NULL;
static void *mSyncMutex  = NULL;

/* below function is based on the CPU speed of 48MHz,
   6 iteration of while loop takes 1us*/
void AddDelayInMicroSec(int delay)
{
    volatile int cnt = 6 * delay;
    while (cnt--)
        ;
}

#if UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V2
static void heliosIrqCb(void)
{
#elif (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V3)
static void heliosIrqCb(void *args)
{
#endif
    // Disable interrupt
    phNxpUwb_GpioIrqDisable(heliosIrqCb);

    // Signal TML read task
    phOsalUwb_ProduceSemaphore(mIrqWaitSem);
}

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
    if (data != NULL && len > 0) {
        return phNxpUwb_SpiWrite(data, len);
    }
    else {
        return RETURNED_FAILURE;
    }
}

int phNxpUwb_HeliosInit()
{
    /*This semaphore is signaled in the ISR context.*/
    if (UWBSTATUS_SUCCESS != phOsalUwb_CreateSemaphore(&mIrqWaitSem, 0)) {
        NXPLOG_UWB_TML_D("Error: UWB_HeliosInit(), could not create semaphore mWaitIrqSem\n");
        return UWBSTATUS_FAILED;
    }

    /*This mutex is used to make SPI read and write operations mutually exclusive*/
    if (UWBSTATUS_SUCCESS != phOsalUwb_CreateMutex(&mSyncMutex)) {
        NXPLOG_UWB_TML_D("Error: UWB_HeliosInit(), could not create mutex mSyncMutex\n");
        return UWBSTATUS_FAILED;
    }

    phNxpUwb_SpiInit();
    return UWBSTATUS_SUCCESS;
}

void phNxpUwb_HeliosDeInit()
{
    if (UWBSTATUS_SUCCESS != phOsalUwb_ProduceSemaphore(mIrqWaitSem)) {
        NXPLOG_UWB_TML_D("Error: UWB_HeliosInit(), could not post semaphore mIrqWaitSem\n");
    }

    phOsalUwb_DeleteSemaphore(&mIrqWaitSem);
    mIrqWaitSem = NULL;

    phOsalUwb_DeleteMutex(&mSyncMutex);
    mSyncMutex = NULL;
}

int phNxpUwb_HbciTransceive(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rspLen)
{
    int retStatus;

    *rspLen = 0;

    retStatus = phNxpUwb_Hbci_Write(data, len);
    if (retStatus == RETURNED_FAILURE) {
        goto end;
    }

    phNxpUwb_GpioIrqEnable(heliosIrqCb);

    if (phOsalUwb_ConsumeSemaphore(mIrqWaitSem) != UWBSTATUS_SUCCESS) {
        goto end;
    }

    retStatus = phNxpUwb_Hbci_Read(rsp, rspLen);
    if (retStatus == RETURNED_FAILURE) {
        goto end;
    }

end:
    AddDelayInMicroSec(50);
    return retStatus;
}

uint16_t phNxpUwb_UciRead(uint8_t *rsp)
{
    uint8_t status;
    uint16_t payloadLen;
    uint16_t bytesRead            = 0;
    uint16_t len                  = UCI_RX_HDR_LEN;
    uint16_t totalBtyesToRead     = 0;
    uint32_t IsExtndLenIndication = 0;
    uint8_t count                 = 0;
start:

    phNxpUwb_GpioIrqEnable(heliosIrqCb);

    if (phOsalUwb_ConsumeSemaphore(mIrqWaitSem) != UWBSTATUS_SUCCESS) {
        goto end;
    }

    phOsalUwb_LockMutex(mSyncMutex);
    AddDelayInMicroSec(1);

    if (!phNxpUwb_HeliosInteruptStatus()) {
        phOsalUwb_UnlockMutex(mSyncMutex);
        goto start;
    }

    /*Read Ready Indication from HOST to Helios*/
    phNxpUwb_RtcSyncWrite(true);
    while (phNxpUwb_HeliosInteruptStatus()) {
        if (count == 100) {
            break;
        }
        AddDelayInMicroSec(10);
        count++;
    }

    phNxpUwb_GpioIrqEnable(heliosIrqCb);

    if (phOsalUwb_ConsumeSemaphore(mIrqWaitSem) != UWBSTATUS_SUCCESS) {
        goto end;
    }
    count = 0;
    while (!phNxpUwb_HeliosInteruptStatus()) {
        if (count == 10) {
            phNxpUwb_RtcSyncWrite(false);
            phOsalUwb_UnlockMutex(mSyncMutex);
            return bytesRead;
        }
        USLEEP(1000);
        count++;
    }

    status = phNxpUwb_SpiRead(rsp, len);
    if (status != RETURNED_FAILURE) {
        bytesRead = (uint16_t)(bytesRead + len);
    }
    else {
        goto end;
    }

    IsExtndLenIndication = (rsp[EXTND_LEN_INDICATOR_OFFSET] & EXTND_LEN_INDICATOR_OFFSET_MASK);

    totalBtyesToRead = rsp[NORMAL_MODE_LEN_OFFSET];

    if (IsExtndLenIndication) {
        totalBtyesToRead = (uint16_t)((totalBtyesToRead << 8) | rsp[EXTENDED_LENGTH_OFFSET]);
    }

    payloadLen = totalBtyesToRead;

    if (payloadLen != 0) {
        status = phNxpUwb_SpiRead(&rsp[len], payloadLen);
        if (status != RETURNED_FAILURE) {
            bytesRead = (uint16_t)(bytesRead + payloadLen);
        }
    }

end:
    count = 0;
    while (phNxpUwb_HeliosInteruptStatus()) {
        if (count >= 20) { // Sleep of 500us * 20 = 10ms as per artf786394
            break;
        }
        AddDelayInMicroSec(500);
        count++;
    }

    phNxpUwb_RtcSyncWrite(false);

    phOsalUwb_UnlockMutex(mSyncMutex);

    return bytesRead;
}

int phNxpUwb_UciWrite(uint8_t *data, uint16_t len)
{
    int bytesWrote = -1;
    int status;

    phOsalUwb_LockMutex(mSyncMutex);
    if (data == NULL || len == 0) {
        goto end;
    }

    status = phNxpUwb_SpiWrite(data, UCI_HDR_LEN);
    if (status == RETURNED_FAILURE) {
        goto end;
    }

    AddDelayInMicroSec(50);
    bytesWrote = (uint16_t)(UCI_HDR_LEN);

    /* Send payload */
    if (len > UCI_HDR_LEN) {
        status = phNxpUwb_SpiWrite(&data[UCI_HDR_LEN], (uint16_t)(len - UCI_HDR_LEN));
        if (status == RETURNED_FAILURE) {
            bytesWrote = -1;
            goto end;
        }
        else {
            bytesWrote = len;
        }
    }

end:
    phOsalUwb_UnlockMutex(mSyncMutex);
    return bytesWrote;
}

void phNxpUwb_HeliosIrqEnable(void)
{
    phNxpUwb_GpioIrqEnable(heliosIrqCb);
}
