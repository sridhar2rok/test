/*
 * The Clear BSD License

 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "board.h"
#include "usb.h"
#include "fsl_debug_console.h"
#include "virtual_com.h"
#include "Uwb_Usb_Vcom_Pnp.h"
#include "UwbPnpInternal.h"

static void UWB_Usb_SendDoneCb(uint8_t *pData, uint16_t size);

extern usb_cdc_vcom_struct_t *s_cdcVcom_pt;
SemaphoreHandle_t mHifNtfnSem, mHifRspSem;

void Uwb_Usb_Init(void (*rcvCb)(uint8_t *, uint32_t *))
{
    /* This semaphore is signaled when ACK is received for the Bulkin Operations(USB Write) for sending Notifications from Rhodes*/
    mHifNtfnSem = xSemaphoreCreateBinary();
    if (!mHifNtfnSem) {
        PRINTF_WITH_TIME("Error: main, could not create semaphore mHifNtfnSem\n");
        while (1)
            ;
    }
    /* This semaphore is signaled when ACK is received for the Bulkin Operations(USB Write) for sending UCI resp from Rhodes*/
    mHifRspSem = xSemaphoreCreateBinary();
    if (!mHifRspSem) {
        PRINTF_WITH_TIME("Error: main, could not create semaphore mHifRspSem\n");
        while (1)
            ;
    }

    USB_Init(&UWB_Usb_SendDoneCb, rcvCb);
}
void UWB_Usb_SendDoneCb(uint8_t *pData, uint16_t size)
{
    if (pData != NULL && size > 4) {
        uint8_t header = pData[0] & 0xF0;
        if ((header == 0x60) || (header == 0x70)) {
            PRINTF("Sent ntf: 0x%X 0x%X 0x%X 0x%X 0x%X\n", pData[0], pData[1], pData[2], pData[3], pData[4]);
            BaseType_t xHigherPriorityTaskWoken;
            xSemaphoreGiveFromISR(mHifNtfnSem, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else if (header == 0x40) {
            PRINTF("Sent rsp: 0x%X 0x%X 0x%X 0x%X\n", pData[0], pData[1], pData[2], pData[3]);
            BaseType_t xHigherPriorityTaskWoken;
            xSemaphoreGiveFromISR(mHifRspSem, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else {
        }
    }
}
uint32_t UWB_Usb_UciSendNtfn(uint8_t *pData, uint16_t size)
{
    usb_status_t error = kStatus_USB_Error;
    error              = USB_DeviceCdcAcmSend(s_cdcVcom_pt->cdcAcmHandle, USB_CDC_VCOM_BULK_IN_ENDPOINT, pData, size);
    if (error != kStatus_USB_Success) {
        PRINTF("UWB_WRITER: error sending over USB [%d]\n", error);
    }
    else {
#if (INTERNAL_FIRMWARE_DOWNLOAD == ENABLED)
        xSemaphoreTake(mHifNtfnSem, 100); /* Wait for 100msec */
#else
        if (xSemaphoreTake(mHifNtfnSem, 10000) == pdFALSE) {
            /* Wait for worst case 10 Sec, Since Windows Host is Slow */
            PRINTF("Ntf Timed out after 10 sec of waiting\n");
        }
#endif
    }
    return (uint32_t)error;
}

uint32_t UWB_Usb_SendUCIRsp(uint8_t *pData, uint16_t size)
{
    usb_status_t error = kStatus_USB_Error;
    error              = USB_DeviceCdcAcmSend(s_cdcVcom_pt->cdcAcmHandle, USB_CDC_VCOM_BULK_IN_ENDPOINT, pData, size);
    if (error != kStatus_USB_Success) {
        PRINTF("UCI_READER: error sending over USB [%d]\n", error);
    }
    if (xSemaphoreTake(mHifRspSem, 5000) == pdTRUE) {
        /* Wait for Max 5 sec to read the response by windows since it is slow.
           Waiting for Response */
    }
    else {
        PRINTF("UCI_READER: Sem Timeout for Uci Rsp\n");
    }
    return (uint32_t)error;
}

uint32_t UWB_Usb_SendRsp(uint8_t *pData, uint16_t size)
{
    usb_status_t error = kStatus_USB_Error;
    error              = USB_DeviceCdcAcmSend(s_cdcVcom_pt->cdcAcmHandle, USB_CDC_VCOM_BULK_IN_ENDPOINT, pData, size);
    if (error != kStatus_USB_Success) {
        PRINTF("UCI_READER: error sending over USB [%d]\n", error);
    }
    return (uint32_t)error;
}
