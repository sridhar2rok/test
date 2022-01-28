/* Copyright 2020 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */

#include "phUwb_BuildConfig.h"
#include <string.h>
#include <stdint.h>
#include "UWB_GpioIrq.h"
#include "UWB_Spi_Driver_Interface.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "driver_config.h"
#include "UwbCore_Types.h"
#include "phUwbErrorCodes.h"

extern interface_config_t swInterface_config;
static QueueHandle_t mIrqWaitSem;

#define FW_MODE_IS_UCI  (swInterface_config.interface_mode == kInterfaceModeUci)
#define FW_MODE_IS_SWUP (swInterface_config.interface_mode == kInterfaceModeSwup)

#define RECEIVE_SWUP_RESPONSE           \
    retry:                              \
    if (retry_count > 9) {              \
        return 0;                       \
    }                                   \
    resp_len = phNxpUwb_RciRead(resp);  \
    if (resp_len == 0) {                \
        vTaskDelay(pdMS_TO_TICKS(100)); \
        retry_count++;                  \
        goto retry;                     \
    }

uint16_t UWB_SpiUciRead(uint8_t *rsp)
{
    if (FW_MODE_IS_UCI) {
        if (!phNxpUwb_HeliosInteruptStatus()) {
            /* Interrupt is active */
            return phNxpUwb_UciRead(rsp);
        }
        else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

uint8_t UWB_HeliosSpiInit(void)
{
    phNxpUwb_HeliosInit();

    /*This semaphore is signaled in the ISR context.*/
    mIrqWaitSem = xQueueCreateCountingSemaphore(1, 0);
    if (!mIrqWaitSem) {
        DEBUGOUT("Error: UWB_HeliosInit(), could not create semaphore mWaitIrqSem\n");
        return false;
    }

    return true;
}

uint16_t UWB_SpiUciWrite(uint8_t *data, uint16_t len)
{
    int ret;
    if (FW_MODE_IS_UCI) {
        while (!phNxpUwb_HeliosInteruptStatus() || !phNxpUwb_RdyRead()) {
        }
        ret = phNxpUwb_UciWrite(data, len);
        if (ret == -1) {
            return 0;
        }
        else {
            return (uint16_t)(ret);
        }
    }
    else {
        return 0;
    }
}

uint16_t UWB_SpiRciTransceive(uint8_t *data, uint16_t len)
{
    uint16_t resp_len                       = 0;
    static uint8_t resp[RCI_COMMAND_LENGTH] = {0};
    int retry_count                         = 0;

    if (FW_MODE_IS_SWUP) {
        while (!phNxpUwb_HeliosInteruptStatus() || !phNxpUwb_RdyRead()) {
        }
        if (phNxpUwb_RciWrite(data, len) == RETURNED_SUCCESS) {
            RECEIVE_SWUP_RESPONSE;

            if (0 == UWB_Hif_SendUCIRsp(resp, resp_len)) {
                return resp_len;
            }
        }
    }
    return 0;
}
