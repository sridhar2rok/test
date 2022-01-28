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
#include <SwupPkgDSP.h>
#include "SwupUCI.h"
#include <Swup_update.h>

#include <phNxpUwb_SpiTransport.h>

/********************************************************************************/
/* Definitions                                                                  */
/********************************************************************************/

#include <phNxpUwb_DriverInterface.h>

tUWBAPI_STATUS doSwupUpdate(void)
{
    tUWBAPI_STATUS status  = UWBAPI_STATUS_FAILED;
    uint32_t dspVersion    = 0;
    phUwbDevInfo_t devInfo = {0};
    UwbApi_Init(AppCallback);
    status = UwbApi_GetStackCapabilities(&devInfo);
    if (status == UWBAPI_STATUS_OK) {
        dspVersion = devInfo.dspMajor << 8 * 2 | devInfo.dspMinor << 8 | devInfo.dspPatchVersion;
    }
    else {
        dspVersion = 0xFFFFFF;
    }

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
    if (dspVersion != 0xFFFFFF) {
        /* Device is in UCI mode */
        if (dspVersion == MAC_RANGING_DSP_VERSION) {
            NXPLOG_APP_W("Same package version found, exiting");
            goto exit;
        }
        else {
            /* Device is in UCI mode. Enable SWUP and move forward */
            status = Uci_EnableSwup();
            phOsalUwb_Delay(500);
            if (status != UWBAPI_STATUS_OK) {
                /* Enable SWUP failed. Device is in UCI mode. Exit */
                goto exit;
            }
        }
    }
    /* If code reaches here, device is in SWUP mode or dspVersion failed */
    SwupDeviceId_t swupDeviceId;
    phNxpUwb_SwitchProtocol(kInterfaceModeSwup);
    if (Swup_ReadDeviceId(&swupDeviceId) != STATUS_CMD_SUCCESS) {
        /* Not in SWUP. Exit. */
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }

    if (STATUS_CMD_SUCCESS != SwupUpdate(MAC_RANGING_DSP_PKG, MAC_RANGING_DSP_LEN, kSWUP_KEY_VESRION_UNKNOWN)) {
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
    else {
        status = UWBAPI_STATUS_OK;
    }

exit:
    phOsalUwb_Delay(2000);
    phNxpUwb_SwitchProtocol(kInterfaceModeUci);
    /** De-initialize transport layer
     *  Initialization should be taken care in
     *  UWB stack init or the application layer (in case of failure)
     */
    phNxpUwb_HeliosDeInit();
    return status;
}
