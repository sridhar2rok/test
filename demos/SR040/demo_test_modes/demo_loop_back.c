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

#include "demo_test_modes.h"
#include <phNxpLogApis_App.h>
#include <AppInternal.h>
#include "AppRecovery.h"
#include "demo_test_modes.h"

tUWBAPI_STATUS do_LOOP_BACK(void)
{
    tUWBAPI_STATUS status;

    PRINTF("Starting Test Mode\n");

    UwbApi_SetAppConfig(SESSION_ID_RFTEST, SR040_NBIC_CONF, 0x0);

    status = UwbApi_StartTestMode(LOOP_BACK_MODE, NULL);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StartTestMode Failed");
        goto exit;
    }
    /* When Loopback test notification is received, this semaphore is signalled */
    if (phOsalUwb_ConsumeSemaphore_WithTimeout(testLoopBackNtfSem, 200)) {
        NXPLOG_APP_I("testLoopBack Received");
        status = UWBAPI_STATUS_OK;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    PRINTF("Stopping Test Mode\n");
    status = UwbApi_StopTestMode();
    if (status == UWBAPI_STATUS_HPD_WAKEUP) {
        status = UWBAPI_STATUS_OK;
    }
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StopTestMode Failed");
        goto exit;
    }

exit:
    return status;
}
