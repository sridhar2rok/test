/*
 * Copyright 2019,2020 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UWBAPI_TYPES_PROPRIETARY_H
#define UWBAPI_TYPES_PROPRIETARY_H

#include "UwbApi_Types.h"
#include "uci_ext_defs.h"

/* DPD Timeout Range */
#if UWBIOT_TML_SPI
/**  When running natively, time depends on underlying HW, but save power */
#define UWBD_DPD_TIMEOUT_MIN 1
#else
/**  When running from PC, Relax a bit. */
#define UWBD_DPD_TIMEOUT_MIN 100
#endif

#define UWBD_DPD_TIMEOUT_MAX 5000

/**  Maximum timeout Supported by UWBS */

/* HPD Timeout Range */
/**  Minimum timeout Supported by UWBS */
#if UWBIOT_TML_SPI
/**  When running natively, time depends on underlying HW, but save power */
#define UWBD_HPD_TIMEOUT_MIN 10
#else
#define UWBD_HPD_TIMEOUT_MIN 100
#endif

/**  Maximum timeout Supported by UWBS */
#define UWBD_HPD_TIMEOUT_MAX 5000

/* Session ID RF Test */
/**  Session ID used for RF TEST Mode */
#define SESSION_ID_RFTEST 0x00000000

/**  Time out value for calibration notification */
#define UWBD_TEST_MODE_NTF_TIMEOUT 2000

/**  Max value of Response Length for Radio Config get version command */
#define UWB_MAX_RADIO_CONFIG_VERSION_LEN 0x08

/** \addtogroup uwb_apis_sr040
 *
 * @{ */

/**
 * \brief  Structure lists out the UWB Device Info Parameters.
 */
typedef struct phUwbDevInfo
{
    /** UCI version */
    UINT16 uciVersion;
    /** Device Name */
    UINT8 devName[UCI_EXT_PARAM_ID_DEVICE_NAME_LEN];
    /** Fw Major Version */
    UINT8 fwMajor;
    /** Fw Minor Version */
    UINT8 fwMinor;
    /** Fw Patch Version */
    UINT8 fwPatchVersion;
    /** MW Major Version */
    UINT8 mwMajor;
    /** MW Minor Version */
    UINT8 mwMinor;
    /** Device Minor Version */
    UINT8 devMinor;
    /** Device Major Version */
    UINT8 devMajor;
    /** Serial No */
    UINT8 serialNo[UCI_EXT_PARAM_ID_SERIAL_NUMBER_LEN];
    /** DSP Fw Major Version */
    UINT8 dspMajor;
    /** DSP Fw Minor Version */
    UINT8 dspMinor;
    /** DSP Fw Patch Version */
    UINT8 dspPatchVersion;
    /** Fw Major Version */
    UINT8 bbMajor;
    /** Fw Minor Version */
    UINT8 bbMinor;
    /** Fw Patch Version */
    UINT8 bbPatchVersion;
    /** Fw Patch Version */
    UINT8 cccVersion[UCI_EXT_PARAM_ID_CCC_VERSION_LEN];
} phUwbDevInfo_t;

/**
 *  \name SetApp Configuration parameters supported in UWB API layer.
 */
typedef enum appConfig
{
    /** Ranging Method
     *
     *  - 0:TDoA
     *  - 1:SS-TWR
     *  - 2:DS-TWR */
    RANGING_METHOD = 0x01,
    /** STS Config
     *
     *  - 0:Static STS
     *  - 1:Dynamic STS */
    STS_CONFIG = 0x02,
    /**  5,6,8,9 */
    CHANNEL_NUMBER = 0x04,
    /**  Slot duration in us */
    SLOT_DURATION = 0x08,
    /**  Ranging interval in ms */
    RANGING_INTERVAL = 0x09,
    /**  STS index init value */
    STS_INDEX = 0x0A,
    /**  0:CRC16, 1:CRC32 */
    MAC_FCS_TYPE = 0x0B,
    /**
     * - 1:Enable,
     * - 0:Disable
     *
     * - b0 – Measurement Report Phase,
     * - b1 – Ranging Control Phase
     * - b2 : b7 - RFU
     * (default = 0x03) */
    RANGING_ROUND_PHASE_CONTROL = 0x0C,
    /** 0: No AOA, 1:-90 to +90 */
    AOA_RESULT_REQ = 0X0D,
    /**  0:Disable, 1:Enable, 2:Enable in proximity range */
    RNG_DATA_NTF = 0x0E,
    /**  Proximity near in cm */
    RNG_DATA_NTF_PROXIMITY_NEAR = 0x0F,
    /**  Proximity far in cm */
    RNG_DATA_NTF_PROXIMITY_FAR = 0x10,
    /**  0:No STS, 1:STS follows SFD, 2:STS follows PSDU, 3:STS follows SFD */
    RFRAME_CONFIG = 0x12,
    /**  [9-24]:BPRF, [25-32]:HPRF */
    PREAMBLE_CODE_INDEX = 0x14,
    /**  [0,2]:BPRF, [1,3]:HPRF */
    SFD_ID = 0x15,
    /**  0:6.81MBPS, 1:7.80MBPS */
    PSDU_DATA_RATE = 0x16,
    /**  0:32 symbols, 1:64 symbols */
    PREAMBLE_DURATION = 0x17,
    /**  b0: MAC header present, b1:MAC footer present */
    MAC_CFG = 0x19,
    /**  0:Interval based, 1:Block based */
    RANGING_TIME_STRUCT = 0x1A,
    /**  Slot per Ranging Round, Not applicable for Contention Based Ranging */
    SLOTS_PER_RR = 0x1B,
    /**  0:disable, 1:Enable */
    TX_ADAPTIVE_PAYLOAD_POWER = 0x1C,
    /**  1: responder1,...,N: responder N */
    RESPONDER_SLOT_INDEX = 0x1E,
    /**  0: BPRF, 1:HPRF */
    PRF_MODE = 0x1F,
    /**  1: Enable, 0:Disable [Default] */
    KEY_ROTATION = 0x23,
    /**  Session Priority 1-100, default : 50 */
    SESSION_PRIORITY = 0x25,
    /**  MAC address mode Default 0 [SHORT] */
    MAC_ADDRESS_MODE = 0x26,

    /** Number of STS segments in the frame.
     * 0x00:No STS Segments (if RFRAME_CONFIG is 0).
     *
     * If RFRAME_CONFIG Config is set to 1 or 3 then
     *
     * - 0x01:1 STS Segment(default)
     * - 0x02:2 STS Segments */
    NUMBER_OF_STS_SEGMENTS = 0x29,

    /**  Maximum Ranging Round Retry */
    MAX_RR_RETRY = 0x2A,
    /**  Ranging round hopping, 1=Enable, 0=Disable [Default] */
    RANGING_ROUND_HOPPING = 0x2C,
    /** Config to enable result report, 0: Disable, This is applicable only RANGING_ROUND_PHASE_CONTROL enabled */
    RESULT_REPORT_CONFIG = 0x2E,
    /** Random Transmission of Blink Frame among the configured number of slots within the Blink Interval. Default : 0 */
    BLINK_RANDOM_INTERVAL = 0x32,

    END_OF_SUPPORTED_APP_CONFIGS,
    /**  Sts Index Restart */
    STS_INDEX_RESTART = 0xE300,
    /** TX Power ID
     * - 0: max power (14db)
     * - 104: least power(-12db)
     *
     * Range 0 to 104: 0.25db per step */
    TX_POWER_ID = 0xE301,
    /**  Phy logging for Test Receive Mode, 1=Enable, 0=Disable [Default] */
    RX_PHY_LOGGING_ENABLE,
    /**  Phy logging for Test Transmit Mode, 1=Enable, 0=Disable [Default] */
    TX_PHY_LOGGING_ENABLE,
    /**  RX radio configuration slot index in flash
    * - Low byte indicates slot index for SP0 type
    * - High byte indicates slot index for SP3 type
    * Minimum slot index: 0x10, Maximum slot index: 0x1F
    * Default: 0x0100 [SP3(High Byte): index 0x01, SP0(Low Byte): index 0x00]
    */
    RX_RADIO_CFG_IDXS,
    /**  TX radio configuration slot index in flash
    * - Low byte indicates slot index for SP0 type
    * - High byte indicates slot index for SP3 type
    * Minimum slot index: 0x10, Maximum slot index: 0x1F
    * Default: 0x1110 [SP3(High Byte): index 0x11, SP0(Low Byte): index 0x10]
    */
    TX_RADIO_CFG_IDXS,
    /**MAX BLOCK NUMBER
     */
    MAX_BLOCK_NUM,
    /**
     * PHSCA_UCIMSG_SESSION_NBIC_CONF_ID - 0xF2u
     *
     * NBIC disabled - 0u,
     * NBIC enabled - 1u,
     * NBIC enabled FS mode enabled - 2,
     */
    SR040_NBIC_CONF,
    END_OF_SUPPORTED_EXT_CONFIGS,
} eAppConfig; /* Update ext_app_config_mapping when modifying this */

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
 *  \name Calibrations Configuration parameters supported in UWB API layer.
 */
typedef enum calibParam
{
    /**  A difference between received and expected TX power */
    TX_POWER_DIFF = 0x0,
    /** A difference (in Hz) between received and expected UWB frequency */
    FREQ_DIFF,
    /**  Antenna delay in 15.65ps resolution */
    ANTENNA_DELAY,
    /**  Current Limiter Value, 0:minimum 20:maximum, 20:default*/
    CURRENT_LIMIT_VALUE,
    /**  Temp compens flag Value */
    TEMP_COMPENS_FLAG = 0x05,
    /**  Tx Adaptive power calc Value */
    TX_ADAPTIVE_POWER_CALC = 0x06,
    /** DDFS Tone Values */
    DDFS_TONE_VALUES = 0x07,
    /** DPD Timer Penalty in us */
    DPD_TIMER_PENALTY_US
} eCalibParam;

/**
 *  \name Session NVM Manage parameters supported in UWB API layer.
 */
typedef enum sessionNvmManage
{
    /* Session ID to Persist */
    SESSION_NVM_MANAGE_PERSIST = 0x00,
#if 0
    /* Session ID to Remove */
    SESSION_NVM_MANAGE_DELETE = 0x01,  /* Not available */
#endif
    /* Delete All persisted sessions */
    SESSION_NVM_MANAGE_DELETE_ALL = 0x02,
} esessionNvmManage;

/**
 *  \name Status code for Calibration
 */
typedef enum calibStatus
{
    /**  Requested/Stored in the NVM trim data has been successfully applied */
    APPLIED = 0x00,
    /**  Data in NVM has been corrupted and it is replaced with the default trim values */
    CORRUPTED,
    /**  Operation failed to complete, previous trim configuration is still applied */
    FAILURE
} eCalibStatus;

/**
 *  \name Radio Config Download Seqeunce
 */
typedef enum downloadSequence
{
    /** Radio Config Init Configuration Sequence */
    kSR040_RadioCrc_InitDownload = 0x00,
    /** Radio Config Download data Sequence */
    kSR040_RadioCrc_DownloadChunk,
    /** Radio Config Program flash Sequence */
    kSR040_RadioCrc_Finish,
    /** Radio Config Get CRC Sequence */
    kSR040_RadioCrc_GetCRC,
    /** Radio Config Get Version Sequence */
    kSR040_RadioCrc_GetVersion
} edownloadSequence;

#define INIT_CONFIG   kSR040_RadioCrc_InitDownload
#define DOWNLOAD_DATA kSR040_RadioCrc_DownloadChunk
#define PROGRAM_FLASH kSR040_RadioCrc_Finish
#define GET_CRC       kSR040_RadioCrc_GetCRC
#define GET_VERSION   kSR040_RadioCrc_GetVersion

/**
 * \brief UWBD Type for Calibration Status Notification.
 */
typedef struct
{
    /** Calibration status */
    eCalibStatus status;
} phCalibrationStatus_t;

/**
 * \brief TX DIFF Calibration Data
 */
typedef struct
{
    /** channel number, [5 - 9] */
    UINT8 channelNo;
    /** sign:
     *
     * - 0: +ve,
     * - 1: -ve */
    UINT8 signVal;
    /** An absolute value in 0.25 dB steps.
     *
     * - 0: 0 dB (no offset),
     * - 1: 0.25 dB
     * - 2: 0.5db,
     * - ..
     * - 12: 3 dB (max) */
    UINT8 absoluteVal;
} phTxPowerDiffCalibData_t;

/**
 * \brief FREQUENCY DIFF Calibration Data
 */
typedef struct
{
    /** channel number, [5 - 9] */
    UINT8 channelNo;
    /** sign:
     *
     * - 0: +ve,
     * - 1: -ve */
    UINT8 signVal;
    /** Offset:
     *
     * - 0 Hz (no offset)
     * - 0x7A120: 500000 Hz (max)
     *
     * Note: This offset will be converted to PPM in 0.1 ppm
     * resolution per original frequency of each channel.
     *
     * For example, possible minimum offset frequency for Ch5
     * is 649Hz which is 0.1ppm per original frequency of Ch5 6489600Khz */
    UINT32 absoluteFreqOffset;
} phFreqDiffCalibData_t;

/**
 * \brief Antenna Delay Calibration Data
 */
typedef struct
{
    /** channel number, [5 - 9] */
    UINT8 channelNo;
    /**  antenna delay in 15.65ps
     *
     * resolution
     * - 0x0000 : 0 ps(no delay)
     * - 0xFFFF : 1 025 638.4 ps(max) */
    UINT16 antennaDelay;
} phAntennaDelayCalibData_t;

/**
 * \brief Tx Adaptive power calculation Data
 */
typedef struct
{
    /** channel number, [5 - 9] */
    UINT8 channelNo;
    /**  Power Id RMS delay in 0.25 dB steps */
    UINT8 powerIdRms;
    /** Peak Delta default 0x00 */
    UINT8 peakDelta;
} phTxAdaptivePowerCalibData_t;

/**
 * \brief DDFS Tone Configuration Data
 */
typedef struct
{
    /** channel number [5 - 9] */
    UINT8 channelNo;
    /**  RFU byte. */
    UINT8 rfu;
    /** Content of register TX_DDFS_TONE_0 */
    UINT32 txDdfsTone0RegVal;
    /** Content of register TX_DDFS_TONE_1 */
    UINT32 txDdfsTone1RegVal;
    /** Spur Duration */
    UINT32 spurDuration;
    /** Content of register GAINVAL_SET  */
    UINT8 gainvalsetRegVal;
    /** Content of register DDFSGAINBYPASS_ENBL  */
    UINT8 gainByPassEnblRegVal;
    /** Spur Periodicity */
    UINT16 spurPeriodicity;
} phDdfsToneConfigData_t;

/**
 * \brief Calibration data
 */
typedef union {
    /** TX POWER DIFF Calibration Data */
    phTxPowerDiffCalibData_t txPowerDiffCalibData;
    /** FREQ DIFF Calibration Data */
    phFreqDiffCalibData_t freqDiffCalibData;
    /** Antenna Delay Calibration Data */
    phAntennaDelayCalibData_t antennaDelayaCalibData;
    /** Current Limit Value */
    UINT8 currLimitValue;
    /** Tx Adaptive power Calibration Data */
    phTxAdaptivePowerCalibData_t txAdaptivePowerCalibData;
    /** DDFS tone config data for all channels */
    phDdfsToneConfigData_t ddfsToneConfigData[4];
    /** temp CompesFlag 1:Disable 0:Enable */
    UINT8 tempCompensFlag;
    /** dpd timer penalty in us. Default value: 14917 */
    UINT32 dpdTimerPenalty;
} phCalibrationData;

/**
 *  \name Enum defined for Test Mode Start/Stop
 */
typedef enum testMode
{
    /** Start Device in TEST_RECEIVE_MODE */
    RECEIVE_MODE = 0x00,
    /** Start Device in TEST_TRANSMIT_MODE */
    TRANSMIT_MODE,
    /** Start Device in TEST_CONTINUOUS_WAVE_MODE */
    CONTINUOUS_WAVE_MODE,
    /** Start Device in TEST_LOOP_BACK_MODE */
    LOOP_BACK_MODE,
    /** Start Device in TEST_INITIATOR_MODE */
    INITIATOR_MODE,
    /** Start Device in TEST_LOOP_BACK_MODE_NVM_AND_SAVE */
    LOOP_BACK_MODE_NVM_AND_SAVE,
} eTestMode;

/** Slot type in test mode Tx/Rx */
typedef enum
{
    kUWB_SR040_TxRxSlotType_SP0 = 0,
    kUWB_SR040_TxRxSlotType_SP1 = 1,
    kUWB_SR040_TxRxSlotType_SP3 = 3,
} UWB_SR040_TxRxSlotType_t;

/**
 * \brief Common Params used in Tx & Rx Test Mode
 */
typedef struct
{
    /** Start Delay in Micro Seconds, Default is 0 */
    UINT16 startDelay;
    /** Test Slot Type, 0x00 : SP0 type which has only PSDU without STS
                        0x03 : SP3 type which has only STS
                        Default: 0x00 SP0 */
    UWB_SR040_TxRxSlotType_t slotType;
    /** Specifies at which number of events the testmode will be stopped.
        0x00 means transmit/receive infinite number of events
        Default : 0x00 */
    UINT32 eventCounterMax;
} phRxTxCommonParams_t;

/**
 * \brief Structure required for #RECEIVE_MODE
 */
typedef struct
{
    /** Params common for Tx & Rx Test Mode */
    phRxTxCommonParams_t commonParams;
    /** Timeout value for Receive Mode in ms resolution 1 LSB = 1ms
                        Timeout shall be equal at least 1ms.
                        Default: 100ms */
    UINT16 timeOut;
    /* Enable or Disable Psdu Logging */
    UINT8 enableLogging;
} phReceiveModeParams_t;

/**
 * \brief Structure required for #TRANSMIT_MODE
 */
typedef struct
{
    /** Params common for Tx & Rx Test Mode */
    phRxTxCommonParams_t commonParams;
    /** Payload for Transmit Mode
        For SP0 this is mandatory parameter. */
    UINT8 *psduData;
    /* Length of the PSDU */
    UINT8 psduLen;
    /** Cycle time for periodic mode. Defined from Start of TX to Start of next TX.
        1 LSB = 1us. (Min. value should be 1000us,
        otherwise we would need complex error handling based on frame
        length settings)
        Should be in range [1000us .. 10s] = [0x3E8us .. 0x989680us]
        Default: 1000us */
    UINT32 txCycleTime;
    /* Enable or Disable Psdu Logging */
    UINT8 enableLogging;
} phTransmitModeParams_t;

/**
 * \brief Structure required for #INITIATOR_MODE
 */
typedef struct
{
    /** RCM & RMI PSDU to be encrypted or not, 0: Not-Encrypted, 1: Encrypted*/
    UINT8 isPsduEncrypted;
} phInitiatorModeParams_t;

/**
 * \brief Test Mode Params
 */
typedef union {
    /** Structure to be used when #RECEIVE_MODE is the input */
    phReceiveModeParams_t receiveModeParams;
    /** Structure to be used when #TRANSMIT_MODE is the input */
    phTransmitModeParams_t transmitModeParams;
    /** Structure to be used when #INITIATOR_MODE is the input */
    phInitiatorModeParams_t initiatorModeParams;
} phTestModeParams;

/**
 * \brief UWBD Type for Test Loopback Status Notification.
 */
typedef struct
{
    /** Generic Status Code */
    UINT8 status;
    /** Calculated Group Delay */
    UINT32 groupDelay;
} phTestLoopbackData_t;

/**
 * \brief Test Initiator Range Status Notification.
 */
typedef struct
{
    /** Session ID of the currently active session */
    UINT32 sessionId;
    /** Current Block Index */
    UINT16 blockIndex;
    /** STS index of RMI */
    UINT32 stsIndex;
    /** Next Ranging Round Index */
    UINT16 rrIndex;
} phTestInitiatorRangekData_t;

/**
 * \brief Phy Log Notification.
 */
typedef struct
{
    /** Size of data */
    UINT16 size;
    /** Data received part of phy log */
    UINT8 data[MAX_UCI_PACKET_SIZE];
} phPhyLogNtfnData_t;

/**
 * \brief Test Mode Output Data
 */
typedef union {
    /** Structure to be used for Test Loopback Status Notification */
    phTestLoopbackData_t loopBackData;
} phTestModeOutputData;
/** @}  */ /* @addtogroup uwb_apis_sr040 */

/**
 * \brief Radio Config Get Version command response.
 */
typedef struct phRadioConfigGetVersionResp
{
    /** Major Version of the corresponding Rx/Tx Sequence */
    UINT16 major;
    /** Minor Version of the corresponding Rx/Tx Sequence */
    UINT16 minor;
    /** patch Version of the corresponding Rx/Tx Sequence */
    UINT16 patch;
} phRadioConfigGetVersionResp_t;

#endif // UWBAPI_TYPES_PROPRIETARY_H
