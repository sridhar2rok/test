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

#include "TLV_Builder.h"
#include "TLV_Defs.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Standard library*/
#include <string.h>
#include <stdbool.h>

/* Interface includes */
#include "private_profile_interface.h"
//#include "UWBT_Uart.h"

/* Log */
#include "fsl_debug_console.h"

#define MAX_RSP_LEN 64
static uint8_t mTlv[TLV_VALUE_OFFSET + MAX_RSP_LEN];
static uint16_t mValueLen = 0;
static uint8_t *mValuePtr;

extern QueueHandle_t tlvMngQueue;
static SemaphoreHandle_t mHifSem;

#define phPlatform_Is_Irq_Context() (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)

static volatile UWB_Hif_t mInterface = UWB_HIF_BLE;

static struct
{
    uint8_t data[TLV_MAX_TOTAL_SIZE];
    uint16_t rcvSize;
    uint16_t expSize;
    bool offsetFlag;
} rcvTlv;

bool tlvStart(uint8_t type)
{
    bool ok   = true;
    mValueLen = 0;
    mTlv[0]   = type;
    mValuePtr = &mTlv[3];

    return ok;
}

static void tlvCancel(void)
{
    mValueLen = 0;
    mValuePtr = &mTlv[3];
}

bool tlvAdd_UINT8(uint8_t data)
{
    bool ok = true;
    if (mValueLen + 1 > MAX_RSP_LEN) {
        ok = false;
        //PRINTF("%s(): value field too large\n", __FUNCTION__);
        tlvCancel();
        goto end;
    }

    *mValuePtr = data;
    mValuePtr++;
    mValueLen++;
end:
    return ok;
}

bool tlvAdd_UINT16(uint16_t data)
{
    bool ok = true;

    if (mValueLen + 2 > MAX_RSP_LEN) {
        ok = false;
        //PRINTF("%s(): value field too large\n", __FUNCTION__);
        tlvCancel();
        goto end;
    }

    mValuePtr[1] = data & 0xFF;
    mValuePtr[0] = (data >> 8) & 0xFF;
    mValuePtr += 2;
    mValueLen += 2;

end:
    return ok;
}

bool tlvAdd_UINT32(uint32_t data)
{
    bool ok = true;

    if (mValueLen + 4 > MAX_RSP_LEN) {
        ok = false;
        //PRINTF("%s(): value field too large\n", __FUNCTION__);
        tlvCancel();
        goto end;
    }

    mValuePtr[3] = data & 0xFF;
    mValuePtr[2] = ((data >> 8) & 0xFF);
    mValuePtr[1] = ((data >> 16) & 0xFF);
    mValuePtr[0] = ((data >> 24) & 0xFF);

    mValuePtr += 4;
    mValueLen += 4;

end:
    return ok;
}

bool tlvAdd_PTR(uint8_t *data, uint16_t len)
{
    bool ok = true;

    if (mValueLen + len > MAX_RSP_LEN) {
        ok = false;
        //PRINTF("%s(): value field too large\n", __FUNCTION__);
        tlvCancel();
        goto end;
    }

    memcpy(mValuePtr, data, len);
    mValuePtr += len;
    mValueLen += len;

end:
    return ok;
}

bool tlvSend(void)
{
    /* Send TLV */
    bool ok = true;

    mTlv[1] = mValueLen & 0xff;
    mTlv[2] = (mValueLen >> 8) & 0xff;

    if (mInterface == UWB_HIF_BLE) {
        if (!Qpp_Send(mTlv, mValueLen + TLV_VALUE_OFFSET)) {
            PRINTF("%s(): failed to send over BLE\n", __FUNCTION__);
            ok = false;
            goto end;
        }
#if 0 // Disabled for Tag V3
    }else if(mInterface == UWB_HIF_UART){

        if(!UWB_UartSend(mTlv, mValueLen + TLV_VALUE_OFFSET)){
            PRINTF("%s(): failed to send over UART\n", __FUNCTION__);
            ok = false;
            goto end;
        }
#endif
    }
    else {
        PRINTF("%s(): ERROR, invalid interface %x\n", __FUNCTION__, mInterface);
        ok = false;
        goto end;
    }

    if (xSemaphoreTake(mHifSem, portMAX_DELAY) != pdTRUE) {
        PRINTF("%s(): failed to wait HIF semaphore\n", __FUNCTION__);
        ok = false;
        goto end;
    }

end:
    return ok;
}

void tlvSendDoneCb(void)
{
    BaseType_t higherPrioTask;
    if (phPlatform_Is_Irq_Context()) {
        xSemaphoreGiveFromISR(mHifSem, &higherPrioTask);
        portYIELD_FROM_ISR(higherPrioTask);
    }
    else {
        xSemaphoreGive(mHifSem);
        portYIELD();
    }
}

void tlvRecv(UWB_Hif_t interface, uint8_t *tlv, uint8_t tlvSize)
{
    mInterface = interface;

    memcpy(rcvTlv.data + rcvTlv.rcvSize, tlv, tlvSize);
    rcvTlv.rcvSize += tlvSize;

    if (rcvTlv.rcvSize >= TLV_VALUE_OFFSET && !rcvTlv.offsetFlag) {
        /* Parse value-field length. */
        rcvTlv.expSize    = ((rcvTlv.data[2] << 8) | rcvTlv.data[1]) + TLV_VALUE_OFFSET;
        rcvTlv.offsetFlag = true;
    }

    if (rcvTlv.rcvSize >= TLV_VALUE_OFFSET && rcvTlv.expSize == rcvTlv.rcvSize) {
        /* Full TLV received. */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        static tlv_t tlv;
        tlv.type          = rcvTlv.data[0];
        tlv.size          = rcvTlv.expSize - TLV_VALUE_OFFSET;
        tlv.value         = &rcvTlv.data[TLV_VALUE_OFFSET];
        rcvTlv.rcvSize    = 0;
        rcvTlv.offsetFlag = false;

        if (phPlatform_Is_Irq_Context()) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            xQueueSendFromISR(tlvMngQueue, &tlv, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else {
            xQueueSend(tlvMngQueue, &tlv, pdMS_TO_TICKS(0));
            portYIELD();
        }
    }
}

void tlvFlushInput(void)
{
    //PRINTF("%s(): flushing input buffer\n", __FUNCTION__);
    rcvTlv.expSize = rcvTlv.rcvSize = 0;
    rcvTlv.offsetFlag               = false;
}

bool tlvBuilderInit(void)
{
    rcvTlv.rcvSize = rcvTlv.expSize = 0;
    rcvTlv.offsetFlag               = false;

    mHifSem = xSemaphoreCreateCounting(1, 0);
    if (!mHifSem) {
        PRINTF("Error: %s(): Could not create TLV mutex\n", __FUNCTION__);
        return false;
    }

#if 0 // Disabled for Tag V3
    if(!UWB_UartInit(tlvSendDoneCb, tlvRecv)){
        PRINTF("Error: %s(): Could not initialize UART Hif\n");
        return false;
    }
#endif
    return true;
}
