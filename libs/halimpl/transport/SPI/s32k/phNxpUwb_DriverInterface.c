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

/* S32K  includes */
#include "fsl_debug_console.h"
#include <lpspi_master_driver.h>
#include <pin_control.h>
#include <lpspiCom1.h>

/* UWB includes */
#include "driver_config.h"
#include "UwbCore_Types.h"
#include "phUwbErrorCodes.h"

/** SPI communication port to use. */
#define SPI_COMMUNICATION_PORT 0

#define FRAME_TRANSMISSION_ERROR       1
#define FRAME_TRANSMISSION_SUCCESSFULL 0

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

lpspi_state_t masterState;

#define HELIOS_INT  HELIOS_IRQ
#define SR040_RESET RST_N
#define SR040_RDY   RDY_N

#define PIN_WRITE(component, value) UWB_GpioSet(component, value)
#define PIN_READ(component)         UWB_GpioRead(component)
#define ENABLE_HELIOS_IRQ(cb)       UWB_GpioIrqEnable(HELIOS_IRQ, cb, NULL)
#define DISABLE_HELIOS_IRQ()        UWB_GpioIrqDisable(HELIOS_IRQ)
#define ENABLE_RDY_N_IRQ(cb)        UWB_GpioIrqEnable(RDY_N_IRQ, cb, NULL)
#define DISABLE_RDY_N_IRQ()         UWB_GpioIrqDisable(RDY_N_IRQ)

static volatile bool uciReadSuspend = false;

int phNxpUwb_SpiInit(void)
{
    lpspi_master_config_t LpspiConfig = {
        .bitsPerSec      = UWB_SPI_BAUDRATE,
        .whichPcs        = LPSPI_PCS0,
        .pcsPolarity     = LPSPI_ACTIVE_LOW,
        .isPcsContinuous = false,
        .bitcount        = 8U,
        .lpspiSrcClk     = 48000000U,
        .clkPhase        = LPSPI_CLOCK_PHASE_1ST_EDGE,
        .clkPolarity     = LPSPI_SCK_ACTIVE_LOW,
        .lsbFirst        = false,
        .transferType    = LPSPI_USING_DMA,
        .rxDMAChannel    = 0U,
        .txDMAChannel    = 1U,
        .callback        = NULL,
        .callbackParam   = NULL,
    };

    /* Initialize LPSPI instance */
    LPSPI_DRV_MasterInit(SPI_COMMUNICATION_PORT, &masterState, &LpspiConfig);

    return true;
}

int phNxpUwb_SpiWrite(uint8_t *data, uint16_t len)
{
    uint32_t bytesRemained;

    if (data == NULL || len == 0) {
        return RETURNED_FAILURE;
    }

    LPSPI_DRV_MasterTransfer(SPI_COMMUNICATION_PORT, data, NULL, len);
    while (STATUS_BUSY == LPSPI_DRV_MasterGetTransferStatus(SPI_COMMUNICATION_PORT, &bytesRemained)) {
    }

    return RETURNED_SUCCESS;
}

int phNxpUwb_SpiRead(uint8_t *data, uint16_t len)
{
    uint32_t bytesRemained;
    uint8_t dumy_tx[512] = {0};

    if (data == NULL || len == 0) {
        return RETURNED_FAILURE;
    }

    LPSPI_DRV_MasterTransfer(SPI_COMMUNICATION_PORT, dumy_tx, data, len);
    while (STATUS_BUSY == LPSPI_DRV_MasterGetTransferStatus(SPI_COMMUNICATION_PORT, &bytesRemained)) {
    }

    return RETURNED_SUCCESS;
}

int phNxpUwb_SpiDeInit(void)
{
    LPSPI_DRV_MasterDeinit(SPI_COMMUNICATION_PORT);
    return RETURNED_SUCCESS;
}

void phNxpUwb_GpioIrqEnable(void (*cb)(void *args))
{
    ENABLE_HELIOS_IRQ(cb);
}

void phNxpUwb_GpioIrqDisable(void (*cb)(void *args))
{
    DISABLE_HELIOS_IRQ();
}

void phNxpUwb_RdyGpioIrqEnable(void (*cb)(void *args))
{
    ENABLE_RDY_N_IRQ(cb);
}

void phNxpUwb_RdyGpioIrqDisable(void (*cb)(void *args))
{
    DISABLE_RDY_N_IRQ();
}

bool phNxpUwb_HeliosInteruptStatus()
{
    return getPinSpiIrq_n();
}

bool phNxpUwb_ReadyStatus()
{
    return true;
}

void phNxpUwb_sr040Reset()
{
    /* Activate RSTn line */
    togglePinRanger4Reset();
}

bool phNxpUwb_RdyRead()
{
    return getPinSpiRdy_n();
}
