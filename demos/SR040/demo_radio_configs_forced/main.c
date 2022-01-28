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

#include "phUwb_BuildConfig.h"

#include "UwbApi.h"
#include "RadioConfig.h"
#include <AppInternal.h>
#include "AppRecovery.h"

/*
* Below list contains the application configs which are only related to default configuration.
*/

/********************************************************************************/
/*               Ranging APP configuration setting here                         */
/********************************************************************************/
/********************************************************************************/

AppContext_t appContext;

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("SR040 : Radio Configs Update");

    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    status = UwbApi_Init(AppCallback);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_Init Failed");
        goto exit;
    }

    /* DO a forced update... over-write even if CRC Matches */
    status = RadioConfig_tdoaAndTracker(true);
    if (status != UWBAPI_STATUS_OK) {
        Log("RadioConfig_tdoaAndTracker Failed");
        goto exit;
    }

    Log("Done!");
exit:
    UWBIOT_EXAMPLE_END(status);
}
