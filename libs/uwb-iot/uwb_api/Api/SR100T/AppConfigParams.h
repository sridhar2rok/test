/*
 *
 * Copyright 2018-2020 NXP.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 *
 */

#ifndef _APP_CONFIG_PARAMS_
#define _APP_CONFIG_PARAMS_

#include "UwbApi.h"
#include "uci_defs.h"
#include "uci_ext_defs.h"
#include "uci_test_defs.h"

const UINT8 uciRfTest_AppParamIds[] = {
    /* Application Configuration Parameters */
    UCI_PARAM_ID_NO_OF_CONTROLEES,   /* NO_OF_CONTROLEES             */
    UCI_PARAM_ID_DEVICE_MAC_ADDRESS, /* DEVICE_MAC_ADDRESS           */
    UCI_PARAM_ID_DST_MAC_ADDRESS,    /* DST_MAC_ADDRESS              */
    UCI_PARAM_ID_MAC_CFG,            /* MAC_CFG                      */
};

const UINT8 uciRfTest_TestParamIds[] = {
    /* RF Test Parameters */
    UCI_TEST_PARAM_ID_NUM_PACKETS,         /*NUM_OF_PACKETS         */
    UCI_TEST_PARAM_ID_T_GAP,               /*T_GAP                  */
    UCI_TEST_PARAM_ID_T_START,             /*T_START                */
    UCI_TEST_PARAM_ID_T_WIN,               /*T_WIN                  */
    UCI_TEST_PARAM_ID_RANDOMIZE_PSDU,      /*RANDOMIZE_PSDU         */
    UCI_TEST_PARAM_ID_RAW_PHR,             /*RAW PHR                */
    UCI_TEST_PARAM_ID_RMARKER_RX_START,    /*RX START               */
    UCI_TEST_PARAM_ID_RMARKER_TX_START,    /*TX_START               */
    UCI_TEST_PARAM_ID_STS_INDEX_AUTO_INCR, /*STS INDEX AUTO INCR    */
};

const UINT16 uciDebugParamIds[] = {
    ((EXTENDED_DEBUG_CONFIG_ID << 8) | UCI_EXT_PARAM_ID_THREAD_SECURE),         /*THREAD_SECURE          */
    ((EXTENDED_DEBUG_CONFIG_ID << 8) | UCI_EXT_PARAM_ID_THREAD_SECURE_ISR),     /*THREAD_SECURE_ISR      */
    ((EXTENDED_DEBUG_CONFIG_ID << 8) | UCI_EXT_PARAM_ID_THREAD_NON_SECURE_ISR), /*THREAD_NON_SECURE_ISR  */
    ((EXTENDED_DEBUG_CONFIG_ID << 8) | UCI_EXT_PARAM_ID_THREAD_SHELL),          /*THREAD_SHELL           */
    ((EXTENDED_DEBUG_CONFIG_ID << 8) | UCI_EXT_PARAM_ID_THREAD_PHY),            /*THREAD_PHY             */
    ((EXTENDED_DEBUG_CONFIG_ID << 8) | UCI_EXT_PARAM_ID_THREAD_RANGING),        /*THREAD_RANGING         */
    ((EXTENDED_DEBUG_CONFIG_ID << 8) | UCI_EXT_PARAM_ID_DATA_LOGGER_NTF),       /*DATA_LOGGER_NTF        */
    ((EXTENDED_DEBUG_CONFIG_ID << 8) | UCI_EXT_PARAM_ID_CIR_LOG_NTF),           /*CIR_LOG_NTF            */
    ((EXTENDED_DEBUG_CONFIG_ID << 8) | UCI_EXT_PARAM_ID_PSDU_LOG_NTF),          /* PSDU_LOG_NTF          */
    ((EXTENDED_DEBUG_CONFIG_ID << 8) | UCI_EXT_PARAM_ID_RFRAME_LOG_NTF)         /* RFRAME_LOG_NTF        */
};

const UINT8 uciRangingParamIds[] = {
    /* Application Configuration Parameters */
    UCI_PARAM_ID_DEVICE_ROLE,        /* DEVICE_ROLE                  */
    UCI_PARAM_ID_MULTI_NODE_MODE,    /* MULTI_NODE_MODE              */
    UCI_PARAM_ID_MAC_ADDRESS_MODE,   /* MAC_ADDR_MODE                */
    UCI_PARAM_ID_NO_OF_CONTROLEES,   /* NO_OF_CONTROLEES             */
    UCI_PARAM_ID_DEVICE_MAC_ADDRESS, /* DEVICE_MAC_ADDRESS           */
    UCI_PARAM_ID_DST_MAC_ADDRESS,    /* DST_MAC_ADDRESS              */
    UCI_PARAM_ID_DEVICE_TYPE,        /* DEVICE_TYPE                  */
};

const UINT8 app_config_mapping[] = {UCI_PARAM_ID_RANGING_METHOD,
    UCI_PARAM_ID_STS_CONFIG,
    UCI_PARAM_ID_CHANNEL_NUMBER,
    UCI_PARAM_ID_SLOT_DURATION,
    UCI_PARAM_ID_RANGING_INTERVAL,
    UCI_PARAM_ID_STS_INDEX,
    UCI_PARAM_ID_MAC_FCS_TYPE,
    UCI_PARAM_ID_RANGING_ROUND_PHASE_CONTROL,
    UCI_PARAM_ID_AOA_RESULT_REQ,
    UCI_PARAM_ID_RNG_DATA_NTF,
    UCI_PARAM_ID_RNG_DATA_NTF_PROXIMITY_NEAR,
    UCI_PARAM_ID_RNG_DATA_NTF_PROXIMITY_FAR,
    UCI_PARAM_ID_RFRAME_CONFIG,
    UCI_PARAM_ID_RX_MODE,
    UCI_PARAM_ID_PREAMBLE_CODE_INDEX,
    UCI_PARAM_ID_SFD_ID,
    UCI_PARAM_ID_PSDU_DATA_RATE,
    UCI_PARAM_ID_PREAMBLE_DURATION,
    UCI_PARAM_ID_RX_ANTENNA_PAIR_SELECTION,
    UCI_PARAM_ID_MAC_CFG,
    UCI_PARAM_ID_RANGING_TIME_STRUCT,
    UCI_PARAM_ID_SLOTS_PER_RR,
    UCI_PARAM_ID_TX_ADAPTIVE_PAYLOAD_POWER,
    UCI_PARAM_ID_TX_ANTENNA_SELECTION,
    UCI_PARAM_ID_RESPONDER_SLOT_INDEX,
    UCI_PARAM_ID_PRF_MODE,
    UCI_PARAM_ID_MAX_CONTENTION_PHASE_LEN,
    UCI_PARAM_ID_CONTENTION_PHASE_UPDATE_LEN,
    UCI_PARAM_ID_SCHEDULED_MODE,
    UCI_PARAM_ID_KEY_ROTATION,
    UCI_PARAM_ID_KEY_ROTATION_RATE,
    UCI_PARAM_ID_SESSION_PRIORITY,
    UCI_PARAM_ID_MAC_ADDRESS_MODE,
    UCI_PARAM_ID_NUMBER_OF_STS_SEGMENTS,
    UCI_PARAM_ID_MAX_RR_RETRY,
    UCI_PARAM_ID_UWB_INITIATION_TIME,
    UCI_PARAM_ID_RANGING_ROUND_HOPPING,
    UCI_PARAM_ID_IN_BAND_TERMINATION_ATTEMPT_COUNT,
    UCI_PARAM_ID_SUB_SESSION_ID,
    UCI_PARAM_ID_BLINK_RANDOM_INTERVAL,
    UCI_PARAM_ID_RESULT_REPORT_CONFIG,
    UCI_PARAM_ID_MAX_NUMBER_OF_BLOCKS};

const UINT16 ext_app_config_mapping[] = {
    ((UCI_EXT_PARAM_ID_TOA_MODE << 8) | EXTENDED_APP_CONFIG_ID),
    ((UCI_EXT_PARAM_ID_CIR_CAPTURE_MODE << 8) | EXTENDED_APP_CONFIG_ID),
    ((UCI_EXT_PARAM_ID_MAC_PAYLOAD_ENCRYPTION << 8) | EXTENDED_APP_CONFIG_ID),
    ((UCI_EXT_PARAM_ID_RFU_1 << 8) | EXTENDED_APP_CONFIG_ID),
    ((UCI_EXT_PARAM_ID_RFU_2 << 8) | EXTENDED_APP_CONFIG_ID),
    ((UCI_EXT_PARAM_ID_SESSION_SYNC_ATTEMPTS << 8) | EXTENDED_APP_CONFIG_ID),
    ((UCI_EXT_PARAM_ID_SESSION_SCHED_ATTEMPTS << 8) | EXTENDED_APP_CONFIG_ID),
    ((UCI_EXT_PARAM_ID_SCHED_STATUS_NTF << 8) | EXTENDED_APP_CONFIG_ID),
    ((UCI_EXT_PARAM_ID_TX_POWER_DELTA_FCC << 8) | EXTENDED_APP_CONFIG_ID),
};

const UINT16 ext_test_config_mapping[] = {
    ((UCI_EXT_TEST_PARAM_ID_RSSI_AVG_FILT_CNT << 8) | EXTENDED_TEST_CONFIG_ID),
    ((UCI_EXT_TEST_PARAM_ID_RSSI_CALIBRATION_OPTION << 8) | EXTENDED_TEST_CONFIG_ID),
    ((UCI_EXT_TEST_PARAM_ID_AGC_GAIN_VAL_RX << 8) | EXTENDED_TEST_CONFIG_ID),
};

#endif
