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

UINT8 DEVICE_MAC_ADDR_SHORT[2] = {0x00, 0x01};
UINT8 DST_MAC_ADDR_SHORT[2]    = {0x01, 0x00};

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
    phOsalUwb_Delay(1000);
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
    PRINT_APP_NAME("SR040 : Demo TDoA TAG SR040");

    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

restart:

    /** Main function to initialize stack */
    status = UwbApi_Init(AppCallback);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_Init Failed");
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
        LOG_E("UwbApi_GetStackCapabilities Failed");
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

    /* Before Session init, clear all the sessions in NVM */
    status = UwbApi_SessionNvmManage(SESSION_NVM_MANAGE_DELETE_ALL, 0);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SessionNvmManage() Failed");
        goto exit;
    }

    status = UwbApi_SessionInit(RANGING_APP_SESSION_ID, UWBD_RANGING_SESSION);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SessionInit() Failed");
        goto exit;
    }

    const Param_t listOfParams[] = {
        /*mandatory as per config Digest Usage*/
        {RFRAME_CONFIG, kUWB_RfFrameConfig_Sfd_Sts},
        {STS_CONFIG, kUWB_StsConfig_StatisSts_Tdoa},
        {RANGING_METHOD, kUWB_RangingMethod_TDoA},
        /* Keep the RANGING_INTERVAL (i.e. BLINK_INTERVAL) always more
         * than ~15ms */
        {RANGING_INTERVAL, 1 * 1000},
        {BLINK_RANDOM_INTERVAL, 10},
        //{RNG_DATA_NTF, 1},
        {SFD_ID, 0},
        //{SLOTS_PER_RR, 24},
        //{MAC_CFG, 3},
        //{SLOT_DURATION, 2000},
        {CHANNEL_NUMBER, 9},
        {PREAMBLE_CODE_INDEX, 10},
        {TX_POWER_ID, 30}, /* 0 => MAX, 104 => MIN */
    };

    for (size_t index = 0; index < sizeof(listOfParams) / sizeof(Param_t); index++) {
        status = UwbApi_SetAppConfig(RANGING_APP_SESSION_ID, listOfParams[index].Id, listOfParams[index].value);
        if (status != UWBAPI_STATUS_OK) {
            LOG_W(
                "ERROR! Set for Param Id: 0x%2x, Param Value : %ld", listOfParams[index].Id, listOfParams[index].value);
            goto exit;
        }
        else {
            LOG_D("Success!Set for Param Id: 0x%2x, Param Value : %ld",
                listOfParams[index].Id,
                listOfParams[index].value);
        }
    }

    phRangingParams_t inRangingParams;
    inRangingParams.deviceRole       = kUWB_DeviceRole_Initiator;
    inRangingParams.multiNodeMode    = kUWB_MultiNodeMode_OnetoMany;
    inRangingParams.noOfControlees   = RANGING_APP_NO_OF_ANCHORS_P2P;
    inRangingParams.deviceType       = kUWB_DeviceType_Controller;
    inRangingParams.macAddrMode      = RANGING_APP_DEVICE_MAC_ADD_MODE_SHORT;
    inRangingParams.deviceMacAddr[0] = DEVICE_MAC_ADDR_SHORT[0];
    inRangingParams.deviceMacAddr[1] = DEVICE_MAC_ADDR_SHORT[1];

    inRangingParams.dstMacAddr[0] = DST_MAC_ADDR_SHORT[0];
    inRangingParams.dstMacAddr[1] = DST_MAC_ADDR_SHORT[1];

    status = UwbApi_SetRangingParams(RANGING_APP_SESSION_ID, &inRangingParams);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SetRangingParams() Failed");
        goto exit;
    }

    phRangingParams_t outRangingParams;
    status = UwbApi_GetRangingParams(RANGING_APP_SESSION_ID, &outRangingParams);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_GetRangingParams() Failed");
        goto exit;
    }

    /* Before ranging, store the session in NVM */
    status = UwbApi_SessionNvmManage(SESSION_NVM_MANAGE_PERSIST, RANGING_APP_SESSION_ID);
    if (status != UWBAPI_STATUS_OK) {
        LOG_E("UwbApi_SessionNvmManage() Failed");
        goto exit;
    }

    status = UwbApi_StartRangingSession(RANGING_APP_SESSION_ID);
    if (status == UWBAPI_STATUS_OK) {
        /* Fine */
    }
    else {
        LOG_E("UwbApi_StartRangingSession() Failed. status=%X", status);
        goto exit;
    }

#define SLEEP_DURATION_MS (5 * 60 * 1000)

    LOG_I("Started sending Blink Packets for  %ds", SLEEP_DURATION_MS / 1000);

    phOsalUwb_Delay(SLEEP_DURATION_MS);

    LOG_I("Delete created session from NVM.  (Only for DEMO)");
    /* Before deinit, clear the session in NVM */
    status = UwbApi_SessionNvmManage(SESSION_NVM_MANAGE_DELETE_ALL, 0);
    if (status != UWBAPI_STATUS_OK) {
        LOG_D("UwbApi_SessionNvmManage() Failed");
    }

    LOG_I("Put device to HPD");
    status = UwbApi_SuspendDevice();
    if (status == UWBAPI_STATUS_OK) {
        /* OK */
    }
    else if (status == UWBAPI_STATUS_HPD_WAKEUP) {
        status = UwbApi_SuspendDevice();
    }
    else {
        LOG_E("UwbApi_SuspendDevice() Failed");
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
            LOG_E("UwbApi_ShutDown() Failed");
        }
        else
            goto restart;
    }

    UWBIOT_EXAMPLE_END(status);
}

#endif // UWB_BUILD_STANDALONE_DEFAULT
