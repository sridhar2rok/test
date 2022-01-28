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

tUWBAPI_STATUS do_TX_ONLY(UWB_SR040_TxRxSlotType_t slotType, UINT8 psdu_length)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    LOG_I("DO:%s", __FUNCTION__);

    /* The frame that we want to send */
    UINT8 psduData[127];
    for (size_t i = 0; i < sizeof(psduData); i++) {
        psduData[i] = (uint8_t)i + 1;
    }

    if (psdu_length > sizeof(psduData)) {
        LOG_E("psdu_length=%d > sizeof(psduData)=%d", psdu_length, sizeof(psduData));
        goto exit;
    }

    phTestModeParams txParams;
    /* How many frames do we want to send?
     *
     * - 0 => Infinite
     * - Anyting else => Send that much frames
     */
    txParams.transmitModeParams.commonParams.eventCounterMax = 1 * 1000;
    /* Whether we are sending SP0, SP1 or SP3 frames? */
    txParams.transmitModeParams.commonParams.slotType = slotType;
    /* Dealy between each frames, in micro seconds
     *
     * Keep multiple of 1000us
     */
    txParams.transmitModeParams.txCycleTime = 1 * 1000;
    /* Frame that we want to send */
    txParams.transmitModeParams.psduData = psduData;
    /* Lenght of the frame */
    txParams.transmitModeParams.psduLen = psdu_length;
    /* Enable log notifications? */
    txParams.transmitModeParams.enableLogging = 1;
    /* Do we want a delay before each Tx? */
    txParams.transmitModeParams.commonParams.startDelay = 0;

    /* Start the transmit mode, with the parameters set above */
    status = UwbApi_StartTestMode(TRANSMIT_MODE, &txParams);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_StartTestMode Failed");
        goto exit;
    }

    if (txParams.transmitModeParams.commonParams.eventCounterMax != 0) {
        /* Wait for some time, before stopping the test session */
        vTaskDelay(pdMS_TO_TICKS(200 + (txParams.transmitModeParams.txCycleTime *
                                           txParams.transmitModeParams.commonParams.eventCounterMax * 1.05 / 1000)));
    }
    else {
        /* wait 5 minutes */
        vTaskDelay(pdMS_TO_TICKS(5 * 60 * 1000));
    }

    /* Stop the test session */
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
