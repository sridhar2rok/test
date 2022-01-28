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
#include "demo_rx_per.h"

/* The frame that we want to receive */
UINT8 expectedPSDUData[64];

static tUWBAPI_STATUS do_SetPowerSavingStatus(const uint8_t val)
{
    LOG_I("DO:%s", __FUNCTION__);
    UINT8 powerSaveCoreCMD[]   = {0x20, //gid of core_set_config_cmd
        0x04,                         //oid of core_set_config_cmd
        0x00,                         //rfu
        0x04,                         //payloadlength
        0x01,                         //noofparameters
        UCI_PARAM_ID_LOW_POWER_MODE,
        0x01, //length of power mode
        val};
    UINT8 powerSaveCoreRSP[10] = {0};
    UINT16 length              = sizeof(powerSaveCoreRSP);

    tUWBAPI_STATUS status =
        UwbApi_SendRawCommand(powerSaveCoreCMD, sizeof(powerSaveCoreCMD), powerSaveCoreRSP, &length);
    return status;
}

tUWBAPI_STATUS do_RX_ONLY(UWB_SR040_TxRxSlotType_t slotType)
{
    tUWBAPI_STATUS status;

    for (size_t i = 0; i < sizeof(expectedPSDUData); i++) {
        expectedPSDUData[i] = (uint8_t)i + 1;
    }
    //expectedPSDUData[sizeof(expectedPSDUData) - 2] = 0x92;
    //expectedPSDUData[sizeof(expectedPSDUData) - 1] = 0x82;

    phTestModeParams rxParams;
    /* Abort after receiving these many frames */
    rxParams.receiveModeParams.commonParams.eventCounterMax = 1000;
    /* Delay before RX */
    rxParams.receiveModeParams.commonParams.startDelay = 0;
    /* Whether SP0 or SP3 frames? */
    rxParams.receiveModeParams.commonParams.slotType = slotType;
    /* Timeout for frame reception */
    rxParams.receiveModeParams.timeOut = 60 * 1000;
    /* Enable logs? */
    rxParams.receiveModeParams.enableLogging = 1;

    demo_rx_per_Init(slotType, expectedPSDUData, sizeof(expectedPSDUData));

    int wait_ms                = (rxParams.receiveModeParams.commonParams.eventCounterMax) * 1000;
    int last_pending_ntfs      = rxParams.receiveModeParams.commonParams.eventCounterMax;
    const int max_pending_ntfs = rxParams.receiveModeParams.commonParams.eventCounterMax;
    int last_wait_ms           = wait_ms;

restart_test_mode:
    LOG_I("DO:%s", "UwbApi_StartTestMode()");
    /* Start the test mode to receive frames as set by rxParams */
    status = UwbApi_StartTestMode(RECEIVE_MODE, &rxParams);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StartTestMode Failed");
        goto exit;
    }

    status = do_SetPowerSavingStatus(0);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("do_SetPowerSavingStatus(0) Failed");
        goto exit;
    }

    /* Wait */
    vTaskDelay(pdMS_TO_TICKS(200));
    do {
        const int pending_ntfs = max_pending_ntfs - demo_rx_per_GetNtfsCount();
        /* Wait for some time before stopping the test mode */
        if (pending_ntfs <= 0) {
            /* abort if we have more or expected notifications */
            break;
        }
        else if (pending_ntfs == (int)rxParams.receiveModeParams.commonParams.eventCounterMax) {
            /* We did not receive anything yet */
            break;
        }
        else {
            LOG_I("NTFs Pending=%d Got=%d wait_ms=%d", pending_ntfs, demo_rx_per_GetNtfsCount(), wait_ms);
            vTaskDelay(pdMS_TO_TICKS(500));
            wait_ms -= 500;
            if (pending_ntfs < 2 && wait_ms > (pending_ntfs * rxParams.receiveModeParams.timeOut)) {
                /* For last ntf, don't wait way too long */
                wait_ms = pending_ntfs * rxParams.receiveModeParams.timeOut / 60;
            }
        }
        if (pending_ntfs == last_pending_ntfs && last_wait_ms != wait_ms) {
            rxParams.receiveModeParams.commonParams.eventCounterMax = pending_ntfs;
            LOG_W("Continue Test Mode");
            goto restart_test_mode;
        }
        last_pending_ntfs = pending_ntfs;
        last_wait_ms      = wait_ms;
    } while (wait_ms > 0);

    if (demo_rx_per_GetNtfsCount() >= rxParams.receiveModeParams.commonParams.eventCounterMax) {
        /* OK */
    }
    else {
        LOG_I("All not not yet received. Waiting for 2 seconds");
        vTaskDelay(pdMS_TO_TICKS(1000 * 2));
    }
    LOG_I("Stopping RX Mode");
    /* Stop the test mode */
    status = UwbApi_StopTestMode();
    if (rxParams.receiveModeParams.timeOut != 0 && status == UWBAPI_STATUS_HPD_WAKEUP) {
        /* It's OK, don't keep re-running */
        status = UWBAPI_STATUS_OK;
    }

    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StopTestMode Failed");
        goto exit;
    }

    status = do_SetPowerSavingStatus(1);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("do_SetPowerSavingStatus(1) Failed");
        // May fail.
        //goto exit;
    }

    demo_rx_per_PrintSummary(slotType);

exit:
    return status;
}
