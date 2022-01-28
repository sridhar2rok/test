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

#include "UwbPnpInternal.h"
#if UWBIOT_UWBD_SR100T
#include "Uwb_Usb_Vcom_Pnp.h"
#endif
#if UWBIOT_UWBD_SR040
#include "pnp_usart.h"
#endif
#include "UWB_Evt.h"
#include "fsl_debug_console.h"
#include "phUwb_BuildConfig.h"

extern bool mError;
volatile uint16_t mRcvTlvSize    = 0;
volatile uint16_t mRcvTlvSizeExp = 0;
static uint8_t mRcvTlvBuf[MAX_RSP_PACKET_SIZE];
QueueHandle_t mHifCommandQueue;

bool Uwb_Is_Hif_Active()
{
    return mError;
}

void Uwb_Reset_Hif_State(bool state)
{
    mError = state;
}

uint32_t UWB_Hif_UciSendNtfn(uint8_t *pData, uint16_t size)
{
#if UWBIOT_UWBD_SR100T
    return UWB_Usb_UciSendNtfn(pData, size);
#elif UWBIOT_UWBD_SR040
    return UWB_USART_UciSendNtfn(pData, size);
#endif
}

uint32_t UWB_Hif_SendUCIRsp(uint8_t *pData, uint16_t size)
{
#if UWBIOT_UWBD_SR100T
    return UWB_Usb_SendUCIRsp(pData, size);
#elif UWBIOT_UWBD_SR040
    return UWB_USART_SendUCIRsp(pData, size);
#endif
}

uint32_t UWB_Hif_SendRsp(uint8_t *pData, uint16_t size)
{
#if UWBIOT_UWBD_SR100T
    return UWB_Usb_SendRsp(pData, size);
#elif UWBIOT_UWBD_SR040
    return UWB_USART_SendRsp(pData, size);
#endif
}
void Uwb_Hif_ReadDataCb(uint8_t *pData, uint32_t *pLen)
{
    while (*pLen > 0) {
        uint32_t cpSize;
        if (mRcvTlvSize < 3) {
            cpSize = 1;
        }
        else if (mRcvTlvSize + *pLen <= mRcvTlvSizeExp) {
            cpSize = *pLen;
        }
        else {
            cpSize = mRcvTlvSizeExp - mRcvTlvSize;
        }

        memcpy(&mRcvTlvBuf[mRcvTlvSize], pData, cpSize);
        *pLen -= cpSize;
        pData += cpSize;
        mRcvTlvSize += cpSize;

        if (mRcvTlvSize == 3) {
            /* TLV size received */
            mRcvTlvSizeExp = 3 + (mRcvTlvBuf[1] << 8 | mRcvTlvBuf[2]);
        }
    }

    if (mRcvTlvSize == mRcvTlvSizeExp) {
        mRcvTlvSize = mRcvTlvSizeExp = 0;
        BaseType_t xHigherPriorityTaskWoken;
        mError = false;
        xSemaphoreGiveFromISR(mHifIsr_Sem, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void UWB_Hif_Init()
{
}

void UWB_Hif_Handler_Task(void *args)
{
    UWB_Evt_t evt;
    tlv_t tlv;

    evt.type = USB_TLV_EVT;
    evt.data = &tlv;
#if UWBIOT_UWBD_SR100T
    Uwb_Usb_Init(&Uwb_Hif_ReadDataCb);
#elif UWBIOT_UWBD_SR040
    Uwb_USART_Init(&Uwb_Hif_ReadDataCb);
#endif

    while (1) {
        /* Wait to receive TLV over USB */
        if (!mError && xSemaphoreTake(mHifIsr_Sem, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        if (!mError) {
            tlv.type  = mRcvTlvBuf[0];
            tlv.size  = (mRcvTlvBuf[1] << 8 | mRcvTlvBuf[2]);
            tlv.value = &mRcvTlvBuf[3];
            if (tlv.size > 3)
                DEBUGOUT("USB 0x%X 0x%X 0x%X 0x%X\n", mRcvTlvBuf[0], mRcvTlvBuf[1], mRcvTlvBuf[2], mRcvTlvBuf[3]);
            else
                DEBUGOUT("USB 0x%X 0x%X 0x%X\n", mRcvTlvBuf[0], mRcvTlvBuf[1], mRcvTlvBuf[2]);
            DEBUGOUT("[TLV %02x]\n", tlv.type);
            xQueueSend(mHifCommandQueue, &evt, portMAX_DELAY);
        }
    }
}

void UWB_WriterTask(void *args)
{
    tlv_t tlv;
    while (1) {
        xSemaphoreTake(mHifSyncMutex, portMAX_DELAY);
        if (uxQueueSpacesAvailable(mHifWriteQueue) != WRITER_QUEUE_SIZE) {
            /* DEBUGOUT("Q Size %d \n",uxQueueSpacesAvailable(mUsbWriteQueue));*/
            if (xQueueReceive(mHifWriteQueue, &tlv, portMAX_DELAY) == pdFALSE) {
                xSemaphoreGive(mHifSyncMutex);
                vTaskDelay(
                    pdMS_TO_TICKS(5)); /* Give some time for Reader Task to Read and post the Data to this Queue.*/
                continue;
            }
            xSemaphoreTake(mHifWriteMutex, portMAX_DELAY);
            // uint8_t *Buffer = (uint8_t *)tlv.value;
            // DEBUGOUT("Sending ntf: 0x%X 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3], Buffer[4]);
            UWB_Hif_UciSendNtfn(tlv.value, tlv.size);
            DEBUGOUT("After Sending ntf\n");
            if ((uint8_t *)tlv.value != NULL) {
                vPortFree((uint8_t *)tlv.value);
                tlv.value = NULL;
                /* DEBUGOUT("FREE\r\n"); */
            }
            xSemaphoreGive(mHifWriteMutex);
        }
        xSemaphoreGive(mHifSyncMutex);
        vTaskDelay(pdMS_TO_TICKS(5)); /* Give some time for Reader Task to Read the Data.*/
    }
}

void UWB_HandleEvt(UWB_EvtType_t ev, void *args)
{
    switch (ev) {
    case USB_TLV_EVT:
#if UWBIOT_UWBD_SR100T
        UWB_Handle_SR100T_TLV((tlv_t *)args);
#elif UWBIOT_UWBD_SR040
        UWB_Handle_SR040_TLV((tlv_t *)args);
#endif
        break;
    default:
        PRINTF_WITH_TIME("ERROR: Unknown event type %02x\n", ev);
        break;
    }
}

void UWB_Pnp_App_Task(void *args)
{
    /* This may go somehwere else.. */
    UWB_Evt_t evt;

    PRINTF_WITH_TIME("UWB_HeliosTask(): suspending communication interfaces\n");
    /*Suspend USB task which gets USB messages from the Host. During HBCI internal download Rhodes can not accept
     * any commands from Host*/
    vTaskSuspend(mHifTask);
    /*Suspend ALL tasks which are used for UCI operations. During HBCI mode UCI tasks need to be disabled.*/
    vTaskSuspend(mUciReaderTask);
    vTaskSuspend(mHifWriterTask);

#if UWBIOT_UWBD_SR040
    UpdateFirmware();
#else
#if (INTERNAL_FIRMWARE_DOWNLOAD == ENABLED)
    UWB_GpioSet(HELIOS_ENABLE, 1);
    UWB_GpioSet(HELIOS_RTC_SYNC, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    if (UWB_HbciEncryptedFwDownload()) {
        PRINTF_WITH_TIME("UWB_HeliosTask(): HELIOS FW download completed\n");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    else {
        PRINTF_WITH_TIME("UWB_HeliosTask(): CRITICAL ERROR downloading Helios image\n");
    }
#endif // INTERNAL_FIRMWARE_DOWNLOAD
#endif // UWBIOT_UWBD_SR040

    PRINTF_WITH_TIME("UWB_HeliosTask(): resuming communication interfaces\n");
    /* After HBCI DND resume USB task. Now Host can send command over USB CDC interface.*/
    vTaskResume(mHifTask);
    /*Resume ALL tasks which are used for UCI operations. After HBCI mode UCI tasks need to be enabled.*/
    vTaskResume(mUciReaderTask);
    vTaskResume(mHifWriterTask);
    while (1) {
        if (xQueueReceive(mHifCommandQueue, &evt, portMAX_DELAY) == pdFALSE) {
            continue;
        }
        UWB_HandleEvt(evt.type, evt.data);
    }
}
