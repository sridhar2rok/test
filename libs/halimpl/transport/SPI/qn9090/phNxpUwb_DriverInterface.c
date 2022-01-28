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

/* System includes */
#include <string.h>
#include <stdint.h>
#include "UWB_GpioIrq.h"
#include "phNxpUwb_DriverInterface.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Freescale includes */
#include "fsl_debug_console.h"
#include "fsl_spi.h"
#include "fsl_gpio.h"

/* UWB includes */
#include "driver_config.h"
#include "UwbCore_Types.h"
#include "phUwbErrorCodes.h"

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
QueueHandle_t mSyncMutex;

#define HELIOS_INT  HELIOS_IRQ
#define SR040_RESET RST_N
#define SR040_RDY   RDY_N_IRQ

#define PIN_WRITE(component, value) UWB_GpioSet(component, value)
#define PIN_READ(component)         UWB_GpioRead(component)
#define ENABLE_HELIOS_IRQ(cb)       UWB_GpioIrqEnable(HELIOS_IRQ, cb, NULL)
#define DISABLE_HELIOS_IRQ()        UWB_GpioIrqDisable(HELIOS_IRQ)
#define ENABLE_RDY_N_IRQ(cb)        UWB_GpioIrqEnable(RDY_N_IRQ, cb, NULL)
#define DISABLE_RDY_N_IRQ()         UWB_GpioIrqDisable(RDY_N_IRQ)

static volatile bool uciReadSuspend = false;

void heliosIrqCb(void *args)
{
    BaseType_t yield;
    // Disable interrupt
    DISABLE_HELIOS_IRQ();

    // Signal TML read task
    //phOsalUwb_ProduceSemaphore(mIrqWaitSem);
    xSemaphoreGiveFromISR(mIrqWaitSem, &yield);
}

int phNxpUwb_SpiInit(void)
{
    spi_master_config_t masterConfig;
    // uint32_t sourceClock;

    CLOCK_AttachClk(kOSC32M_to_SPI_CLK); /*!< Switch SPI_CLK to OSC32M */

    /*This semaphore is signaled in the ISR context.*/
    mIrqWaitSem = xQueueCreateCountingSemaphore(1, 0);
    if (!mIrqWaitSem) {
        PRINTF("Error: UWB_HeliosInit(), could not create semaphore mWaitIrqSem\n");
        return false;
    }

    /*This mutex is used to make SPI read and write operations mutually exclusive*/
    mSyncMutex = xSemaphoreCreateMutex();
    if (!mSyncMutex) {
        PRINTF("Error: UWB_HeliosInit(), could not create mutex mSyncMutex\n");
        return false;
    }

    /*CPOL =1, CPHA = 0*/

    masterConfig.baudRate_Bps = UWB_SPI_BAUDRATE;
    masterConfig.dataWidth    = kSPI_Data8Bits;
    masterConfig.polarity     = kSPI_ClockPolarityActiveLow;
    masterConfig.phase        = kSPI_ClockPhaseFirstEdge;
    masterConfig.direction    = kSPI_MsbFirst;

    masterConfig.delayConfig.preDelay      = 15U;
    masterConfig.delayConfig.postDelay     = 15U;
    masterConfig.delayConfig.frameDelay    = 15U;
    masterConfig.delayConfig.transferDelay = 15U;

    masterConfig.sselPol = kSPI_SpolActiveAllLow;
    masterConfig.sselNum = UWB_SPI_SSEL;

    masterConfig.txWatermark = kSPI_TxFifo0;
    masterConfig.rxWatermark = kSPI_RxFifo1;

    masterConfig.enableLoopback = false;
    masterConfig.enableMaster   = true;

    // SPI_MasterGetDefaultConfig(&masterConfig);

    // masterConfig.pinCfg = kLPSPI_SdoInSdiOut;
    SPI_MasterInit(UWB_SPI_BASEADDR, &masterConfig, CLOCK_GetFreq(kCLOCK_Fro32M));

    return true;
}

int phNxpUwb_SpiWrite(uint8_t *data, uint16_t len)
{
    spi_transfer_t xfer;
    status_t status;
    uint8_t ret = RETURNED_FAILURE;

    if (data == NULL || len == 0) {
        goto end;
    }

    xSemaphoreTake(mSyncMutex, portMAX_DELAY);

    /* Send header */
    xfer.txData      = data;
    xfer.rxData      = NULL;
    xfer.dataSize    = len;
    xfer.configFlags = kSPI_FrameAssert;

    status = SPI_MasterTransferBlocking(UWB_SPI_BASEADDR, &xfer);
    if (status == kStatus_Success) {
        ret = RETURNED_SUCCESS;
    }
    xSemaphoreGive(mSyncMutex);

end:
    return ret;
}

int phNxpUwb_SpiRead(uint8_t *data, uint16_t len)
{
    spi_transfer_t xfer;
    status_t status;
    int ret = RETURNED_FAILURE;

    if (data == NULL || len == 0) {
        goto end;
    }

    xSemaphoreTake(mSyncMutex, portMAX_DELAY);

    /* Send header */
    xfer.txData      = NULL;
    xfer.rxData      = data;
    xfer.dataSize    = len;
    xfer.configFlags = kSPI_FrameAssert;

    status = SPI_MasterTransferBlocking(UWB_SPI_BASEADDR, &xfer);
    if (status == kStatus_Success) {
        ret = RETURNED_SUCCESS;
    }
    xSemaphoreGive(mSyncMutex);

end:
    return ret;
}

int phNxpUwb_SpiDeInit(void)
{
    xSemaphoreGive(mIrqWaitSem);
    vSemaphoreDelete(mIrqWaitSem);
    vSemaphoreDelete(mSyncMutex);

    return true;
}
void phNxpUwb_GpioIrqEnable(void (*cb)(void *args))
{
    ENABLE_HELIOS_IRQ(cb);
}
void phNxpUwb_RdyGpioIrqEnable(void (*cb)(void *args))
{
    ENABLE_RDY_N_IRQ(cb);
}

void phNxpUwb_RdyGpioIrqDisable(void (*cb)(void *args))
{
    DISABLE_RDY_N_IRQ();
}

void phNxpUwb_GpioIrqDisable(void (*cb)(void *args))
{
    DISABLE_HELIOS_IRQ();
}

bool phNxpUwb_HeliosInteruptStatus()
{
    /*In SR040 INT_N pin goes low when data is available*/
    return PIN_READ(HELIOS_INT);
}

void phNxpUwb_sr040Reset()
{
    //   UWB_GpioSet(RST_N, 1);
    //   vTaskDelay(pdMS_TO_TICKS(100));
    UWB_GpioSet(RST_N, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    UWB_GpioSet(RST_N, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
}

bool phNxpUwb_RdyRead()
{
    return PIN_READ(SR040_RDY);
}
