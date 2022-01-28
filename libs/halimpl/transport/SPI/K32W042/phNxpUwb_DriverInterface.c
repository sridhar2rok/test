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

/* System includes */
#include "UWB_GpioIrq.h"
#include <stdint.h>

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/* Freescale includes */
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_lpspi.h"
#include "fsl_port.h"

/* UWB includes */
#include "driver_config.h"
#include "UwbCore_Types.h"

#include "phNxpUwb_SpiTransport.h"
#include "phOsalUwb.h"
#include "phUwb_BuildConfig.h"
#include "phUwbErrorCodes.h"

#if UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V2
#define HELIOS_SYNC                 UWB_HELIOS_SYNC_GPIO, UWB_HELIOS_SYNC_PIN
#define HELIOS_CE                   UWB_HELIOS_CE_GPIO, UWB_HELIOS_CE_PIN
#define HELIOS_INT                  UWB_HELIOS_IRQ_GPIO, UWB_HELIOS_IRQ_PIN
#define PIN_INIT(GPIO)              GPIO_PinInit(gpio, pin, conf)
#define PIN_WRITE(gpio, pin, value) GPIO_PinWrite(gpio, pin, value)
#define PIN_READ(gpio, pin)         GPIO_PinRead(gpio, pin)
#define ENABLE_HELIOS_IRQ(cb)                         \
    UWB_GpioIrqConf(UWB_HELIOS_IRQ_GPIO,              \
        UWB_HELIOS_IRQ_PORT,                          \
        UWB_HELIOS_IRQ_PIN,                           \
        kPORT_InterruptLogicOne,                      \
        configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, \
        cb);
#define DISABLE_HELIOS_IRQ(cb) \
    UWB_GpioIrqConf(UWB_HELIOS_IRQ_GPIO, UWB_HELIOS_IRQ_PORT, UWB_HELIOS_IRQ_PIN, kPORT_InterruptOrDMADisabled, 0, cb);
#elif UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V3
#define HELIOS_SYNC                 HELIOS_SYNC
#define HELIOS_CE                   HELIOS_ENABLE
#define HELIOS_RTC                  HELIOS_RTC_SYNC
#define HELIOS_INT                  HELIOS_IRQ
#define PIN_WRITE(component, value) UWB_GpioSet(component, value)
#define PIN_READ(component)         UWB_GpioRead(component)
#define ENABLE_HELIOS_IRQ(cb)       UWB_GpioIrqEnable(HELIOS_IRQ, cb, NULL)
#define DISABLE_HELIOS_IRQ()        UWB_GpioIrqDisable(HELIOS_IRQ)
#endif

static status_t UWB_SpiUciTransferSpeedupWrapper(LPSPI_Type *base, lpspi_transfer_t *transfer)
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

int phNxpUwb_SpiWrite(uint8_t *data, uint16_t len)
{
    spi_transfer_t xfer;
    status_t status;
    int ret = RETURNED_FAILURE;

    if (data != NULL && len > 0) {
        xfer.txData      = data;
        xfer.rxData      = NULL;
        xfer.dataSize    = len;
        xfer.configFlags = UWB_SPI_SLAVE_PCS | kSPI_MasterPcsContinuous;

        // This while was added to avoid a bug happening with KL28 in which it will
        // fail in sending a command but continue with the FW download like nothing
        // happened
        while ((LPSPI_GetStatusFlags(UWB_SPI_BASEADDR) & kLPSPI_ModuleBusyFlag))
            ;
        status = UWB_SpiUciTransferSpeedupWrapper(UWB_SPI_BASEADDR, &xfer);
        if (status != kStatus_Success) {
            PRINTF("spiWrite(): tx error %04x\n", status);
            return ret;
        }
    }
    else {
        return ret;
    }
    ret = RETURNED_SUCCESS;
    return ret;
}

int phNxpUwb_SpiRead(uint8_t *data, uint16_t len)
{
    spi_transfer_t xfer;
    status_t status;
    int ret = RETURNED_FAILURE;

    if (data != NULL && len > 0) {
        xfer.txData      = NULL;
        xfer.rxData      = data;
        xfer.dataSize    = len;
        xfer.configFlags = UWB_SPI_SLAVE_PCS | kSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;

        status = UWB_SpiUciTransferSpeedupWrapper(UWB_SPI_BASEADDR, &xfer);
        if (status != kStatus_Success) {
            PRINTF("spiWrite(): tx error %04x\n", status);
            return ret;
        }
    }
    else {
        return ret;
    }
    ret = RETURNED_SUCCESS;
    return ret;
}

int phNxpUwb_SpiInit(void)
{
    spi_master_config_t masterConfig;

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

    masterConfig.pcsToSckDelayInNanoSec        = 0; // 100000000000U / UWB_SPI_BAUDRATE;
    masterConfig.lastSckToPcsDelayInNanoSec    = 0; // 100000000000U / UWB_SPI_BAUDRATE;
    masterConfig.betweenTransferDelayInNanoSec = 0; // 100000000000U / UWB_SPI_BAUDRATE;

    masterConfig.whichPcs           = UWB_SPI_MASTER_CFG_PCS;
    masterConfig.pcsActiveHighOrLow = kLPSPI_PcsActiveLow;

    // masterConfig.pinCfg = kLPSPI_SdoInSdiOut;
    masterConfig.pinCfg        = kLPSPI_SdiInSdoOut;
    masterConfig.dataOutConfig = kLpspiDataOutRetained;
    SPI_MasterInit(UWB_SPI_BASEADDR, &masterConfig, UWB_SPI_COCK_FREQ);

    return RETURNED_SUCCESS;
}

#if (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V2)
void phNxpUwb_GpioIrqEnable(void (*cb)(void))
{
#elif (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V3)
void phNxpUwb_GpioIrqEnable(void (*cb)(void *args))
{
#endif
    ENABLE_HELIOS_IRQ(cb);
}

#if (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V2)
void phNxpUwb_GpioIrqDisable(void (*cb)(void))
{
#elif (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V3)
void phNxpUwb_GpioIrqDisable(void (*cb)(void *args))
{
#endif
    DISABLE_HELIOS_IRQ();
}

bool phNxpUwb_RtcSyncWrite(bool value)
{
    return PIN_WRITE(HELIOS_SYNC, value);
}

bool phNxpUwb_RtcSyncRead()
{
    return PIN_READ(HELIOS_SYNC);
}

bool phNxpUwb_HeliosInteruptStatus()
{
    return UWB_GpioRead(HELIOS_INT);
}
