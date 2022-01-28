/*
 * Copyright 2019 NXP.
 *
 * All rights are reserved. Reproduction in whole or in part is
 * prohibited without the written consent of the copyright owner.
 *
 * NXP reserves the right to make changes without notice at any time.
 *
 * NXP makes no warranty, expressed, implied or statutory, including but
 * not limited to any implied warranty of merchantability or fitness for any
 * particular purpose, or that the use will not infringe any third party patent,
 * copyright or trademark. NXP must not be liable for any loss or damage
 * arising from its use.
 */

#include "Utilities.h"
#include "phUwb_BuildConfig.h"
#include "UwbApi.h"
#if 1 //(UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_STANDALONE_DEFAULT)
#include "AppInternal.h"
#include "handler.h"
#if defined(ENABLE_NFC) && ENABLE_NFC == TRUE
#include "SeApi.h"
#include "wearable_platform_int.h"
#include "Binding_SE_Test.h"
#endif

#include "UWB_GpioIrq.h"
#include "TLV_Builder.h"
#include "Utilities.h"

extern QueueHandle_t tlvMngQueue;

void UWB_AppCallback(eNotificationType opType, void *pData)
{
    // Empty callback
}

void AppCallback_BLEApp(eNotificationType opType, void *pData)
{
    switch (opType) {
    case UWBD_ACTION_APP_CLEANUP: {
        // Post to TLV task in app
        tlv_t tlv;
        static uint8_t buf[2] = {NTF_HPD_WAKEUP, UWBAPI_STATUS_HPD_WAKEUP};

        tlv.type  = UWB_NTF_MANAGEMENT;
        tlv.size  = 2;
        tlv.value = &buf[0];
        xQueueSend(tlvMngQueue, &tlv, pdMS_TO_TICKS(0));

        // Handle_ErrorScenario(APP_CLEANUP);
    } break;
    default:
        break;
    }
}

extern UwbHandlerState mState;

void setup_handler(
    UINT8 subType, UINT8 *valueBuffer, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    *pRespTagType = UWB_SETUP_MANAGEMENT;
    switch (subType) {
    case UWB_INIT: {
        SWITCH_HELIOS(true);

#if UWBIOT_UWBD_SR100T
        UINT8 fwMode = 0;
        UWB_STREAM_TO_UINT8(fwMode, valueBuffer);
        if (fwMode == FACTORY_FW)
            status = UwbApi_FactoryInit(AppCallback);
        else
            status = UwbApi_Init(UWB_AppCallback);
#elif UWBIOT_UWBD_SR040
        status = UwbApi_Init(AppCallback_BLEApp);
#endif

#if defined(ENABLE_NFC) && ENABLE_NFC == TRUE
        if (SeApi_InitWithFirmware(&transactionOccurred,
                NULL,
                gphDnldNfc_DlSeqSz,
                gphDnldNfc_DlSequence,
                gphDnldNfc_DlSeqDummyFwSz,
                gphDnldNfc_DlSequenceDummyFw) != SEAPI_STATUS_OK) {
            PRINTF("SeApi_Init() Failed");
        }
#endif

        if (status == UWBAPI_STATUS_OK)
            mState = init;

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);

    } break;

    case UWB_SHUTDOWN: {
        status = UwbApi_ShutDown();

        if (status == UWBAPI_STATUS_OK) {
            mState = uninit;
            SWITCH_HELIOS(false);
            UWB_GpioIrqDisable(HELIOS_IRQ);
        }

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
    } break;
    case UWB_FACTORY_TEST: {
#if defined(FACTORY_MODE) && FACTORY_MODE == ENABLED
        factoryFwTestStatus_t status = doFactoryFwTest();
        serializeDataFromFactoryTestStatus(status, pRespBuf, pRespSize);
#endif
    } break;
    case UWB_MAINLINE_TEST: {
#if UWBIOT_UWBD_SR100T
        mainLineFwTestStatus_t status = doMainlineFwTest();
        serializeDataFromMainlineTestStatus(status, pRespBuf, pRespSize);
#elif UWBIOT_UWBD_SR040
        status = UWBAPI_STATUS_INVALID_PARAM; // UWBAPI not supported on SR040
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#endif
    } break;

    case MCU_RESET: {
        //PRINTF_WITH_TIME("Issuing NVIC_SystemReset, MCU going to reboot!\n");
        NVIC_SystemReset(); // No prints after this line wont appear
    } break;
    default:
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
        PRINTF("%s: Unknown Command\n", __func__);
        break;
    }
}

#endif
