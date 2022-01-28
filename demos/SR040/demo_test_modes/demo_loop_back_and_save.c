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

tUWBAPI_STATUS do_LOOP_BACK_AND_SAVE(void)
{
    tUWBAPI_STATUS status;

    LOG_I("Setting NBIC");

    UwbApi_SetAppConfig(SESSION_ID_RFTEST, SR040_NBIC_CONF, 0x1);

    LOG_I("Starting Test Mode:: LOOP_BACK_MODE_NVM_AND_SAVE");
    status = UwbApi_StartTestMode(LOOP_BACK_MODE_NVM_AND_SAVE, NULL);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StartTestMode Failed");
        goto exit;
    }
    vTaskDelay(pdMS_TO_TICKS(300));

    LOG_I("Stopping Test Mode");
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
