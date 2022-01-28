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
#include "tmlUCIRPC.h"
#include "phNxpLogApis_TmlUwb.h"
#include "phUwbErrorCodes.h"

#define PAYLOAD_LEN_MSB  0x02
#define PAYLOAD_LEN_LSB  0x03
#define UCI_HDR_LEN      0x04
#define HBCI_HEADER_SIZE 0x04

#define NORMAL_MODE_LEN_OFFSET          0x03
#define EXTND_LEN_INDICATOR_OFFSET      0x01
#define EXTND_LEN_INDICATOR_OFFSET_MASK 0x80
#define EXTENDED_LENGTH_OFFSET          0x02

static void *mIrqWaitSem = NULL;
static void *mSyncMutex  = NULL;

void phNxpUwb_GpioIrqEnable(void (*cb)(void *args))
{
    //PRINT_ENTRY();
    //ENABLE_HELIOS_IRQ(cb);
    Sleep(100);
    cb(NULL);
}

void phNxpUwb_GpioIrqDisable(void (*cb)(void *args))
{
    //PRINT_ENTRY();
    //DISABLE_HELIOS_IRQ();
    Sleep(100);
    //cb(NULL);
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

int phNxpUwb_HeliosInit()
{
    rpcTxnStatus_t rpc_status;
    /*This semaphore is signaled in the ISR context.*/
    if (UWBSTATUS_SUCCESS != phOsalUwb_CreateSemaphore(&mIrqWaitSem, 0)) {
        NXPLOG_UWB_TML_D("Error: UWB_HeliosInit(), could not create semaphore mWaitIrqSem\n");
        return false;
    }

    /*This mutex is used to make SPI read and write operations mutually exclusive*/
    if (UWBSTATUS_SUCCESS != phOsalUwb_CreateMutex(&mSyncMutex)) {
        NXPLOG_UWB_TML_D("Error: UWB_HeliosInit(), could not create mutex mSyncMutex\n");
        return false;
    }

    //if (RETURNED_SUCCESS != phNxpUwb_SpiInit()) {
    //    return false;
    //}
    rpc_status = tmlUCIRPC_Init();
    if (kRpcTxnStatus_SUCCESS == rpc_status) {
        return true;
    }
    else {
        return false;
    }
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
    int retStatus = RETURNED_FAILURE;
    rpcTxnStatus_t rpc_status;
    {
        printf("HBCI TX Len=%d\n", len);
#if 0
        for (unsigned long i = 0; i < len; i++) {
            printf("%02X ", 0xFF & (data)[i]);
        }
        printf("\n");
#endif
    }
    rpc_status = tmlUCIRPC_HBCI_TRx(data, len, rsp, rspLen);
    if (kRpcTxnStatus_SUCCESS == rpc_status) {
        retStatus = RETURNED_SUCCESS;
    }

    return retStatus;
}

uint16_t xphNxpUwb_UciRead(uint8_t *rsp)
{
    uint16_t bytesRead = 0;

    rpcTxnStatus_t rpc_status;
    rpc_status = tmlUCIRPC_UCI_Rx(rsp, &bytesRead);
    if (kRpcTxnStatus_SUCCESS == rpc_status) {
    }
    else {
        bytesRead = 0;
    }
    return bytesRead;
}

int phNxpUwb_UciWrite(uint8_t *data, uint16_t len)
{
    int bytesWrote = -1;
    rpcTxnStatus_t rpc_status;
    rpc_status = tmlUCIRPC_UCI_Tx(data, len);
    if (kRpcTxnStatus_SUCCESS == rpc_status) {
        bytesWrote = len;
    }
    else {
    }
    return bytesWrote;
}

void phNxpUwb_HeliosIrqEnable(void)
{
    phNxpUwb_GpioIrqEnable(heliosIrqCb);
}
