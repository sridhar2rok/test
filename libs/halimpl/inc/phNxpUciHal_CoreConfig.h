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

#ifndef _PHNXPUCIHAL_RHODESCONFIG_H_
#define _PHNXPUCIHAL_RHODESCONFIG_H_

#include "phUwb_BuildConfig.h"
#include "UwbCore_Types.h"

#define TYPE_VAL  0
#define TYPE_DATA 1
#define TYPE_STR  2

typedef struct
{
    unsigned char key;
    unsigned char type;
    const void *val;
} NxpParam_t;

#define CONFIG_VAL (void *)

typedef enum
{
    UWB_MODEM_FRAME_CONFIG,
    UWB_MODEM_PREAMBLE_IDX,
    UWB_MODEM_SFD_IDX,
    UWB_MODEM_RF_CHAN_NO,
    UWB_MODEM_PREAMBLE_DURATION,
    UWB_CORE_CONFIG_PARAM,
    UWB_BOARD_VARIANT_CONFIG,
    UWB_BOARD_VARIANT_VERSION,
    UWB_MW_RANGING_FEATURE,
    UWB_FW_LOG_THREAD_ID,
    UWB_SET_FW_LOG_LEVEL,
    UWB_STANDBY_TIMEOUT_VALUE,
    UWB_RANG_CYCLE_INTERVAL,
    UWB_APP_SESSION_TIMEOUT,
    UWB_RANG_SESSION_INTERVAL,
    UWB_DPD_ENTRY_TIMEOUT,
    UWB_LOW_POWER_MODE,
    UWB_HPD_ENTRY_TIMEOUT,
    UWB_MHR_IN_CCM,
    UWB_DDFS_TONE_CONFIG_ENABLE,
    UWB_TELEC_CONFIG
} NxpUwbConfig;

#endif //_PHNXPUCIHAL_RHODESCONFIG_H_
