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
#include "ApplMain.h"
#include "TLV_Builder.h"

// void BleInit(void *args);

/********************************************************************************/

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("SR040 : Demo BLE Tracker");

    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    tlvMngInit();
    tlvBuilderInit();

    /*
     * Initialization for SR040 1st time bootup in liftime
     */
    status = UwbApi_Init(AppCallback);
    if (status == UWBAPI_STATUS_OK) {
        UwbApi_SuspendDevice();
        UwbApi_ShutDown();
    }
    else {
        Log("SR040 bootup Initialization Failed");
        UwbApi_ShutDown();
        UWBIOT_EXAMPLE_END(status);
    }

    // FIXME
    // Check if value 0 is valid. Previous value was args. Declaration expects uint32_t.
    // In MK project, they declare it in this file as void BleInit(void *args);
    main_task(0);
}
