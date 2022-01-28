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

#include <AppInternal.h>
#include <SwupApi.h>
#include "SwupUCI.h"
#include <Swup_update.h>

#include <phNxpUwb_SpiTransport.h>

/********************************************************************************/
/* Definitions                                                                  */
/********************************************************************************/

#include <phNxpUwb_DriverInterface.h>

tUWBAPI_STATUS doSwupInfo(void)
{
    tUWBAPI_STATUS statusGetStackCapabilities = UWBAPI_STATUS_FAILED;
    tUWBAPI_STATUS status;
    phUwbDevInfo_t devInfo = {0};
    UwbApi_Init(AppCallback);
    statusGetStackCapabilities = UwbApi_GetStackCapabilities(&devInfo);

    /** Shutdown UCI stack and open only
     *  the transport layer.
     */
    UwbApi_ShutDown();

    /* Initialize transport layer to send raw commands */
    phNxpUwb_HeliosInit();

    phNxpUwb_HeliosReset();

    /** Check if Device has SWUP enabled.
     *  Send GetDeviceInfo command.
     *  If this function fails, the device is in
     *  UCI mode. Send raw SwupActivate command
     *  to activate SWUP mode.
     */

    phNxpUwb_SwitchProtocol(kInterfaceModeUci);
    if (UWBAPI_STATUS_OK == statusGetStackCapabilities) {
        /* Device is in UCI mode. Enable SWUP and move forward */
        status = Uci_EnableSwup();
        phOsalUwb_Delay(500);
        if (status != UWBAPI_STATUS_OK) {
            /* Enable SWUP failed. Device is in UCI mode. Exit */
            goto exit;
        }
    }
    else {
        LOG_W("Assuming device in SWUP Mode already.");
    }
    /* If code reaches here, device is in SWUP mode or dspVersion failed */
    SwupDeviceId_t swupDeviceId;
    SwupDeviceInfo_t swupDeviceInfo;
    phNxpUwb_SwitchProtocol(kInterfaceModeSwup);
    if (Swup_GetDeviceInfo(&swupDeviceInfo) == STATUS_CMD_SUCCESS) {
        // LOG_X32_I(swupDeviceInfo.hardwareId);
        // LOG_AU8_I(swupDeviceInfo.productId, sizeof(swupDeviceInfo.productId));
        // LOG_AU8_I(swupDeviceInfo.romId, sizeof(swupDeviceInfo.romId));
        LOG_AU8_I(swupDeviceInfo.swupVersion, sizeof(swupDeviceInfo.swupVersion));
        LOG_AU8_I(swupDeviceInfo.typeCheckId, sizeof(swupDeviceInfo.typeCheckId));
        status = UWBAPI_STATUS_OK;
    }
    else {
        /* Not in SWUP. Exit. */
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }

    if (Swup_ReadDeviceId(&swupDeviceId) == STATUS_CMD_SUCCESS) {
        LOG_X32_I(swupDeviceId.serialNumber);
        LOG_X16_I(swupDeviceId.waferCoordinateX);
        LOG_X16_I(swupDeviceId.waferCoordinateY);
        // LOG_AU8_I(swupDeviceId.waferId, sizeof(swupDeviceId.waferId));
        // LOG_X16_I(swupDeviceId.waferNumber);
        status = UWBAPI_STATUS_OK;
    }
    else {
        /* Not in SWUP. Exit. */
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }

exit:
    phNxpUwb_SwitchProtocol(kInterfaceModeUci);
    /** De-initialize transport layer
     *  Initialization should be taken care in
     *  UWB stack init or the application layer (in case of failure)
     */
    phNxpUwb_HeliosDeInit();
    return status;
}
