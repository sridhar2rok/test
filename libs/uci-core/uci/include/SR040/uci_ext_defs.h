/*
*
* Copyright 2019-2020 NXP.
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

/******************************************************************************
 *
 *  This file contains the definition from UCI specification
 *
 ******************************************************************************/

#ifndef UWB_UCI_EXT_DEFS_H
#define UWB_UCI_EXT_DEFS_H

#include <stdint.h>

/**********************************************
 *     Device Information Parameters          *
 **********************************************/
#define UCI_EXT_PARAM_ID_DEVICE_NAME            0xE3
#define UCI_EXT_PARAM_ID_FIRMWARE_VERSION       0xE4
#define UCI_EXT_PARAM_ID_DEVICE_VERSION         0xE5
#define UCI_EXT_PARAM_ID_SERIAL_NUMBER          0xE6
#define UCI_EXT_PARAM_ID_DSP_VERSION            0xE7
#define UCI_EXT_PARAM_ID_RANGER4_VERSION        0xE8
#define UCI_EXT_PARAM_ID_CCC_VERSION            0xE9

#define UCI_EXT_PARAM_ID_DEVICE_NAME_LEN        0x08
#define UCI_EXT_PARAM_ID_FIRMWARE_VERSION_LEN   0x03
#define UCI_EXT_PARAM_ID_DEVICE_VERSION_LEN     0x02
#define UCI_EXT_PARAM_ID_SERIAL_NUMBER_LEN      0x08
#define UCI_EXT_PARAM_ID_DSP_VERSION_LEN        0x03
#define UCI_EXT_PARAM_ID_RANGER4_VERSION_LEN    0x02
#define UCI_EXT_PARAM_ID_CCC_VERSION_LEN        0x08

/* R4 Specific Commands */
#define EXT_UCI_MSG_LOG_NTF                            0x00
#define EXT_UCI_MSG_RADIO_CONFIG_DOWNLOAD_CMD          0x11
#define EXT_UCI_MSG_ACTIVATE_SWUP_CMD                  0x12
#define EXT_UCI_MSG_TEST_START_CMD                     0x20
#define EXT_UCI_MSG_TEST_STOP_CMD                      0x21
#define EXT_UCI_MSG_TEST_INITIATOR_RANGE_DATA_NTF      0x22
#define EXT_UCI_MSG_STACK_TEST_CMD                     0x23
#define EXT_UCI_MSG_DEVICE_SUSPEND_CMD                 0x24
#define EXT_UCI_MSG_TEST_LOOPBACK_NTF                  0x25
#define EXT_UCI_MSG_SET_TRIM_VALUES_CMD                0x26
#define EXT_UCI_MSG_GET_ALL_UWB_SESSIONS_CMD           0x27
#define EXT_UCI_MSG_SESSION_NVM_MANAGE_CMD             0x2B

/* Core Device Configurations */
#define UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT             0xE9
#define UCI_EXT_PARAM_ID_HPD_ENTRY_TIMEOUT             0xEA
#define UCI_EXT_PARAM_ID_MHR_IN_CCM                    0xF9
#define UCI_EXT_PARAM_ID_DDFS_TONE_CONFIG_ENABLE       0xFA
#define UCI_EXT_PARAM_ID_TELC_CONFIG                   0XFB

/* R4 Proprietary Application Configurations */
#define UCI_EXT_PARAM_ID_TX_MAX_BLOCK_NUM              0xA1
#define UCI_EXT_PARAM_ID_NBIC_CONF_ID                  0xE6
#define UCI_EXT_PARAM_ID_M_UPSK                        0xEB
#define UCI_EXT_PARAM_ID_SALTED_HASH                   0xEC
#define UCI_EXT_PARAM_ID_D_URSK                        0xED
#define UCI_EXT_PARAM_ID_D_UDSK                        0xEE
#define UCI_EXT_PARAM_ID_TX_POWER_ID                   0xF2
#define UCI_EXT_PARAM_ID_DEBUG_LOG_LEVEL               0xF3
#define UCI_EXT_PARAM_ID_RX_PHY_LOGGING_ENBL           0xF4
#define UCI_EXT_PARAM_ID_TX_PHY_LOGGING_ENBL           0xF5
#define UCI_EXT_PARAM_ID_LOG_PARAMS_CONF               0xF6
#define UCI_EXT_PARAM_ID_CIR_TAP_OFFSET                0xF7
#define UCI_EXT_PARAM_ID_CIR_NUM_TAPS                  0xF8
#define UCI_EXT_PARAM_ID_STS_INDEX_RESTART             0xF9
#define UCI_EXT_PARAM_ID_RX_RADIO_CFG_IDXS             0xFB
#define UCI_EXT_PARAM_ID_TX_RADIO_CFG_IDXS             0xFC
#define UCI_EXT_PARAM_ID_CRYPTO_KEY_USAGE_FLAG         0xFD

/* Length of R4 Proprietary Application Configurations */
#define UCI_EXT_PARAM_ID_M_UPSK_LEN                    0x11
#define UCI_EXT_PARAM_ID_SALTED_HASH_LEN               0x11
#define UCI_EXT_PARAM_ID_D_URSK_LEN                    0x11
#define UCI_EXT_PARAM_ID_D_UDSK_LEN                    0x11
#define UCI_EXT_PARAM_ID_NBIC_CONF_ID_LEN              0x01
#define UCI_EXT_PARAM_ID_TX_POWER_ID_LEN               0x01
#define UCI_EXT_PARAM_ID_DEBUG_LOG_LEVEL_LEN           0x01
#define UCI_EXT_PARAM_ID_RX_PHY_LOGGING_ENBL_LEN       0x01
#define UCI_EXT_PARAM_ID_TX_PHY_LOGGING_ENBL_LEN       0x01
#define UCI_EXT_PARAM_ID_LOG_PARAMS_CONF_LEN           0x04
#define UCI_EXT_PARAM_ID_CIR_TAP_OFFSET_LEN            0x02
#define UCI_EXT_PARAM_ID_CIR_NUM_TAPS_LEN              0x02
#define UCI_EXT_PARAM_ID_STS_INDEX_RESTART_LEN         0x01
#define UCI_EXT_PARAM_ID_RX_RADIO_CFG_IDXS_LEN         0x02
#define UCI_EXT_PARAM_ID_TX_RADIO_CFG_IDXS_LEN         0x02
#define UCI_EXT_PARAM_ID_TX_MAX_BLOCK_NUM_LEN          0x02
#define UCI_EXT_PARAM_ID_CRYPTO_KEY_USAGE_FLAG_LEN     0x01
#define UCI_EXT_PARAM_ID_DDFS_TONE_CONFIG_ENABLE_LEN   0x01
#define UCI_EXT_PARAM_ID_TELC_CONFIG_LEN               0X04

#define UCI_EXT_TEST_MODE_RECEIVE_MODE                 0x00
#define UCI_EXT_TEST_MODE_TRANSMIT_MODE                0x01
#define UCI_EXT_TEST_MODE_CONTINUOUS_WAVE_MODE         0x02
#define UCI_EXT_TEST_MODE_LOOP_BACK_MODE               0x03
#define UCI_EXT_TEST_MODE_INITIATOR_MODE               0x04
#define UCI_EXT_TEST_MODE_LOOP_BACK_AND_SAVE_MODE      0x05

#define UCI_EXT_RECEIVE_MODE_NO_OF_PARAMS              0x05
#define UCI_EXT_TRANSMIT_MODE_NO_OF_PARAMS             0x06
#define UCI_EXT_CONTINUOUS_WAVE_MODE_NO_OF_PARAMS      0x01
#define UCI_EXT_LOOP_BACK_MODE_NO_OF_PARAMS            0x01
#define UCI_EXT_INITIATOR_MODE_NO_OF_PARAMS            0x02

#define UCI_EXT_TEST_MODE_ID_TEST_MODE                 0x00
#define UCI_EXT_TEST_MODE_ID_DELAY                     0x01
#define UCI_EXT_TEST_MODE_ID_SLOT_TYPE                 0x02
#define UCI_EXT_TEST_MODE_ID_PSDU                      0x03
#define UCI_EXT_TEST_MODE_ID_TIME_OUT                  0x04
#define UCI_EXT_TEST_MODE_ID_EVENT_COUNTER_MAX         0x05
#define UCI_EXT_TEST_MODE_ID_TX_CYCLE_TIME             0x06
#define UCI_EXT_TEST_MODE_ID_ENCRYPTED_PSDU            0x07

#define UCI_EXT_TEST_MODE_ID_TEST_MODE_LEN             0x01
#define UCI_EXT_TEST_MODE_ID_DELAY_LEN                 0x02
#define UCI_EXT_TEST_MODE_ID_SLOT_TYPE_LEN             0x01
#define UCI_EXT_TEST_MODE_ID_TIME_OUT_LEN              0x02
#define UCI_EXT_TEST_MODE_ID_EVENT_COUNTER_MAX_LEN     0x04
#define UCI_EXT_TEST_MODE_ID_TX_CYCLE_TIME_LEN         0x04
#define UCI_EXT_TEST_MODE_ID_ENCRYPTED_PSDU_LEN        0x01

/* Session State codes, as per UCI*/
/** Session State - Session status notification with in band stop received error */
#define UCI_SESSION_IN_BAND_STOP_RECEIVED               0x84

#endif /* UWB_UCI_EXT_DEFS_H */
