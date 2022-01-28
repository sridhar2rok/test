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
#include "driver_config.h"

#include "phNxpUwb_Common.h"
#include "phNxpUwb_SpiTransport.h"
#include "phNxpUwb_DriverInterface.h"
#include "phUwb_BuildConfig.h"
#include "phOsalUwb.h"
#include "UWBT_BuildConfig.h"
/*S32K includes*/
#include <S32K144.h>
#include <crc_driver.h>
#include <crc1.h>
#include <pin_control.h>
#include "uwb_iot_ftr_default.h"
#include "phNxpLogApis_TmlUwb.h"
#include "phUwbErrorCodes.h"

#define PAYLOAD_LEN_MSB 0x02
#define PAYLOAD_LEN_LSB 0x03

#define TIME_WAIT_MS                  100
#define TIME_WAIT_AFTER_OPERATION     200
#define TIME_WAIT_AFTER_CS_READ       10
#define TIME_WAIT_AFTER_CS_WRITE      10
#define TIMEOUT_DELAY                 5
#define WAIT_FOR_READY_AT_WRITE_COUNT 400

#define RCI_PKT_LEN                     256
#define UCI_HDR_LEN                     0x04
#define NORMAL_MODE_LEN_OFFSET          0x03
#define EXTND_LEN_INDICATOR_OFFSET      0x01
#define EXTND_LEN_INDICATOR_OFFSET_MASK 0x80
#define EXTENDED_LENGTH_OFFSET          0x02

/** @brief CRC module number */
#define PHSCA_S32CHECKSUM_CRC_INSTANCE (INST_CRC1)

static void *mIrqWaitSem      = NULL;
static void *mSyncMutex       = NULL;
static void *mReadyIrqWaitSem = NULL;

static void heliosIrqCb(void *args)
{
    // Disable interrupt
    phNxpUwb_GpioIrqDisable(heliosIrqCb);

    // Signal TML read task
    phOsalUwb_ProduceSemaphore(mIrqWaitSem);
}

static void readyIrqCb(void *args)
{
    // Disable interrupt
    phNxpUwb_RdyGpioIrqDisable(readyIrqCb);

    // Signal TML read task
    phOsalUwb_ProduceSemaphore(mReadyIrqWaitSem);
}

void phNxpUwb_SwitchProtocol(interface_handler_t protocolType)
{
    uint8_t resp[256];
    /* Flush read buffer */
    phOsalUwb_LockMutex(mSyncMutex);
    vTaskDelay(pdMS_TO_TICKS(1));
    phNxpUwb_SpiRead(resp, sizeof(resp) - 1);
    phOsalUwb_UnlockMutex(mSyncMutex);
}

static void crc16_xmodem_init()
{
    PCC->PCCn[PCC_CRC_INDEX] |= PCC_PCCn_CGC_MASK; /* enable CRC clock */
    /* Call init function*/
    (void)CRC_DRV_Init(PHSCA_S32CHECKSUM_CRC_INSTANCE, &crc1_InitConfig0);
}

static uint16_t crc16_xmodem(uint8_t *input, size_t len)
{
    uint16_t chkResult = 0u;

    CRC_DRV_Configure(PHSCA_S32CHECKSUM_CRC_INSTANCE, &crc1_InitConfig0);
    CRC_DRV_WriteData(PHSCA_S32CHECKSUM_CRC_INSTANCE, input, len);
    chkResult = CRC_DRV_GetCrcResult(PHSCA_S32CHECKSUM_CRC_INSTANCE);

    return chkResult;
}

int phNxpUwb_HeliosInit()
{
    /*This semaphore is signaled in the ISR context.*/
    if (UWBSTATUS_SUCCESS != phOsalUwb_CreateSemaphore(&mIrqWaitSem, 0)) {
        NXPLOG_UWB_TML_D("Error: UWB_HeliosInit(), could not create semaphore mWaitIrqSem\n");
        return UWBSTATUS_FAILED;
    }

    if (UWBSTATUS_SUCCESS != phOsalUwb_CreateSemaphore(&mReadyIrqWaitSem, 0)) {
        NXPLOG_UWB_TML_D("Error: UWB_HeliosInit(), could not create semaphore mReadyIrqWaitSem\n");
        return UWBSTATUS_FAILED;
    }

    /*This mutex is used to make SPI read and write operations mutually exclusive*/
    if (UWBSTATUS_SUCCESS != phOsalUwb_CreateMutex(&mSyncMutex)) {
        NXPLOG_UWB_TML_D("Error: UWB_HeliosInit(), could not create mutex mSyncMutex\n");
        return UWBSTATUS_FAILED;
    }

    phNxpUwb_SpiInit();
    crc16_xmodem_init();

    return UWBSTATUS_SUCCESS;
}

void phNxpUwb_HeliosDeInit()
{
    if (UWBSTATUS_SUCCESS != phOsalUwb_ProduceSemaphore(mIrqWaitSem)) {
        NXPLOG_UWB_TML_D("Error: UWB_HeliosInit(), could not post semaphore mIrqWaitSem\n");
    }

    phOsalUwb_DeleteSemaphore(&mIrqWaitSem);
    mIrqWaitSem = NULL;

    if (UWBSTATUS_SUCCESS != phOsalUwb_ProduceSemaphore(mReadyIrqWaitSem)) {
        NXPLOG_UWB_TML_D("Error: UWB_HeliosInit(), could not post semaphore mReadyIrqWaitSem\n");
    }

    phOsalUwb_DeleteSemaphore(&mReadyIrqWaitSem);
    mReadyIrqWaitSem = NULL;

    phOsalUwb_DeleteMutex(&mSyncMutex);
    mSyncMutex = NULL;

    phNxpUwb_SpiDeInit();
}

uint16_t phNxpUwb_UciRead(uint8_t *rsp)
{
    int status = RETURNED_FAILURE;
    uint16_t payloadLen;
    uint16_t bytesRead                 = 0;
    uint16_t len                       = UCI_HDR_LEN;
    uint16_t totalBtyesToRead          = 0;
    uint32_t IsExtndLenIndication      = 0;
    uint8_t dummyBuff[UCI_HDR_LEN + 1] = {0};
    uint16_t count                     = 0;
    int timeout                        = 0;

    /* INTn line needs to be LOW before selecting the chip. SR040 will use this line to signal to
     * HostS that data is available to read, after selected by CSn line. Implemented
     * handshaking follows following sheme:
     * 1. INTn is Low        <- Required initial state
     * 2. Set CSn low        <- Select SR040 device
     * 3. Read Response
     * 4. wait till INTn pin goes high
     * 5. Set CSn high       <- SR040 no longer selected*/

    phNxpUwb_BackoffDelayReset(&count);

start:
    phNxpUwb_GpioIrqEnable(heliosIrqCb);
    if (phOsalUwb_ConsumeSemaphore_WithTimeout(mIrqWaitSem, 10) != UWBSTATUS_SUCCESS) {
        ENSURE_BACKOFF_TIMEOUT_OR_RET(count, bytesRead);
        goto start;
    }

    phOsalUwb_LockMutex(mSyncMutex);

    setPinSpiCS_n(PIN_LEVEL_LOW);

    /* INT_n should remain asserted. If not, it could be a spurious
     * interrupt or the device may have gone in to low power mode
     */
    if (PIN_LEVEL_HIGH == phNxpUwb_HeliosInteruptStatus()) {
        setPinSpiCS_n(PIN_LEVEL_HIGH);
        phOsalUwb_UnlockMutex(mSyncMutex);
        ENSURE_BACKOFF_TIMEOUT_OR_CLEANUP(count);
        goto start;
    }

    /* Read UCI header */
    status = phNxpUwb_SpiRead(dummyBuff, UCI_HDR_LEN + 1);
    if (status != RETURNED_FAILURE) {
        if (dummyBuff[0] == 0x00) {
            /* First byte read was 0x00, UCI header starts from second byte */
            phOsalUwb_MemCopy(&rsp[0], &dummyBuff[1], len);
        }
        else if (dummyBuff[0] == 0xFF) {
            /* Read failed. The device may have gone in to low power mode.
             * Return failure from here.
             */
            setPinSpiCS_n(PIN_LEVEL_HIGH);
            goto unlockmutex;
        }
        else {
            phOsalUwb_MemCopy(&rsp[0], &dummyBuff[0], len);
        }
        bytesRead = (uint16_t)(bytesRead + len);
    }
    else {
        goto unlockmutex;
    }

    if (rsp[0] == 0x00) {
        /* Empty packet. Backoff and retry read */
        phOsalUwb_UnlockMutex(mSyncMutex);
        ENSURE_BACKOFF_TIMEOUT_OR_CLEANUP(count);
        goto start;
    }

    IsExtndLenIndication = (rsp[EXTND_LEN_INDICATOR_OFFSET] & EXTND_LEN_INDICATOR_OFFSET_MASK);

    totalBtyesToRead = rsp[NORMAL_MODE_LEN_OFFSET];

    if (IsExtndLenIndication) {
        totalBtyesToRead = (uint16_t)((totalBtyesToRead << 8) | rsp[EXTENDED_LENGTH_OFFSET]);
    }

    payloadLen = totalBtyesToRead;

    /* Read payload */
    if (payloadLen != 0) {
        status = phNxpUwb_SpiRead(&rsp[len], payloadLen);
        if (status != RETURNED_FAILURE) {
            bytesRead = (uint16_t)(bytesRead + payloadLen);
        }
    }

    /* INT pin must go HIGH after data is read out */
    phNxpUwb_BackoffDelayReset(&count);
    do {
        /*wait till INTn pin goes High*/
        ENSURE_BACKOFF_TIMEOUT_OR_UNLOCKMUTEX(count);
    } while ((PIN_LEVEL_LOW == phNxpUwb_HeliosInteruptStatus()));

    setPinSpiCS_n(PIN_LEVEL_HIGH);

unlockmutex:
    phOsalUwb_UnlockMutex(mSyncMutex);

cleanup:
    return bytesRead;
}

int phNxpUwb_UciWrite(uint8_t *data, uint16_t len)
{
    int timeout    = 0;
    int bytesWrote = -1;
    int status     = RETURNED_FAILURE;
    uint16_t count = 0;

    if (data == NULL || len == 0) {
        goto cleanup;
    }
    phNxpUwb_BackoffDelayReset(&count);

start:
    phOsalUwb_LockMutex(mSyncMutex);

    /* RDYn line needs to be high before selecting the chip. SR040 will use this line to signal to
     * HostS that it is ready to receive new command, after selected by CSn line. Implemented
     * handshaking follows following sheme:
     * 1. RDYn is high       <- Required initial state
     * 2. Set CSn low        <- Select SR040 device
     * 3. Wait RDYn gets low <- SR040 ready to receive
     * 4. Send command
     * 5. Set CSn high       <- SR040 no longer selected
     * 6. wait till RDYn pin goes high */

    /* Check for INT_N, it should not be low before write operation */
    if (PIN_LEVEL_LOW == phNxpUwb_HeliosInteruptStatus()) {
        phOsalUwb_UnlockMutex(mSyncMutex);
        ENSURE_BACKOFF_TIMEOUT_OR_CLEANUP(count);
        count += 15;
        goto start;
    }

    /* Initial condition: RDY pin must be HIGH */
    phNxpUwb_BackoffDelayReset(&count);
    while (PIN_LEVEL_LOW == phNxpUwb_RdyRead()) {
        ENSURE_BACKOFF_TIMEOUT_OR_UNLOCKMUTEX(count);
    }

    setPinSpiCS_n(PIN_LEVEL_LOW);
    phNxpUwb_BackoffDelayReset(&count);

    /* Wait for RDY pin to be asserted */
    phNxpUwb_RdyGpioIrqEnable(readyIrqCb);
    if (phOsalUwb_ConsumeSemaphore_WithTimeout(mReadyIrqWaitSem, 10) != UWBSTATUS_SUCCESS) {
        phOsalUwb_UnlockMutex(mSyncMutex);
        ENSURE_BACKOFF_TIMEOUT_OR_CLEANUP(count);
        goto start;
    }

    /* RDY pin asserted, write data */
    status = phNxpUwb_SpiWrite(data, len);
    if (status == RETURNED_FAILURE) {
        goto unlockmutex;
    }

    /* De-assert CS. RDY pin must go HIGH */
    setPinSpiCS_n(PIN_LEVEL_HIGH);
    phNxpUwb_BackoffDelayReset(&count);
    do {
        /*wait till ready pin goes High*/
        ENSURE_BACKOFF_TIMEOUT_OR_UNLOCKMUTEX(count);
    } while ((PIN_LEVEL_LOW == phNxpUwb_RdyRead()));

    bytesWrote = (uint16_t)(len);

unlockmutex:
    phOsalUwb_UnlockMutex(mSyncMutex);

cleanup:
    return bytesWrote;
}

uint16_t phNxpUwb_RciRead(uint8_t *rsp)
{
    bool ret                             = true;
    int status                           = RETURNED_FAILURE;
    uint16_t payloadLen                  = UCI_HDR_LEN;
    uint8_t rci_read_buffer[RCI_PKT_LEN] = {0};
    uint8_t *pbuf                        = &rci_read_buffer[0];
    uint16_t crc;
    uint16_t bytesRead = 0;
    uint8_t count      = 0;

    /* INTn line needs to be LOW before selecting the chip. SR040 will use this line to signal to
     * HostS that data is available to read, after selected by CSn line. Implemented
     * handshaking follows following sheme:
     * 1. INTn is Low & RDYn is high       <- Required initial state
     * 2. Set CSn low        <- Select SR040 device
     * 3. INTn goes High
     * 4. Wait RDYn gets low <- SR040 ready to send response
     * 5. Read Response
     * 6. Set CSn high       <- SR040 no longer selected, but RDYn can still be low
     * 7. RDYn gets high */

    phOsalUwb_LockMutex(mSyncMutex);
    do {
        /*INT_N pin to be low before starting read operation*/
        count++;
        vTaskDelay(pdMS_TO_TICKS(1));
    } while ((PIN_LEVEL_LOW != (ret = phNxpUwb_HeliosInteruptStatus())) && (count < TIMEOUT_DELAY));

    if (PIN_LEVEL_LOW != ret) {
        goto end;
    }

    setPinSpiCS_n(PIN_LEVEL_LOW);

    status = phNxpUwb_SpiRead(rci_read_buffer, sizeof(rci_read_buffer));

    setPinSpiCS_n(PIN_LEVEL_HIGH);
    vTaskDelay(pdMS_TO_TICKS(TIME_WAIT_AFTER_CS_READ));

    if (status != RETURNED_FAILURE) {
        if (rci_read_buffer[0] == 0x00) {
            pbuf++;
            payloadLen = payloadLen + rci_read_buffer[4];
        }
        else {
            payloadLen = payloadLen + rci_read_buffer[3];
        }
    }
    else {
        goto end;
    }

    while (PIN_LEVEL_HIGH != phNxpUwb_RdyRead()) {
        /*wait for ready pin goes high*/
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    crc = crc16_xmodem(rci_read_buffer, RCI_PKT_LEN - 2);
    if (((crc & 0xFF) != rci_read_buffer[RCI_PKT_LEN - 2]) ||
        (((crc >> 8) & 0xFF) != rci_read_buffer[RCI_PKT_LEN - 1])) {
        status = RETURNED_FAILURE;
        goto end;
    }

    bytesRead = payloadLen;
    phOsalUwb_MemCopy(rsp, pbuf, payloadLen);

end:

    phOsalUwb_UnlockMutex(mSyncMutex);

    return bytesRead;
}

int phNxpUwb_RciWrite(uint8_t *data, uint16_t len)
{
    uint8_t status                        = RETURNED_FAILURE;
    uint8_t rci_write_buffer[RCI_PKT_LEN] = {0};
    uint16_t crc                          = 0x0000;
    uint8_t count                         = 0;
    bool ret                              = true;

    phOsalUwb_LockMutex(mSyncMutex);
    if (data == NULL || len == 0) {
        goto end;
    }
    phOsalUwb_MemCopy(rci_write_buffer, data, len);

    crc                               = crc16_xmodem(rci_write_buffer, RCI_PKT_LEN - 2);
    rci_write_buffer[RCI_PKT_LEN - 2] = ((crc >> (8 * 0)) & 0xFF);
    rci_write_buffer[RCI_PKT_LEN - 1] = ((crc >> (8 * 1)) & 0xFF);

    /*RDYn line needs to be high before selecting the chip. SR040 will use this line to signal to
     * HostS that it is ready to receive new command, after selected by CSn line. Implemented
     * handshaking follows following sheme:
     * 1. RDYn is high       <- Required initial state
     * 2. Set CSn low        <- Select SR040 device
     * 3. Wait RDYn gets low <- SR040 ready to receive
     * 4. Send command
     * 5. Set CSn high       <- SR040 no longer selected, but RDYn can still be low
     * 6. RDYn gets high*/

    while (PIN_LEVEL_HIGH != phNxpUwb_RdyRead()) {
        /*check for ready pin is high */
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    setPinSpiCS_n(PIN_LEVEL_LOW);

    do {
        /*wait for ready pin to go low*/
        ret = phNxpUwb_RdyRead();
        count++;
        vTaskDelay(pdMS_TO_TICKS(1));

    } while ((PIN_LEVEL_LOW != ret) && (count < TIME_WAIT_AFTER_OPERATION));

    if (PIN_LEVEL_LOW != ret) {
        /*ready pin not low will result in transmission error*/
        goto end;
    }

    status = phNxpUwb_SpiWrite(rci_write_buffer, RCI_PKT_LEN);

    setPinSpiCS_n(PIN_LEVEL_HIGH);
    vTaskDelay(pdMS_TO_TICKS(TIME_WAIT_AFTER_CS_WRITE));

    while (PIN_LEVEL_HIGH != phNxpUwb_RdyRead()) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    if (status == RETURNED_FAILURE) {
        goto end;
    }

end:

    phOsalUwb_UnlockMutex(mSyncMutex);
    return status;
}

void phNxpUwb_HeliosIrqEnable(void)
{
    phNxpUwb_GpioIrqEnable(heliosIrqCb);
}

bool phNxpUwb_HeliosReset(void)
{
    bool ret          = true;
    uint8_t resp[5]   = {0};
    uint16_t resp_len = 0;
    uint8_t count     = 0;

    phNxpUwb_sr040Reset();

    do {
        /*wait till IRQ pin goes High*/
        count++;
        vTaskDelay(pdMS_TO_TICKS(1));
    } while ((PIN_LEVEL_LOW != (ret = phNxpUwb_HeliosInteruptStatus())) && (count < TIMEOUT_DELAY));

    if (PIN_LEVEL_LOW != ret) {
        goto end;
    }

    resp_len = phNxpUwb_UciRead(resp);
    if (resp_len == 0x05 && (*resp == 0x60) && (*(resp + 1) == 0x01) && (*(resp + 4) == 0x01)) {
        /* Device is ready */
        ret = true;
    }
    else {
        ret = false;
    }

end:
    return ret;
}
