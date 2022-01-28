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

/* Enable PHY Logs */

#define DEBUG_LOG_LEVEL     0xF3
#define RX_PHY_LOGGING_ENBL 0xF4
#define TX_PHY_LOGGING_ENBL 0xF5
#define LOG_PARAMS_CONF     0xF6

/* Select individual selections */

#define uLogB0_RX_STATUS_AND_ERROR (1 << 0)
#define uLogB0_TX_STATUS_AND_ERROR (1 << 1)
#define uLogB0_CIR_LOG             (1 << 2)
#define uLogB0_UWB_SESSION_ID      (1 << 3)
#define uLogB0_BLOCK_INDEX         (1 << 4)
#define uLogB0_RX_TIMESTAMP        (1 << 5)
#define uLogB0_TX_TIMESTAMP        (1 << 6)
#define uLogB0_RX_PSDU             (1 << 7)

#define uLogB1_TX_PSDU                     (1 << (8 - 8))
#define uLogB1_STS_INDEX                   (1 << (9 - 8))
#define uLogB1_RX_FIRST_PATH_INFO          (1 << (10 - 8))
#define uLogB1_RX_CARRIER_FREQUENCY_OFFSET (1 << (11 - 8))
#define uLogB1_RX_MACSTATUS                (1 << (12 - 8))
#define uLogB1_RX_MAC_DECODEFAILRSN        (1 << (13 - 8))

/* Select what we want to enable */

tUWBAPI_STATUS do_EnablePhyLogs(UWB_SR040_TxRxSlotType_t slotType)
{
    UINT8 cmd[255] = {0x20, 0x04, 0x00};
    UINT8 rsp[100];
    UINT16 respLen = sizeof(rsp);
    UINT16 cmdLen;
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    LOG_I("DO:%s", __FUNCTION__);

    const uint8_t LOG_PARAMS_CONF_VAL_B0 = /* */
        0                                  /* */
        | uLogB0_RX_STATUS_AND_ERROR       /* */
        //| uLogB0_TX_STATUS_AND_ERROR       /* */
        //| uLogB0_CIR_LOG                   /* */
        //| uLogB0_UWB_SESSION_ID            /* */
        //| uLogB0_BLOCK_INDEX               /* */
        | uLogB0_RX_TIMESTAMP /* */
        //| uLogB0_TX_TIMESTAMP              /* */
        //| uLogB0_UWB_SESSION_ID            /* */
        | (kUWB_SR040_TxRxSlotType_SP0 == slotType ? uLogB0_RX_PSDU : 0) /* */
        ;

    const uint8_t LOG_PARAMS_CONF_VAL_B1 = /* */
        0                                  /* */
        // | uLogB1_TX_PSDU                   /* */
        //| (kUWB_SR040_TxRxSlotType_SP3 == slotType ? uLogB1_STS_INDEX : 0)            /* */
        | (kUWB_SR040_TxRxSlotType_SP3 == slotType ? uLogB1_RX_FIRST_PATH_INFO : 0)   /* */
        | uLogB1_RX_CARRIER_FREQUENCY_OFFSET                                          /* */
        | (kUWB_SR040_TxRxSlotType_SP0 == slotType ? uLogB1_RX_MACSTATUS : 0)         /* */
        | (kUWB_SR040_TxRxSlotType_SP0 == slotType ? uLogB1_RX_MAC_DECODEFAILRSN : 0) /* */
        ;

    uint8_t enTxRxPhy[] = {
        4 /* No of Params */,
        DEBUG_LOG_LEVEL,
        1 /* len : DEBUG_LOG_LEVEL */,
        1 /* Enable : DEBUG_LOG_LEVEL */,
        RX_PHY_LOGGING_ENBL,
        1 /* len : RX_PHY_LOGGING_ENBL */,
        1 /* enable : RX_PHY_LOGGING_ENBL */,
        TX_PHY_LOGGING_ENBL,
        1 /* len : TX_PHY_LOGGING_ENBL */,
        1 /* enable  : TX_PHY_LOGGING_ENBL */,
        LOG_PARAMS_CONF,
        4 /* len : LOG_PARAMS_CONF */,
        LOG_PARAMS_CONF_VAL_B0,
        LOG_PARAMS_CONF_VAL_B1,
        0x00,
        0X00,
    };
    cmd[3] = sizeof(enTxRxPhy);
    cmdLen = cmd[3] + 4; /* payload + UCI Header length*/
    phOsalUwb_MemCopy(&cmd[4], enTxRxPhy, sizeof(enTxRxPhy));

    status = UwbApi_SendRawCommand(cmd, cmdLen, rsp, &respLen);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UWB_SetCoreConfig(Enable TxRxPhy) Failed");
    }
    return status;
}
