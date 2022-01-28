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

#include "phUwb_BuildConfig.h"

#if 1 //( UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_PLUG_AND_PLAY_MODE )
/* System includes */
#include <string.h>
#include <stdint.h>
#include "UWB_GpioIrq.h"
#include "UWB_Spi_Driver_Interface.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Freescale includes */
#include "fsl_debug_console.h"
#include "fsl_port.h"
#include "fsl_lpspi.h"
#include "fsl_gpio.h"

/* UWB includes */
#include "driver_config.h"
#include "UwbCore_Types.h"

#define PAYLOAD_LEN_MSB 2
#define PAYLOAD_LEN_LSB 3
#define UCI_HDR_LEN     4

#define HELIOS_SPI_ASYNC    0
#define HELIOS_SPI_BLOCKING 1
#define HELIOS_SPI_METHOD   HELIOS_SPI_BLOCKING

#define NORMAL_MODE_HEADER_LEN 4
#define HBCI_MODE_HEADER_LEN   4
#define NORMAL_MODE_LEN_OFFSET 3
#define UCI_NORMAL_PKT_SIZE    0

#define EXTND_LEN_INDICATOR_OFFSET      1
#define EXTND_LEN_INDICATOR_OFFSET_MASK 0x80
#define EXTENDED_LENGTH_OFFSET          2

static QueueHandle_t mIrqWaitSem;
QueueHandle_t mReadTaskSyncMutex;
status_t UWB_SpiUciTransferSpeedupWrapper(LPSPI_Type *base, lpspi_transfer_t *transfer);

#if UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V2
#define HELIOS_SYNC                 UWB_HELIOS_SYNC_GPIO, UWB_HELIOS_SYNC_PIN
#define HELIOS_CE                   UWB_HELIOS_CE_GPIO, UWB_HELIOS_CE_PIN
#define HELIOS_INT                  UWB_HELIOS_IRQ_GPIO, UWB_HELIOS_IRQ_PIN
#define PIN_INIT(GPIO)              GPIO_PinInit(gpio, pin, conf)
#define PIN_WRITE(gpio, pin, value) GPIO_PinWrite(gpio, pin, value)
#define PIN_READ(gpio, pin)         GPIO_PinRead(gpio, pin)
#define ENABLE_HELIOS_IRQ                             \
    UWB_GpioIrqConf(UWB_HELIOS_IRQ_GPIO,              \
        UWB_HELIOS_IRQ_PORT,                          \
        UWB_HELIOS_IRQ_PIN,                           \
        kPORT_InterruptLogicOne,                      \
        configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, \
        heliosIrqCb);
#define DISABLE_HELIOS_IRQ \
    UWB_GpioIrqConf(       \
        UWB_HELIOS_IRQ_GPIO, UWB_HELIOS_IRQ_PORT, UWB_HELIOS_IRQ_PIN, kPORT_InterruptOrDMADisabled, 0, heliosIrqCb);
#elif UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V3
#define HELIOS_SYNC                 HELIOS_SYNC
#define HELIOS_CE                   HELIOS_ENABLE
#define HELIOS_RTC                  HELIOS_RTC_SYNC
#define HELIOS_INT                  HELIOS_IRQ
#define PIN_WRITE(component, value) UWB_GpioSet(component, value)
#define PIN_READ(component)         UWB_GpioRead(component)
#define ENABLE_HELIOS_IRQ()         UWB_GpioIrqEnable(HELIOS_IRQ, heliosIrqCb, NULL)
#define DISABLE_HELIOS_IRQ()        UWB_GpioIrqDisable(HELIOS_IRQ)
#endif

#if UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V2
static void heliosIrqCb(void)
{
#elif UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V3
static void heliosIrqCb(void *args)
{
#endif
    BaseType_t yield;
    // Disable interrupt
    DISABLE_HELIOS_IRQ();

    // Signal TML read task
    xSemaphoreGiveFromISR(mIrqWaitSem, &yield);
}

/* below function is based on the CPU speed of 48MHz,
   6 iteration of while loop takes 1us*/
void AddDelayInMicroSec(int delay)
{
    volatile int cnt = 6 * delay;
    while (cnt--)
        ;
}

uint16_t UWB_SpiUciRead(uint8_t *rsp)
{
    spi_transfer_t xfer;
    status_t status;
    uint16_t payloadLen;
    uint16_t bytesRead            = 0;
    uint16_t len                  = UCI_HDR_LEN;
    uint16_t totalBtyesToRead     = 0;
    uint32_t IsExtndLenIndication = 0;
    uint8_t count                 = 0;
start:

    ENABLE_HELIOS_IRQ();
    if (xSemaphoreTake(mIrqWaitSem, portMAX_DELAY) != pdTRUE) {
        goto end;
    }

    xSemaphoreTake(mReadTaskSyncMutex, portMAX_DELAY);
    AddDelayInMicroSec(1);
    if (!UWB_GpioRead(HELIOS_INT)) {
        xSemaphoreGive(mReadTaskSyncMutex);
        goto start;
    }

    /*Read Ready Indication from HOST to Helios*/
    PIN_WRITE(HELIOS_SYNC, true);
    while (UWB_GpioRead(HELIOS_INT)) {
        if (count == 100) {
            break;
        }
        AddDelayInMicroSec(10);
        count++;
    }
    ENABLE_HELIOS_IRQ();

    if (xSemaphoreTake(mIrqWaitSem, 50) != pdTRUE) {
        PRINTF("xSemaphoreTake timed out for second IRQ\n");
        goto end;
    }
    else
        PRINTF("second IRQ Received\n");

    count = 0;
    while (!UWB_GpioRead(HELIOS_INT)) {
        if (count == 10) {
            PIN_WRITE(HELIOS_SYNC, false);
            xSemaphoreGive(mReadTaskSyncMutex);
            return bytesRead;
        }
        USLEEP(1000);
        count++;
    }

    xfer.txData      = NULL;
    xfer.rxData      = rsp;
    xfer.dataSize    = len;
    xfer.configFlags = UWB_SPI_SLAVE_PCS | kLPSPI_MasterPcsContinuous;
    status           = UWB_SpiUciTransferSpeedupWrapper(UWB_SPI_BASEADDR, &xfer);

    if (status != kStatus_Success) {
        PRINTF("UWB_SpiUciXfer(): header rx error %04x\n", status);
        goto end;
    }

    bytesRead += len;
    IsExtndLenIndication = (rsp[EXTND_LEN_INDICATOR_OFFSET] & EXTND_LEN_INDICATOR_OFFSET_MASK);
    totalBtyesToRead     = rsp[NORMAL_MODE_LEN_OFFSET];
    if (IsExtndLenIndication) {
        totalBtyesToRead = ((totalBtyesToRead << 8) | rsp[EXTENDED_LENGTH_OFFSET]);
    }
    payloadLen = totalBtyesToRead;

    if (payloadLen != 0) {
        xfer.txData      = NULL;
        xfer.rxData      = &rsp[len];
        xfer.dataSize    = payloadLen;
        xfer.configFlags = UWB_SPI_SLAVE_PCS | kLPSPI_MasterPcsContinuous;

        status = UWB_SpiUciTransferSpeedupWrapper(UWB_SPI_BASEADDR, &xfer);
        if (status == kStatus_Success) {
            bytesRead += payloadLen;
        }
    }

end:
    count = 0;
    while (UWB_GpioRead(HELIOS_INT)) {
        if (count >= 20) { // Sleep of 500us * 20 = 10ms as per artf786394
            break;
        }
        AddDelayInMicroSec(500);
        count++;
    }

    PIN_WRITE(HELIOS_SYNC, false);

    xSemaphoreGive(mReadTaskSyncMutex);

    return bytesRead;
}

uint8_t UWB_SpiHbciXfer(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rspLen)
{
    spi_transfer_t xfer;
    status_t status;
    uint16_t payloadLen;
    uint8_t ok = true;

    if (data != NULL && len > 0) {
        *rspLen = 0;

        xfer.txData = data;

        xfer.rxData      = NULL;
        xfer.dataSize    = len;
        xfer.configFlags = UWB_SPI_SLAVE_PCS | kSPI_MasterPcsContinuous;

        //This while was added to avoid a bug happening with KL28 in which it will fail in sending a command but continue with the FW download like nothing happened
        while ((LPSPI_GetStatusFlags(UWB_SPI_BASEADDR) & kLPSPI_ModuleBusyFlag))
            ;
        status = UWB_SpiUciTransferSpeedupWrapper(UWB_SPI_BASEADDR, &xfer);
        if (status != kStatus_Success) {
            PRINTF("UWB_SpiHbciXfer(): tx error %04x\n", status);
            ok = false;
            goto end;
        }

        ENABLE_HELIOS_IRQ();

        if (xSemaphoreTake(mIrqWaitSem, portMAX_DELAY) != pdTRUE) {
            return 0;
        }
    }

    xfer.txData      = NULL;
    xfer.rxData      = rsp;
    xfer.dataSize    = HBCI_HEADER_SIZE;
    xfer.configFlags = UWB_SPI_SLAVE_PCS | kSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

    status = UWB_SpiUciTransferSpeedupWrapper(UWB_SPI_BASEADDR, &xfer);

    if (status != kStatus_Success) {
        PRINTF("UWB_SpiHbciXfer(): header rx error %04x\n", status);
        ok = false;
        goto end;
    }

    payloadLen = (rsp[PAYLOAD_LEN_MSB] << 8) | rsp[PAYLOAD_LEN_LSB];
    if (payloadLen != 0) {
        xfer.txData      = NULL;
        xfer.rxData      = &rsp[HBCI_HEADER_SIZE];
        xfer.dataSize    = payloadLen;
        xfer.configFlags = UWB_SPI_SLAVE_PCS | kSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;
        status           = UWB_SpiUciTransferSpeedupWrapper(UWB_SPI_BASEADDR, &xfer);

        if (status != kStatus_Success) {
            PRINTF("UWB_SpiHbciXfer(): payload rx error %04x\n", status);
            ok = false;
            goto end;
        }
    }
    *rspLen = HBCI_HEADER_SIZE + payloadLen;
end:
    return ok;
}

uint8_t UWB_SpiHbciXferWithLen(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t rspLen)
{
    spi_transfer_t xfer;
    status_t status;
    uint8_t ok = true;

    if (data != NULL && len > 0) {
        xfer.txData = data;

        xfer.rxData      = NULL;
        xfer.dataSize    = len;
        xfer.configFlags = UWB_SPI_SLAVE_PCS | kSPI_MasterPcsContinuous;

        //This while was added to avoid a bug happening with KL28 in which it will fail in sending a command but continue with the FW download like nothing happened
        while ((LPSPI_GetStatusFlags(UWB_SPI_BASEADDR) & kLPSPI_ModuleBusyFlag))
            ;
        status = UWB_SpiUciTransferSpeedupWrapper(UWB_SPI_BASEADDR, &xfer);
        if (status != kStatus_Success) {
            PRINTF("UWB_SpiHbciXfer(): tx error %04x\n", status);
            ok = false;
            goto end;
        }

        ENABLE_HELIOS_IRQ();

        if (xSemaphoreTake(mIrqWaitSem, portMAX_DELAY) != pdTRUE) {
            return 0;
        }
    }

    xfer.txData      = NULL;
    xfer.rxData      = rsp;
    xfer.dataSize    = rspLen;
    xfer.configFlags = UWB_SPI_SLAVE_PCS | kSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

    status = UWB_SpiUciTransferSpeedupWrapper(UWB_SPI_BASEADDR, &xfer);

    if (status != kStatus_Success) {
        PRINTF("UWB_SpiHbciXfer(): header rx error %04x\n", status);
        ok = false;
        goto end;
    }
#if (ENABLE_HBCI_CMD_LOGGING == ENABLED)
    PRINTF("HBCI rsp:");
    for (int i = 0; i < rspLen; i++) {
        PRINTF(" %02x", rsp[i]);
    }
    PRINTF("\n");
#endif
end:
    return ok;
}

uint8_t UWB_HeliosSpiInit(void)
{
    spi_master_config_t masterConfig;

    /*This semaphore is signaled in the ISR context.*/
    mIrqWaitSem = xQueueCreateCountingSemaphore(1, 0);
    if (!mIrqWaitSem) {
        PRINTF("Error: UWB_HeliosInit(), could not create semaphore mWaitIrqSem\n");
        return false;
    }

    /*This mutex is used to make SPI read and write operations mutually exclusive*/
    mReadTaskSyncMutex = xSemaphoreCreateMutex();
    if (!mReadTaskSyncMutex) {
        PRINTF("Error: UWB_HeliosInit(), could not create mutex mReadTaskSyncMutex\n");
        return false;
    }

#if UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V2

    /* Init CE */
    gpio_pin_config_t output_config = {
        kGPIO_DigitalOutput,
        0,
    };
    PIN_INIT(HELIOS_CE, &output_config);
    PIN_INIT(HELIOS_SYNC, &output_config);

    /* Helios VDD Cfg */
    gpio_pin_config_t heliosVddCfg = {
        kGPIO_DigitalOutput,
        1,
    };

    GPIO_PinInit(UWB_HELIOS_VDD_DIG_EN_GPIO, UWB_HELIOS_VDD_DIG_EN_PIN, &heliosVddCfg);
    PIN_WRITE(UWB_HELIOS_VDD_DIG_EN_GPIO, UWB_HELIOS_VDD_DIG_EN_PIN, 1);

    GPIO_PinInit(UWB_HELIOS_VDD_DIG_RF_GPIO, UWB_HELIOS_VDD_DIG_RF_PIN, &heliosVddCfg);
    PIN_WRITE(UWB_HELIOS_VDD_DIG_RF_GPIO, UWB_HELIOS_VDD_DIG_RF_PIN, 1);
#endif

#if UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V2
    /* Init Helios IRQ */
    gpio_pin_config_t heliosIrq = {kGPIO_DigitalInput, 0};
    GPIO_PinInit(UWB_HELIOS_IRQ_GPIO, UWB_HELIOS_IRQ_PIN, &heliosIrq);
#endif

    PIN_WRITE(HELIOS_CE, 0);
    PIN_WRITE(HELIOS_RTC, 0);
    PIN_WRITE(HELIOS_SYNC, 0);

    CLOCK_SetIpSrc(UWB_SPI_CLOCK_NAME, UWB_SPI_CLOCK_SRC);
    masterConfig.baudRate     = UWB_SPI_BAUDRATE;
    masterConfig.bitsPerFrame = 8 * 4;
    masterConfig.cpol         = kLPSPI_ClockPolarityActiveHigh;
    masterConfig.cpha         = kLPSPI_ClockPhaseFirstEdge;
    masterConfig.direction    = kLPSPI_MsbFirst;

    masterConfig.pcsToSckDelayInNanoSec        = 0; //100000000000U / UWB_SPI_BAUDRATE;
    masterConfig.lastSckToPcsDelayInNanoSec    = 0; //100000000000U / UWB_SPI_BAUDRATE;
    masterConfig.betweenTransferDelayInNanoSec = 0; //100000000000U / UWB_SPI_BAUDRATE;

    masterConfig.whichPcs           = UWB_SPI_MASTER_CFG_PCS;
    masterConfig.pcsActiveHighOrLow = kLPSPI_PcsActiveLow;

    // masterConfig.pinCfg = kLPSPI_SdoInSdiOut;
    masterConfig.pinCfg        = kLPSPI_SdiInSdoOut;
    masterConfig.dataOutConfig = kLpspiDataOutRetained;
    SPI_MasterInit(UWB_SPI_BASEADDR, &masterConfig, UWB_SPI_COCK_FREQ);

    return true;
}

uint16_t UWB_SpiUciWrite(uint8_t *data, uint16_t len)
{
    spi_transfer_t xfer;
    status_t status;
    uint16_t bytesWrote = 0;

    xSemaphoreTake(mReadTaskSyncMutex, portMAX_DELAY);
    if (data == NULL || len == 0) {
        goto end;
    }

    /* Send header */
    xfer.txData      = data;
    xfer.rxData      = NULL;
    xfer.dataSize    = UCI_HDR_LEN;
    xfer.configFlags = UWB_SPI_SLAVE_PCS | kSPI_MasterPcsContinuous;

    status = UWB_SpiUciTransferSpeedupWrapper(UWB_SPI_BASEADDR, &xfer);
    if (status != kStatus_Success) {
        PRINTF("UWB_SpiUciXfer(): could not send UCI header %04x\n", status);
        goto end;
    }
    AddDelayInMicroSec(50);
    bytesWrote += UCI_HDR_LEN;

    /* Send payload */
    if (len > UCI_HDR_LEN) {
        xfer.txData      = &data[UCI_HDR_LEN];
        xfer.rxData      = NULL;
        xfer.dataSize    = len - UCI_HDR_LEN;
        xfer.configFlags = UWB_SPI_SLAVE_PCS | kSPI_MasterPcsContinuous;

        status = UWB_SpiUciTransferSpeedupWrapper(UWB_SPI_BASEADDR, &xfer);

        if (status == kStatus_Success) {
            bytesWrote = len;
        }
    }

end:
    xSemaphoreGive(mReadTaskSyncMutex);
    return bytesWrote;
}

void UWB_HeliosIrqEnable(void)
{
    ENABLE_HELIOS_IRQ();
}

void UWB_HeliosSpiDeInit(void)
{
    xSemaphoreGive(mIrqWaitSem);
    vSemaphoreDelete(mIrqWaitSem);
    vSemaphoreDelete(mReadTaskSyncMutex);
}

void hbci_GPIOwait_ready(void)
{
    while (!PIN_READ(HELIOS_IRQ))
        ;
}

status_t UWB_SpiUciTransferSpeedupWrapper(LPSPI_Type *base, lpspi_transfer_t *transfer)
{
    transfer->configFlags |= kLPSPI_MasterByteSwap;
    LPSPI_SetFrameSize(base, 4 * 8);

    GPIO_WritePinOutput(UWB_HELIOS_CS_GPIO, UWB_HELIOS_CS_PIN, 0);

#ifdef SINGLE_BYTE_SPI_DRIVER

    LPSPI_SetFrameSize(base, 8);
    SPI_MasterTransferBlocking(base, transfer);
    GPIO_WritePinOutput(UWB_HELIOS_CS_GPIO, UWB_HELIOS_CS_PIN, 1);
    return 0;
#endif
#ifndef SINGLE_BYTE_SPI_DRIVER
    status_t status = 0;

    if (transfer->dataSize % 4 != 0) {
        lpspi_transfer_t old_transfer;
        old_transfer.dataSize = transfer->dataSize;
        old_transfer.rxData   = transfer->rxData;
        old_transfer.txData   = transfer->txData;
        uint32_t delta        = transfer->dataSize % 4;

        if (transfer->dataSize > 4) {
            transfer->dataSize -= delta;
            status = SPI_MasterTransferBlocking(base, transfer);
        }
        LPSPI_SetFrameSize(base, 8 * delta);

        if (transfer->dataSize >= 4) {
            if (transfer->rxData != 0) {
                transfer->rxData += transfer->dataSize;
            }
            transfer->txData += transfer->dataSize;
            transfer->dataSize = delta;
        }
        status |= SPI_MasterTransferBlocking(base, transfer);
        transfer->dataSize = old_transfer.dataSize;
        transfer->rxData   = old_transfer.rxData;
        transfer->txData   = old_transfer.txData;
    }
    else {
        status |= SPI_MasterTransferBlocking(base, transfer);
    }

    GPIO_WritePinOutput(UWB_HELIOS_CS_GPIO, UWB_HELIOS_CS_PIN, 1);
    return status;
#endif
}
#endif
