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

tUWBAPI_STATUS do_CONTINUOUS_WAVE(void)
{
    tUWBAPI_STATUS status;
    /* Start continous wave tone
     * The channel is set from demo_test_modes.c */
    status = UwbApi_StartTestMode(CONTINUOUS_WAVE_MODE, NULL);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StartTestMode Failed");
        goto exit;
    }
    NXPLOG_APP_I("Sleeping for 5 seconds");
    vTaskDelay(pdMS_TO_TICKS(5 * 1000));
    status = UwbApi_StopTestMode();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StopTestMode Failed");
        goto exit;
    }
exit:
    return status;
}
