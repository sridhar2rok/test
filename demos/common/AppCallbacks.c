/* Copyright 2019,2020 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */

#include "phOsalUwb.h"
#include "AppInternal.h"
//#include "UwbUsb.h"
//#include "Utilities.h"
#include "AppRecovery.h"

void *perSem;
void *rangingDataSem;
void *inBandterminationSem;
void *testLoopBackNtfSem;
UINT8 data_buff[MAX_APP_DATA_SIZE];
extern QueueHandle_t mUsbMutex;

void Init()
{
    phOsalUwb_CreateSemaphore(&perSem, 1);
    phOsalUwb_CreateSemaphore(&rangingDataSem, 1);
    phOsalUwb_CreateSemaphore(&inBandterminationSem, 1);
    phOsalUwb_CreateSemaphore(&testLoopBackNtfSem, 1);
}

void AppCallback(eNotificationType opType, void *pData)
{
    switch (opType) {
    case UWBD_RANGING_DATA: {
        phRangingData_t *pRangingData = (phRangingData_t *)pData;
#if (UWBIOT_UWBD_SR100T)
        if ((pRangingData->range_meas[0].status == 0x00) && ((pRangingData->range_meas[0].distance != 0xFFFF))) {
            phOsalUwb_ProduceSemaphore(rangingDataSem);
            printRangingData(pRangingData);
            break;
        }
#endif
        printRangingData(pRangingData);
        break;
    }
    case UWBD_TEST_MODE_LOOP_BACK_NTF: {
#if (UWBIOT_UWBD_SR040)
        Log("Group Delay  : %d\n", *(UINT32 *)pData);
        phOsalUwb_ProduceSemaphore(testLoopBackNtfSem);
#endif
        break;
    }
    case UWBD_RFRAME_DATA:
    case UWBD_SCHEDULER_STATUS_NTF: {
        // Nothing to be done
    } break;
#if (UWBIOT_UWBD_SR100T)
    case UWBD_PER_SEND: {
        Log("pPerTxData->status  : %d\n", ((phPerTxData_t *)pData)->status);
        phOsalUwb_ProduceSemaphore(perSem);
    } break;

    case UWBD_PER_RCV: {
        phOsalUwb_ProduceSemaphore(perSem);
    } break;
#endif
    case UWBD_GENERIC_ERROR_NTF: {
        printGenericErrorStatus((phGenericError_t *)pData);
    } break;

    case UWBD_DEVICE_RESET:  // Error Recovery: cleanup all states and end all states.
        cleanUpAppContext(); // This would have called while 1. SeComError 2. other reasons
        break;

    case UWBD_RECOVERY_NTF: // Error Recovery: do uwbd cleanup, fw download, move to ready state
        Handle_ErrorScenario(FW_CRASH);
        break;

    case UWBD_SESSION_DATA: {
        phUwbSessionInfo_t *pSessionInfo = (phUwbSessionInfo_t *)pData;
        printSessionStatusData(pSessionInfo);
        phOsalUwb_ProduceSemaphore(inBandterminationSem);
    } break;
    case UWBD_ACTION_APP_CLEANUP: {
        Handle_ErrorScenario(APP_CLEANUP);
    } break;
    default:
        break;
    }
}
