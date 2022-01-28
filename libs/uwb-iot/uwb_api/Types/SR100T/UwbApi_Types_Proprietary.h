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

#ifndef UWBAPI_TYPES_PROPRIETARY_H
#define UWBAPI_TYPES_PROPRIETARY_H

#include "UwbApi_Types.h"

/* DPD Timeout Range */
/**  Minimum timeout Supported by UWBS */
#define UWBD_DPD_TIMEOUT_MIN 100
/**  Maximum timeout Supported by UWBS */
#define UWBD_DPD_TIMEOUT_MAX 5000
/**  Maximum length of version Supported by UWBS */
#define UWBD_VERSION_LENGTH_MAX 0x3
/**
 * \brief  Structure lists out the UWB Device Info Parameters.
 */

/** \addtogroup Uwb_Board_Specific_Types
 * @{ */
/**
 *  \name SetApp Configuration parameters supported in UWB API layer.
 */
/* @{ */
typedef enum appConfig
{
    /** 0:TDoA, 1:SS-TWR, 2:DS-TWR */
    RANGING_METHOD,
    /** 0:Static STS, 1:Dynamic STS, 2:Dynamic STS for controlee individual key */
    STS_CONFIG,
    /** 5,6,8,9 */
    CHANNEL_NUMBER,
    /** Slot duration in us */
    SLOT_DURATION,
    /** Ranging interval in ms */
    RANGING_INTERVAL,
    /** STS index init value */
    STS_INDEX,
    /** 0:CRC16, 1:CRC32 */
    MAC_FCS_TYPE,
    /** 1:Enable, 0:Disable b0 - Measurement Report
     * Phase,b1 - Ranging Control Phase    b2 : b7
     * - RFU   (default = 0x03) */
    RANGING_ROUND_PHASE_CONTROL,
    /** 0: No AOA, 1:-90 to +90 */
    AOA_RESULT_REQ,
    /** 0:Disable, 1:Enable, 2:Enable in proximity range */
    RNG_DATA_NTF,
    /** Proximity near in cm */
    RNG_DATA_NTF_PROXIMITY_NEAR,
    /** Proximity far in cm */
    RNG_DATA_NTF_PROXIMITY_FAR,
    /** 0:No STS, 1:STS follows SFD, 2:STS follows PSDU, 3:STS
     * follows SFD No PHR or PPDU default: 0x03 */
    RFRAME_CONFIG,
    /** 0:Dual RX, 1:RX1, 2:RX2 */
    RX_MODE,
    /** [9-24]:BPRF, [25-32]:HPRF */
    PREAMBLE_CODE_INDEX,
    /** [0,2]:BPRF, [1,2,3]:HPRF default 0x02 */
    SFD_ID,
    /** 0:6.81MBPS, 1:7.80MBPS */
    PSDU_DATA_RATE,
    /** 0:32 symbols, 1:64 symbols */
    PREAMBLE_DURATION,
    /** 0:Single Rx, b0: Antenna pair 1 ,..., b7: Antenna
     * pair 8 */
    RX_ANTENNA_SELECTION,
    /** b0: MAC header present, b1:MAC footer present */
    MAC_CFG,
    /** 0:Interval based, 1:Block based */
    RANGING_TIME_STRUCT,
    /** Number of slots for per ranging round default : 30 */
    SLOTS_PER_RR,
    /** 0:disable, 1:Enable */
    TX_ADAPTIVE_PAYLOAD_POWER,
    /** b0: Antenna1,..., b7:Antenna8 */
    TX_ANTENNA_SELECTION,
    /** 1: responder1,...,N: responder N */
    RESPONDER_SLOT_INDEX,
    /** 0: BPRF, 1:HPRF */
    PRF_MODE,
    /** Max no of slots for contention based ranging */
    MAX_CONTENTION_PHASE_LEN,
    /** Contention phase update value in no of slots */
    CONTENTION_PHASE_UPDATE_LEN,
    /** 0:Contention based, 1:Time scheduled ranging */
    SCHEDULED_MODE,
    /** 1: Enable, 0:Disable [Default] */
    KEY_ROTATION,
    /** Key Rotation rate 2^n where 0<=n<=15 */
    KEY_ROTATION_RATE,
    /** Session Priority 1-100, default : 50 */
    SESSION_PRIORITY,
    /** MAC address mode Default 0 [SHORT] */
    MAC_ADDRESS_MODE,
    /** Number of STS segments in the frame. 0x00:No STS
     * Segments(if PPDU_COFIG is 0). If PPDU Config is
     * set to 1 or 3 then 0x01:1 STS Segment(default),
     * 0x02:2 STS Segments */
    NUMBER_OF_STS_SEGMENTS,
    /** Number of Failed Ranging Round attempts before terminating
     * the session. Default : 0 */
    MAX_RR_RETRY,
    /** Indicates when ranging operation shall start after
     * ranging start request is issued from AP. Default : 0 */
    UWB_INITIATION_TIME,
    /** Enable/disable the hopping. 0x00: Disable 0x01:
     * Enable Default : 0 */
    RANGING_ROUND_HOPPING,
    /** Indicates how many times in-band termination signal needs to be sent by controller/initiator to a controlee device.
     * Default : 1 */
    IN_BAND_TERMINATION_ATTEMPT_COUNT,
    /** Sub-Session ID for the controlee device.
     * This config is mandatory and it is applicable if STS_CONFIG is set to 2 for controlee device */
    SUB_SESSION_ID,
    /** Random Transmission of Blink Frame among the configured number of slots within the Blink Interval. Default : 0 */
    BLINK_RANDOM_INTERVAL,
    /** Config to enable result report, 0: Disable, This is applicable only RANGING_ROUND_PHASE_CONTROL enabled */
    RESULT_REPORT_CONFIG,
    /**Maximum Number of ranging blocks to executed in a session  Default : 0*/
    MAX_NUMBER_OF_BLOCKS,
    /** End of App Configs*/
    END_OF_SUPPORTED_APP_CONFIGS,
    /** Estimate first path based on 0:RX1, 1:RX2, 2:Min of
     * RX1 and RX2 */
    TOA_MODE = 0xE300,
    /** bits[7:4] - CIR1 capture mode, bits[3:0] - CIR0 capture
     * mode */
    CIR_CAPTURE_MODE,
    /** enable disable encryption of Payload data. 0x00 -
     * Plain Text 0x01 - Encrypted(default) */
    MAC_PAYLOAD_ENCRYPTION,
    /** No of sync attempts in controlee.
     * Default 3. In the range [3-255]. */
    SESSION_SYNC_ATTEMPTS = 0xE305,
    /** No of scheduler attempt. Default 3. In the range
     * [3-255]. */
    SESSION_SCHED_ATTEMPTS,
    /** 1 - Enable, 0 - Disable */
    SCHED_STATUS_NTF,
    /** delta index for TX_POWER_ID for peak power 0 -
     * Default */
    TX_PEAK_POWER_DELTA_FCC,
    /** End of Ext App Configs*/
    END_OF_SUPPORTED_EXT_CONFIGS,
} eAppConfig;
/* @} */

typedef enum SetAppParams_type
{
    /** We don't know the type */
    kAPPPARAMS_Type_Unknown = 0,
    /** It's a 32 bit value */
    kAPPPARAMS_Type_u32 = 4,
    /** It's an array of 8 bit values */
    kAPPPARAMS_Type_au8 = 5,
} SetAppParams_type_t;

typedef struct SetAppParams_value_au8
{
    uint8_t *param_value;
    uint16_t param_len; // Just to  handle parameter of any length
} SetAppParams_value_au8_t;

typedef union SetAppParams_value {
    uint32_t vu32; // All values u8, u16 and u32 are processed here
    SetAppParams_value_au8_t au8;
} SetAppParams_value_t;

typedef struct SetAppParams_List
{
    /** Input: search this tag */
    eAppConfig param_id;
    /** Filled Implicitly: Expected type. */
    SetAppParams_type_t param_type;
    /** Input: Parameter Value */
    SetAppParams_value_t param_value;
} SetAppParams_List_t;

#define UWB_SET_APP_PARAM_VALUE(PARAM, VALUE)                                                  \
    {                                                                                          \
        .param_id = (PARAM), .param_type = (kAPPPARAMS_Type_u32), .param_value.vu32 = (VALUE), \
    }

#define UWB_SET_APP_PARAM_ARRAY(PARAM, ARRAY, LENGTH)                                                    \
    {                                                                                                    \
        .param_id = (PARAM), .param_type = (kAPPPARAMS_Type_au8), .param_value.au8.param_len = (LENGTH), \
        .param_value.au8.param_value = (uint8_t *)(ARRAY),                                               \
    }

/**
 * \brief  Structure lists out the UWB Device Info Parameters.
 */
/* @{ */
typedef struct phUwbDevInfo
{
    /** UCI version */
    UINT16 uciVersion;
    /** Device Name length*/
    UINT8 devNameLen;
    /** Device Name */
    UINT8 devName[48];
    /** Fw Major Version */
    UINT8 fwMajor;
    /** Fw Minor Version */
    UINT8 fwMinor;
    /** Fw Rc Version */
    UINT8 fwRc;
    /** NXP UCI Major Version */
    UINT8 nxpUciMajor;
    /** NXP UCI Minor Version */
    UINT8 nxpUciMinor;
    /** NXP UCI Patch Version */
    UINT8 nxpUciPatch;
    /** NXP Chip Id */
    UINT8 nxpChipId[16];
    /** MW Major Version */
    UINT8 mwMajor;
    /** MW Minor Version */
    UINT8 mwMinor;
} phUwbDevInfo_t;
/* @}*/

/**
 *  \name Do Calibration Configuration parameters supported in UWB API layer.
 */
/* @{ */
typedef enum doCalibParam
{
    /** VCO PLL Calibration, channel dependent */
    DO_CALIB_VCO_PLL = 0x0,
    /** PA_PPA Calibration control */
    DO_CALIB_PAPPPA_CALIB_CTRL
} eDoCalibParam;
/* @} */

/**
 *  \name Calibrations Configuration parameters supported in UWB API layer.
 */
/* @{ */
typedef enum calibParam
{
    /** VCO PLL Calibration, channel dependent */
    VCO_PLL = 0x0,
    /** TX POWER Calibration, channel and antenna dependent */
    TX_POWER,
    /** 38.4MHz XTAL, channel and antenna Independent */
    RF_XTAL_CAP,
    /** RSSI Calibration, channel and antenna dependent */
    RSSI_CALIB_CONST1,
    /** RSSI Calibration, channel and antenna dependent */
    RSSI_CALIB_CONST2,
    /** SNR Calibration, channel and antenna dependent */
    SNR_CALIB_CONST,
    /** power control parameters */
    MANUAL_TX_POW_CTRL,
    /** phase difference of arrival offset value, antenna dependent */
    PDOA_OFFSET,
    /** PA_PPA Calibration control */
    PAPPPA_CALIB_CTRL,
    /** Tx Temperature Compensation */
    TX_TEMPARATURE_COMP,
    /** AoA fine calibration parameter, Antenna dependent */
    AOA_FINE_CALIB
} eCalibParam;
/* @} */

/**
 * \brief  Structure lists out the calibration command response/notification.
 */
/* @{ */
typedef struct phCalibRespStatus
{
    /** Status */
    UINT8 status;
    /** One byte RFU in case of Do_calib command. Calibration state in
     * Get_calib command. */
    UINT8 rfu;
    /** Calibration value out */
    UINT8 calibValueOut[MAX_UCI_PACKET_SIZE];
    /** Calibration value length */
    UINT16 length;
} phCalibRespStatus_t;
/* @}*/

/**
 * \brief UWBD Type for SE_DO_BIND Notification.
 */
/* @{ */
typedef struct
{
    /** Status */
    UINT8 status;
    /** Remaining Binding Count */
    UINT8 count_remaining;
    /** Binding state */
    UINT8 binding_state;
} phSeDoBindStatus_t;
/* @}*/

/**
 * \brief UWBD Type for SE_GET_BINDING_STATUS Notification.
 */
/* @{ */
typedef struct
{
    /** Binding status*/
    /** 0x00: Not Bound,*/
    /** 0x01: Bound Unlocked,*/
    /** 0x02: Bound Locked,*/
    /** 0x03: Unknown ( if any error occurred during getting binding state from SE) */
    UINT8 status;
    /** Remaining binding count in SE */
    UINT8 se_binding_count;
    /** Remaining binding count in uwb device */
    UINT8 uwbd_binding_count;
} phSeGetBindingStatus_t;
/* @}*/

/**
 * \brief UWBD Type for SE_GET_BINDING_COUNT_RSP.
 */
/* @{ */
typedef struct
{
    /** Binding Status */
    UINT8 bindingStatus;
    /** Remaining binding count in uwb device */
    UINT8 uwbdBindingCount;
    /** Remaining binding count in SE */
    UINT8 seBindingCount;
} phSeGetBindingCount_t;
/* @}*/

/**
 * \brief  Structure lists out the additional rframe ranging notification
 * information.
 */
/* @{ */
typedef struct phRframeData
{
    /** Data Length */
    UINT16 dataLength;
    /** Data */
    UINT8 data[MAX_RFRAME_PACKET_SIZE];
} phRframeData_t;
/* @}*/

/**
 * \brief  Structure lists out the scheduler status notification information.
 */
/* @{ */
typedef struct phSchedStatusNtfData
{
    /** Data Length */
    UINT16 dataLength;
    /** Data */
    UINT8 data[MAX_UCI_PACKET_SIZE];
} phSchedStatusNtfData_t;
/* @}*/

/**
 * \brief  Structure lists out the Test connectivity data.
 */
/* @{ */
typedef struct phTestConnectivityData
{
    /** Status */
    UINT8 status;
    /** wtx count */
    UINT8 wtx;
} phTestConnectivityData_t;
/* @}*/

/**
 * \brief  Structure lists out the SE test loop information.
 */
/* @{ */
typedef struct phSeTestLoopData
{
    /** Status */
    UINT8 status;
    /** No of times loop was run */
    UINT16 loop_cnt;
    /** No of times loop successfully completed */
    UINT16 loop_pass_count;
} phTestLoopData_t;
/* @}*/

/**
 * \brief  Structure lists out the configurations to be set for FW Debug Levels.
 */
/* @{ */
typedef struct phDebugParams
{
    /** Secure Thread */
    UINT16 secureThread;
    /** Secure ISR Thread */
    UINT16 secureIsrThread;
    /** Non Secure ISR Thread */
    UINT16 nonSecureIsrThread;
    /** Shell Thread */
    UINT16 shellThread;
    /** PHY Thread */
    UINT16 phyThread;
    /** Ranging Thread */
    UINT16 rangingThread;
    /** Data Logger NTF, 0x00: Disable, 0x01: Enable */
    UINT8 dataLoggerNtf;
    /** CIR Log NTF */
    UINT8 cirLogNtf;
    /** PSDU Log NTF */
    UINT8 psduLogNtf;
    /** RFRAME Log NTF */
    UINT8 rframeLogNtf;
} phDebugParams_t;
/* @}*/
#ifdef __DEPRECATED
/**
 * \brief  Structure lists out the send data notification.
 */
/* @{ */
typedef struct phSendData
{
    /** Session ID */
    UINT32 sessionId;
    /** Status */
    UINT8 status;
} phSendData_t;
/* @}*/

/**
 * \brief  Structure lists out the receive data notification.
 */
/* @{ */
typedef struct phRcvData
{
    /** Session ID */
    UINT32 session_id;
    /** Status */
    UINT8 status;
    /** Data length */
    UINT8 data_len;
    /** Data received */
    UINT8 data[MAX_APP_DATA_SIZE];
} phRcvData_t;
/* @}*/
#endif

/**
 * \brief  Structure lists out the SE Comm Error notification.
 */
/* @{ */
typedef struct phSeCommError
{
    /** Status */
    UINT8 status;
    /** T=1 command for which SE communication is failed. */
    UINT16 cla_ins;
    /** T=1 status codes(SW1SW2) */
    UINT16 t_eq_1_status;
} phSeCommError_t;
/* @}*/

/* @}*/ /* addtogroup Uwb_Board_Specific_Types */

#endif
