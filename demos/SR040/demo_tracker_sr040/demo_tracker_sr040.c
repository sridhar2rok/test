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

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#include "phUwb_BuildConfig.h"
#if 1 // UWB_BUILD_STANDALONE_DEFAULT

#include "UwbApi.h"
#include <AppInternal.h>
#include "AppRecovery.h"
#include "app_Ranging_Cfg.h" /* Include from -I not current directory */
#include "phOsalUwb_Thread.h"
#include <SwupApi.h>
#include "../demo_swup_update_fw/SwupPkgFW.h"
#include <phNxpUwb_SpiTransport.h>
#include <Swup_update.h>
#include "SwupUCI.h"
#include "UwbApi_RadioConfig.h"

/*
 * Below list contains the application configs which are only related to default configuration.
 */

/********************************************************************************/
/*               Ranging APP configuration setting here                         */
/********************************************************************************/

UINT8 DEVICE_MAC_ADDR_SHORT[2] = {0x11, 0x11};
UINT8 DST_MAC_ADDR_SHORT[2]    = {0x22, 0x22};
#define RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT 0x0

/********************************************************************************/

typedef struct
{
    eAppConfig Id;
    UINT32 value;
} Param_t;

static tUWBAPI_STATUS UpdateFirmware(void)
{
    tUWBAPI_STATUS update_status = UWBAPI_STATUS_FAILED;
    SwupResponseStatus_t swup_response;
    UwbApi_ShutDown();
    phNxpUwb_HeliosInit();
    phNxpUwb_HeliosReset();

    update_status = Uci_EnableSwup();
    phOsalUwb_Delay(500);
    if (update_status != UWBAPI_STATUS_OK) {
        goto exit;
    }
    else {
        phNxpUwb_SwitchProtocol(kInterfaceModeSwup);
    }

    swup_response = SwupUpdate(MAC_RANGING_FW_PKG, MAC_RANGING_FW_PKG_LEN, kSWUP_KEY_VESRION_UNKNOWN);

    if (STATUS_CMD_SUCCESS != swup_response) {
        update_status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
    else {
        update_status = UWBAPI_STATUS_OK;
    }

exit:
    phOsalUwb_Delay(2000);
    phNxpUwb_SwitchProtocol(kInterfaceModeUci);
    /** De-initialize transport layer
     *  Initialization should be taken care in
     *  UWB stack init or the application layer (in case of failure)
     */
    phNxpUwb_HeliosDeInit();

    return update_status;
}

static SwupResponseStatus_t RecoverFirmware(void)
{
    SwupDeviceId_t swupDeviceId;
    SwupResponseStatus_t swup_response;

    /** Shutdown UCI stack and open only
     *  the transport layer.
     */
    UwbApi_ShutDown();

    /* Initialize transport layer to send raw commands */
    phNxpUwb_HeliosInit();

    /* Reset the Helios Chip */
    phNxpUwb_HeliosReset();

    /* Switch to SWUP mode */
    phNxpUwb_SwitchProtocol(kInterfaceModeSwup);
    if (Swup_ReadDeviceId(&swupDeviceId) != STATUS_CMD_SUCCESS) {
        /* Not in SWUP. Exit. */
        swup_response = STATUS_GENERIC_ERROR;
        goto exit;
    }

    swup_response = SwupUpdate(MAC_RANGING_FW_PKG, MAC_RANGING_FW_PKG_LEN, kSWUP_KEY_VESRION_UNKNOWN);

    if (STATUS_CMD_SUCCESS == swup_response) {
        phOsalUwb_Delay(1000);
        phNxpUwb_SwitchProtocol(kInterfaceModeUci);
        /** De-initialize transport layer
         *  Initialization should be taken care in
         *  UWB stack init or the application layer (in case of failure)
         */
        phNxpUwb_HeliosDeInit();
    }

exit:
    return swup_response;
}

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("SR040 : Demo Tracker SR040");

    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

restart:

    /** Main function to perform SWUP */
    status = UwbApi_Init(AppCallback);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_Init Failed");
        /* Try to Recover the Firwmare */
        if (STATUS_CMD_SUCCESS != RecoverFirmware()) {
            status = UWBAPI_STATUS_FAILED;
            goto exit;
        }
        else {
            goto restart;
        }
    }

    /* Do a safe Radio Configs  update... skip if CRC Matches */
    status = RadioConfig_tdoaAndTracker(false);
    if (status != UWBAPI_STATUS_OK) {
        Log("RadioConfig_tdoaAndTracker Failed");
        goto exit;
    }

    phUwbDevInfo_t devInfo;
    uint32_t fwVersion = 0;
    status             = UwbApi_GetStackCapabilities(&devInfo);
    printStackCapabilities(&devInfo);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_GetStackCapabilities Failed");
        goto exit;
    }
    if (status == UWBAPI_STATUS_OK) {
        fwVersion = devInfo.fwMajor << 8 * 2 | devInfo.fwMinor << 8 | devInfo.fwPatchVersion;
    }
    else {
        fwVersion = 0xFFFFFF;
    }
    if (fwVersion == MAC_RANGING_FW_PKG_VERSION) {
        NXPLOG_APP_W("Same FW version found, start ranging");
    }
    else {
        NXPLOG_APP_W("Fw Version Mismatch, Updating");
        status = UpdateFirmware();
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_W("Fw Update Failed");
            goto exit;
        }
        else {
            status = UwbApi_Init(AppCallback);
        }
    }

    status = UwbApi_SessionInit(RANGING_APP_SESSION_ID, UWBD_RANGING_SESSION);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_SessionInit() Failed");
        goto exit;
    }

    const SetAppParams_List_t SetAppParamsList[] = {
        UWB_SET_APP_PARAM_VALUE(RANGING_METHOD, 0x02),
        UWB_SET_APP_PARAM_VALUE(RFRAME_CONFIG, 0x03),
        UWB_SET_APP_PARAM_VALUE(SLOTS_PER_RR, 24),
        UWB_SET_APP_PARAM_VALUE(RANGING_INTERVAL, 24 * 8),
        UWB_SET_APP_PARAM_VALUE(SFD_ID, 0),
        UWB_SET_APP_PARAM_VALUE(TX_POWER_ID, 0),
    };

    status = UwbApi_SetAppConfigMultipleParams(
        RANGING_APP_SESSION_ID, sizeof(SetAppParamsList) / sizeof(SetAppParamsList[0]), &SetAppParamsList[0]);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_SetAppConfigMultipleParams() Failed");
        goto exit;
    }

    phRangingParams_t inRangingParams;
    inRangingParams.deviceRole     = kUWB_DeviceRole_Initiator;
    inRangingParams.multiNodeMode  = kUWB_MultiNodeMode_UniCast;
    inRangingParams.noOfControlees = RANGING_APP_NO_OF_ANCHORS_P2P;
    inRangingParams.deviceType     = kUWB_DeviceType_Controller;
    inRangingParams.macAddrMode    = RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT;

    inRangingParams.deviceMacAddr[0] = DEVICE_MAC_ADDR_SHORT[0];
    inRangingParams.deviceMacAddr[1] = DEVICE_MAC_ADDR_SHORT[1];

    inRangingParams.dstMacAddr[0] = DST_MAC_ADDR_SHORT[0];
    inRangingParams.dstMacAddr[1] = DST_MAC_ADDR_SHORT[1];

    status = UwbApi_SetRangingParams(RANGING_APP_SESSION_ID, &inRangingParams);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_SetRangingParams() Failed");
        goto exit;
    }

    phRangingParams_t outRangingParams;
    status = UwbApi_GetRangingParams(RANGING_APP_SESSION_ID, &outRangingParams);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_GetRangingParams() Failed");
        goto exit;
    }

    status = UwbApi_StartRangingSession(RANGING_APP_SESSION_ID);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_StartRangingSession() Failed");
        goto exit;
    }

    phOsalUwb_Delay(5 * 60 * 1000);

    uint32_t delay = 5 * 60 * 1000; /*Waiting for 5 mins*/
    /* When Ranging is terminated due to inband termination this semaphore will
   * be signaled, otherwise ranging will be performed for the time specified */
    if (phOsalUwb_ConsumeSemaphore_WithTimeout(inBandterminationSem, delay)) {
        status = UWBAPI_STATUS_OK;
        Log("\n-------------------------------------------\n in band termination "
            "is done  \n-------------------------------------------\n");
        UwbApi_SessionDeinit(RANGING_APP_SESSION_ID);
        goto exit;
    }

    status = UwbApi_StopRangingSession(RANGING_APP_SESSION_ID);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_StopRangingSession() Failed");
        goto exit;
    }

    status = UwbApi_SessionDeinit(RANGING_APP_SESSION_ID);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_SessionDeinit() Failed");
        goto exit;
    }

exit:
    if (status == UWBAPI_STATUS_TIMEOUT) {
        Handle_ErrorScenario(TIME_OUT);
    }
    else if (status == UWBAPI_STATUS_HPD_WAKEUP) {
        /* This must after HPD. Device Reset is must to come out of HPD*/
        status = UwbApi_ShutDown();
        if (status != UWBAPI_STATUS_OK) {
            Log("UwbApi_ShutDown() Failed");
        }
        else
            goto restart;
    }

    UWBIOT_EXAMPLE_END(status);
}

#endif // UWB_BUILD_STANDALONE_DEFAULT
