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

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("SR040 : Demo SR040 Session NVM DELETE ALL ");

    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

restart:

    /** Main function to perform Session NVM DELETE ALL */
    status = UwbApi_Init(AppCallback);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_Init Failed");
        goto exit;
    }

    /* Clear all the sessions in NVM */
    status = UwbApi_SessionNvmManage(SESSION_NVM_MANAGE_DELETE_ALL, 0);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_SessionNvmManage() Failed");
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
