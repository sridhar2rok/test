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
#include "demo_test_modes.h"
#include "demo_rx_per.h"

#if (DO_TEST_MODE_CONTINUOUS_WAVE + DO_TEST_MODE_LOOP_BACK + DO_TEST_MODE_TX_ONLY + DO_TEST_MODE_RX_ONLY + \
     DO_TEST_MODE_LOOP_BACK_AND_SAVE) == 0
#if !defined(DO_TEST_MODE_LOOP_ANY)
#define DO_TEST_MODE_LOOP_ANY 1
#endif // ! DO_TEST_MODE_LOOP_ANY
#endif

/* Configuration for RX and TX Modes:start */

/* Enable it here, set values inside demo_enable_hprf.c */
#define ENABLE_HPRF 0

/* Enable it here, set values inside demo_enable_phylogs.c */
#define ENABLE_PHYLOGS 1

/* SP0 or SP3 frame */
#define DEMO_TEST_MODE_TxRxSlotType kUWB_SR040_TxRxSlotType_SP3

/* Enable it here, set values inside demo_enable_ddfs.c */
#define ENABLE_DDFS 0

/* Configuration for RX and TX Modes:end */

#if defined(DO_TEST_MODE_LOOP_ANY)
/* doc-configure-demo-test-mode:start */
#define DO_TEST_MODE_CONTINUOUS_WAVE    0
#define DO_TEST_MODE_LOOP_BACK          0
#define DO_TEST_MODE_LOOP_BACK_AND_SAVE 0
#define DO_TEST_MODE_TX_ONLY            1
#define DO_TEST_MODE_RX_ONLY            0
/* doc-configure-demo-test-mode:end */
#endif

/* Enable either of these */
#if (DO_TEST_MODE_CONTINUOUS_WAVE + DO_TEST_MODE_LOOP_BACK + DO_TEST_MODE_TX_ONLY + DO_TEST_MODE_RX_ONLY + \
     DO_TEST_MODE_LOOP_BACK_AND_SAVE) > 1
#error "Enable only one"
#endif

#if (DO_TEST_MODE_CONTINUOUS_WAVE + DO_TEST_MODE_LOOP_BACK + DO_TEST_MODE_TX_ONLY + DO_TEST_MODE_RX_ONLY + \
     DO_TEST_MODE_LOOP_BACK_AND_SAVE) == 0
#error "Enable one"
#endif

/*
 * Below list contains the application configs which are only related to default configuration.
 */
OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("SR040 : Demo Test Modes");

    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    /* Select the channel ID we want to use */
    UINT8 channelId = 9;

    /* Select what power we want to send at.
     * - 0 => 14dbm MAX
     * - 104 => -12dbm Minimum
     */
    UINT8 txPower = 24;

    /*  [9-24]:BPRF, [25-32]:HPRF
     *
     * 10 default in Helios Test modes
     */
    UINT8 preambleCodeIndex = 10;

    /*  [0,2]:BPRF, [1,3]:HPRF */
    UINT8 sfdId = 2;

restart:
    /* Initialize the UWB Middleware */
    status = UwbApi_Init(demo_rx_per_AppCallback);
    if (status != UWBAPI_STATUS_OK) {
        Log("UwbApi_Init Failed");
        goto exit;
    }

#if ENABLE_PHYLOGS
    /* If needed, enable PHY Modes. Applicable for TX/Rx */
    status = do_EnablePhyLogs(DEMO_TEST_MODE_TxRxSlotType);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("do_EnablePhyLogs Failed");
        goto exit;
    }
#endif

    LOG_I("DO:UwbApi_SessionInit(SESSION_ID_RFTEST)");
    /* Create a Test Session */
    status = UwbApi_SessionInit(SESSION_ID_RFTEST, UWBD_RFTEST);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SessionInit Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfig(SESSION_ID_RFTEST, CHANNEL_NUMBER, channelId);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig::CHANNEL_NUMBER Failed");
        goto exit;
    }

    /* Helios Default */
    status = UwbApi_SetAppConfig(SESSION_ID_RFTEST, SFD_ID, sfdId);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig::SFD_ID Failed");
        goto exit;
    }

    /* Helios Default */
    status = UwbApi_SetAppConfig(SESSION_ID_RFTEST, PREAMBLE_CODE_INDEX, preambleCodeIndex);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig::PREAMBLE_CODE_INDEX Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfig(SESSION_ID_RFTEST, TX_POWER_ID, txPower);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig::TX_POWER_ID Failed");
        goto exit;
    }

    status = UwbApi_SetAppConfig(SESSION_ID_RFTEST, TX_ADAPTIVE_PAYLOAD_POWER, 0x0);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig::TX_ADAPTIVE_PAYLOAD_POWER Failed");
        goto exit;
    }

#if (DEMO_TEST_MODE_TxRxSlotType == kUWB_SR040_TxRxSlotType_SP3) || \
    (DEMO_TEST_MODE_TxRxSlotType == kUWB_SR040_TxRxSlotType_SP1)
    status = do_StoreSTSKeys();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("do_StoreSTSKeys Failed");
        goto exit;
    }
#endif

#if ENABLE_HPRF
    /* If needed, enable HPRF settings. Applicable for TX/Rx */
    status = do_EnableHPRFConfigs();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("do_EnableHPRFConfigs Failed");
        goto exit;
    }
#endif

#if ENABLE_DDFS
    /* If needed, enable DDFS settings. Applicable for TX/Rx */
    status = do_EnableDDFSConfigs();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("do_EnableDDFSConfigs Failed");
        goto exit;
    }
#endif

    /* start actual test mode / demo */
    LOG_I("Starting Test Mode");

#if DO_TEST_MODE_CONTINUOUS_WAVE
    status = do_CONTINUOUS_WAVE();
#endif // DO_TEST_MODE_CONTINUOUS_WAVE

#if DO_TEST_MODE_LOOP_BACK
    status = do_LOOP_BACK();
#endif // DO_TEST_MODE_LOOP_BACK

#if DO_TEST_MODE_LOOP_BACK_AND_SAVE
    status = do_LOOP_BACK_AND_SAVE();
#endif // DO_TEST_MODE_LOOP_BACK_AND_SAVE

#if DO_TEST_MODE_RX_ONLY
    status = do_RX_ONLY(DEMO_TEST_MODE_TxRxSlotType);
#endif // DO_TEST_MODE_RX_ONLY

#if DO_TEST_MODE_TX_ONLY
    /* Length is valid only for SP0 frames */
    status = do_TX_ONLY(DEMO_TEST_MODE_TxRxSlotType, 20);
#endif // DO_TEST_MODE_TX_ONLY

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

/* IEEE Keys */

/* clang-format off */
#define STS_KEY_D_URSK_ARRAY  \
    0, /*< plain */  \
    0x14, 0x14, 0x86, 0x74, 0xD1, 0xD3, 0x36, 0xAA, 0xF8, 0x60, 0x50, 0xA8, 0x14, 0xEB, 0x22, 0x0F

#define STS_SALTED_HASH_ARRAY   \
    0, /*< plain */ \
    0x36, 0x2E, 0xEB, 0x34, \
    0xC4, 0x4F, 0xA8, 0xFB, \
    0xD3, 0x7E, 0xC3, 0xCA, \
    0x1F, 0x9A, 0x3D, 0xE4

#define STS_INDEX_ARRAY   0x00, 0, 0, 0x00

#define SESESION_ID_ARRAY 0x00, 0x00, 0x00, 0x00
/* clang-format on */

tUWBAPI_STATUS do_StoreSTSKeys()
{
    tUWBAPI_STATUS status;

    /* clang-format off */
    uint8_t use_sts_keys[] = { 0x21,
        0x03,
        00,
        00, /* len */
        SESESION_ID_ARRAY,
        3, /* N Parameters */
        /* 1:T */ UCI_PARAM_ID_STS_INDEX,
        /*  :L */ 4,
        /*  :V */ STS_INDEX_ARRAY,
        /* 2:T */ UCI_EXT_PARAM_ID_D_URSK,
        /*  :L */ UCI_EXT_PARAM_ID_D_URSK_LEN,
        /*  :V */ STS_KEY_D_URSK_ARRAY,
        /* 3:T */ UCI_EXT_PARAM_ID_SALTED_HASH,
        /*  :L */ UCI_EXT_PARAM_ID_SALTED_HASH_LEN,
        /*  :V */ STS_SALTED_HASH_ARRAY,
    };
    use_sts_keys[3] = sizeof(use_sts_keys) - 4;

    /* clang-format on */
    uint8_t rsp[10] = {0};
    uint16_t rspLen = sizeof(rsp);

    status = UwbApi_SendRawCommand((UINT8 *)use_sts_keys, sizeof(use_sts_keys), rsp, &rspLen);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SendRawCommand:use_sts_keys Failed");
        goto exit;
    }
exit:
    return status;
}
