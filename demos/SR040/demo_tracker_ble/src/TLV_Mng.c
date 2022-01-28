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

/* TLV */
#include "TLV_Defs.h"
#include "TLV_Builder.h"
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Driver includes */
#include "fsl_debug_console.h"

/* Handlers */
#include "GPIO_Handler.h"
#include "UWB_ConfigHandler.h"
#include "handler.h"
#include "UWB_GpioIrq.h"

#define TLV_MNG_STACK_SIZE 400
#define TLV_MNG_PRIO       (tskIDLE_PRIORITY + 2)

QueueHandle_t tlvMngQueue;

static xTaskHandle mTlvMngHnd;
static void tlvMngTask(void *args);

static void handleTLV(tlv_t *tlv);
QueueHandle_t mTlvMutex;

UwbHandlerState mState = uninit;
uint32_t mSessionId;

bool tlvMngInit(void)
{
    tlvMngQueue = xQueueCreate(1, sizeof(tlv_t));
    if (!tlvMngQueue) {
        //PRINTF("Error: %s(), could not create queue tlvMngQueue\n", __FUNCTION__);
        return false;
    }

    mTlvMutex = xSemaphoreCreateMutex();
    if (!mTlvMutex) {
        PRINTF("Error: %s(): Could not create TLV mutex\n", __FUNCTION__);
        return false;
    }

    xTaskCreate(tlvMngTask, "TlvMng", TLV_MNG_STACK_SIZE, NULL, TLV_MNG_PRIO, &mTlvMngHnd);
    if (!mTlvMngHnd) {
        //PRINTF("Error: %s(), could not create tlvMng task\n", __FUNCTION__);
        return false;
    }
    return true;
}

void tlvMngTask(void *args)
{
    while (1) {
        //tlv_t tlv;
        tlv_t evt;
        if (xQueueReceive(tlvMngQueue, &evt, portMAX_DELAY) == pdFALSE) {
            continue;
        }

        //PRINTF("%s(): handling TLV %02x\n", __FUNCTION__, tlv.type);
        handleTLV(&evt);
    }
}

static void handleTLV(tlv_t *tlv)
{
    static UINT8 respBuffer[TLV_MAX_VALUE_SIZE];

    UINT16 respSize   = 0;
    UINT8 respTagType = TLV_TYPE_END;

    UINT8 subType      = tlv->value[0];
    UINT8 *data        = &tlv->value[1];
    UINT16 dataLength  = (UINT16)(tlv->size - sizeof(subType));
    UINT8 *respPayload = &respBuffer[4];

    bool flushHCI = false;

    PRINTF("[TLV %02x]\n", tlv->type);

    xSemaphoreTake(mTlvMutex, portMAX_DELAY);

    switch (tlv->type) {
    case SESSION_MANAGEMENT: {
        session_handler(subType, data, dataLength, &respSize, respPayload, &respTagType);
        PRINTF("Session command\n");
    } break;
    case CONFIG_MANAGEMENT: {
        PRINTF("Configuration command\n");
        config_handler(subType, data, dataLength, &respSize, respPayload, &respTagType);
    } break;
    case RANGE_MANAGEMENT: {
        ranging_handler(subType, data, dataLength, &respSize, respPayload, &respTagType);
        PRINTF("Range command\n");
    } break;
#if UWBIOT_UWBD_SR100T
    case RF_TEST_MANAGEMENT: {
        rf_test_handler(subType, data, dataLength, &respSize, respPayload, &respTagType);
        PRINTF("RF Test command\n");
    } break;
#endif
    case UWB_MISC: {
        uwb_misc_handler(subType, data, dataLength, &respSize, respPayload, &respTagType);
        PRINTF("UWB MISC command\n");
    } break;
#if defined(ENABLE_NFC) && ENABLE_NFC == TRUE
    case SE_MANAGEMENT: {
        se_handler(subType, data, dataLength, &respSize, respPayload, &respTagType);
        PRINTF("SE command\n");
    }
#endif
    break;
    case UWB_NTF_MANAGEMENT: {
        ntf_handler(subType, data, dataLength, &respSize, respPayload, &respTagType);
    } break;

    case UWB_SETUP_MANAGEMENT: {
        setup_handler(subType, data, dataLength, &respSize, respPayload, &respTagType);
    } break;

    case UI_CMD: {
        tlvStart(UI_RSP);
        handleUiCmd(tlv, &flushHCI);
    } break;

    case CONFIG_DEVICE_CMD: {
        tlvStart(CONFIG_DEVICE_CMD);
        handleConfigCmd(tlv, &flushHCI);
    } break;

    default:
        PRINTF("%s: Unknown Command\n", __func__);
        break;
    }

    if (tlv->type <= UWB_SETUP_MANAGEMENT) {
        tlvStart(respTagType);             // TLV Type
        tlvAdd_UINT8(tlv->value[0]);       // TLV Subtype
        tlvAdd_PTR(respPayload, respSize); // TLV payload
    }

    tlvSend();
    xSemaphoreGive(mTlvMutex);
}

void handleShutDown(void)
{
    bool status              = true;
    tUWBAPI_STATUS operation = UWBAPI_STATUS_OK;
    ;
    while (mState != uninit && status) {
        switch (mState) {
        case uninit:
            PRINTF("Already shut down\n");
            break;

        case init:
            operation = UwbApi_ShutDown();

            //Filter error 0xE0 too, despite having the error the MW is deinitialized
            // and can later be initialized again
            if (operation == UWBAPI_STATUS_OK || operation == 0xE0) {
                PRINTF("Deinitialized uwb\n");
                SWITCH_HELIOS(false);
                UWB_GpioIrqDisable(HELIOS_IRQ);
                mState = uninit;
            }
            else {
                status = false;
                PRINTF("Error shutting down: %02X\n", operation);
            }

            break;

        case sessionCreated:
            if ((operation = UwbApi_SessionDeinit(mSessionId)) == UWBAPI_STATUS_OK) {
                PRINTF("Stopped session\n");
                mState = init;
            }
            else {
                status = false;
                PRINTF("Error deleting session: %02X\n", operation);
            }
            break;

        case ranging:
            if ((operation = UwbApi_StopRangingSession(mSessionId)) == UWBAPI_STATUS_OK) {
                PRINTF("Stopped ranging session\n");
                mState = sessionCreated;
            }
            else {
                status = false;
                PRINTF("Error stopping ranging session: %02X\n", operation);
            }
            break;

        default:
            break;
        }
    }
}
