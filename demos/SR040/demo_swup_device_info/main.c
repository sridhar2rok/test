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

#include "phUwb_BuildConfig.h"

#include "UwbApi.h"
#include <AppInternal.h>
#include "AppRecovery.h"
#include "SwupUCI.h"
#include "phNxpUwb_SpiTransport.h"

/********************************************************************************/
extern tUWBAPI_STATUS doSwupInfo(void);

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("SR040 : Demo SWUP Info");

    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    /** Main function to perform SWUP */
    status = doSwupInfo();

    UwbApi_ShutDown();
    NXPLOG_APP_I("Execution completed");
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Application failed");
    }
    else {
        NXPLOG_APP_I("Application passed");
    }

    UWBIOT_EXAMPLE_END(status);
}
