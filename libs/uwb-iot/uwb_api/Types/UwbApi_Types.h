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

#ifndef UWBAPI_TYPES_H
#define UWBAPI_TYPES_H

#include "UwbCore_Types.h"
#include "uci_defs.h"

/**
 *  \brief Constants used by UWBD
 */

/** \addtogroup Uwb_Constants
 * @{ */

/**
  *  \name Constants used by UWB API layer
  */
/* @{ */

/**  MAX UCI packet size */
#define MAX_UCI_PACKET_SIZE 255
/**  MAX RFRAME packet size */
#define MAX_RFRAME_PACKET_SIZE 512
/**  max app data size */
#define MAX_APP_DATA_SIZE 116
/**  Max psdu data size */
#define MAX_PSDU_DATA_SIZE 128
/**  MAX Number of Responders, for QS it can go UPto 12 */
#define MAX_NUM_RESPONDERS 12
/**  MAX Number of Controlees for Physical Access */
#define MAX_NUM_PHYSICAL_ACCESS_CONTROLEES 8
/** LENGTH OF THE MAC ADDRESS*/
#define MAC_ADDR_LENGTH 8
/* @} */

/* @} */

/**
 *  \name Some macros used by UWB API layer.
 */
/* @{ */

/**  Short MAC Mode Address length */
#define MAC_SHORT_ADD_LEN 2
/**  Extended MAC Mode Address length */
#define MAC_EXT_ADD_LEN 8
/**  AOA Phase difference length */
#define AOA_PHASE_DIFF_LEN 4
/**  AOA UCI Field length */
#define AOA_LEN 4
/**  Single AOA length */
#define SINGLE_AOA_LEN 4
/** sub session id length */
#define SUBSESSION_ID_LEN 4
/** add a controllee to multicast addr list */
#define MULTICAST_LIST_ADD_CONTROLEE 0
/** delete a controlee from multicast addr list */
#define MULTICAST_LIST_DEL_CONTROLEE 1
/* @}*/

/*
 *  \name Deprecated related macros.
 */
#define __DEPRECATED 1

#define DEFAULT_EVENT_TYPE 0xFF
/**  UWB command time out 2 Sec */
#define UWB_CMD_TIMEOUT 2000
/**  UWB reset timeout 500 ms */
#define UWB_NTF_TIMEOUT 500
/**  UWB device bind lock timeout 2 sec */
#define UWBD_LOCK_TIMEOUT 2000
/**  timeout for SE APIs */
#define UWBD_SE_TIMEOUT 10000
/**  timeout for SE Ranging APIs */
#define UWBD_SE_RANGING_TIMEOUT 40000
/**  Time out value for calibration notification */
#define UWBD_CALIB_NTF_TIMEOUT 2000
/**  Time out value for do calibration notification */
#define UWBD_DO_CALIB_NTF_TIMEOUT 2000

#define UWB_KEY_FETCH_ERROR_RETRY_COUNT 0x01
#define STATUS_BINDING_SUCCESS          0x50

/** @} */ /* @addtogroup Uwb_Constants */

/**
 *  \brief Status Code used by Uwb Device
 */

/** \addtogroup uwb_status
 * @{ */

/**
 * \name Status codes, as per UCI.
 */
/* @{ */
/**  Command succeeded */
#define UWBAPI_STATUS_OK 0x00
/**  Request is rejected */
#define UWBAPI_STATUS_REJECTED 0x01
/**  Command Failed */
#define UWBAPI_STATUS_FAILED 0x02
/**  API called without UCI being initialized */
#define UWBAPI_STATUS_NOT_INITIALIZED 0x03
/**  Invalid parameter provided */
#define UWBAPI_STATUS_INVALID_PARAM 0x04
/** Invalid value range provided */
#define UWBAPI_STATUS_INVALID_RANGE 0x05
/** Session wrt SESSION ID Does not exist */
#define UWBAPI_STATUS_SESSION_NOT_EXIST 0x11
/**  Session wrt SESSION ID Already Present */
#define UWBAPI_STATUS_SESSION_DUPLICATE 0x12
/**  MAX Sessions exceeded */
#define UWBAPI_STATUS_MAX_SESSIONS_EXCEEDED 0x14
/**  Operation is started with out configuring required parameters for Session */
#define UWBAPI_STATUS_SESSION_NOT_CONFIGURED 0x15

/**  sessions ongoing */
#define UWBAPI_STATUS_SESSIONS_ONGOING 0X16
/**  ESE Rest happened during command processing. */
#define UWBAPI_STATUS_ESE_RESET 0x17
/**  Buffer overflow */
#define UWBAPI_STATUS_BUFFER_OVERFLOW 0xFE
/**  SE Error */
#define UWBAPI_STATUS_ESE_ERROR 0xFF
/**  Device is woken up from HPD */
#define UWBAPI_STATUS_HPD_WAKEUP 0xFC
/**  Command failed with time out */
#define UWBAPI_STATUS_TIMEOUT 0xFD
/* @}*/

/**
 * \name Device Status codes, as per UCI.
 */
/* @{ */
/**  Device State - IDLE */
#define UWBAPI_UCI_DEV_IDLE 0x00
/**  Device State - READY */
#define UWBAPI_UCI_DEV_READY 0x01
/**  Device State - ACTIVE */
#define UWBAPI_UCI_DEV_ACTIVE 0x02
/** Device State - ERROR */
#define UWBAPI_UCI_DEV_ERROR 0xFF
/* @}*/

/**
 * \name Session State codes, as per UCI.
 */
/* @{ */
/**  Session State - Session Init Success */
#define UWBAPI_SESSION_INIT_SUCCESS 0x00
/**  Session State - Session DeInit Success */
#define UWBAPI_SESSION_DEINIT_SUCCESS 0x01
/**  Session State - Session Activated */
#define UWBAPI_SESSION_ACTIVATED 0x02
/**  Session State - Session Idle */
#define UWBAPI_SESSION_IDLE 0x03
/**  Session State - Error */
#define UWBAPI_SESSION_ERROR 0xFF
/* @}*/

/** @} */ /* @addtogroup uwb_status */

/**
 *  \brief Types used by Uwb APIs
 */

/** \addtogroup Uwb_Types
 * @{ */

/**
 * \brief  Status used by UWB APIs.
 */
/* @{ */
typedef UINT8 tUWBAPI_STATUS;
/* @}*/

/**
 * \brief       RESET Modes.
 */
/* @{ */
typedef enum resetType
{
    /** UWB Device Reset */
    UWBD_RESET = 0,
    /** RFU */
    RFU
} eDevResetMode;
/* @}*/

/**
 * \brief  UWBD  Firmware Modes.
 */
/* @{ */
typedef enum sdkMode
{
    /** Factory Firmware */
    FACTORY_FW,
    /** Mainline Firmware */
    MAINLINE_FW
} eFirmwareMode;
/* @}*/

/**
 * \brief  UWBD Session Type.
 */
/* @{ */
typedef enum session_type
{
    /** Ranging Session */
    UWBD_RANGING_SESSION,
    /** Data Transfer Session */
    UWBD_DATA_TRANSFER,
    /** RFU */
    UWBD_RFU,
    /** Test Session */
    UWBD_RFTEST = 0xD0
} eSessionType;
/* @}*/

/**
 * \brief  UWBD notification Type.
 */
/* @{ */
typedef enum notification_type
{
    /** Ranging Data Notification */
    UWBD_RANGING_DATA,
#ifdef __DEPRECATED
    /** Data Sent Notification */
    UWBD_DATA_TRANSFER_SEND,
    /** Data Received Notification */
    UWBD_DATA_TRANSFER_RCV,
#endif
    /** PER Packet Sent Notification */
    UWBD_PER_SEND,
    /** PER receive operation completed notification */
    UWBD_PER_RCV,
    /** Generic Error Notification */
    UWBD_GENERIC_ERROR_NTF,
    /** Upon Receiving Device Reset, Application needs to
     * clear all session context and reinitiate all the
     * sessions */
    UWBD_DEVICE_RESET,
    /** RFRAME Data Notification */
    UWBD_RFRAME_DATA,
    /** Upon receiving Recovery Notification, Application has
     * to invoke Receovery API non main/application thread
     * context. */
    UWBD_RECOVERY_NTF,
    /** UWBS shall send SCHED_STATUS_NTF notification
     * when scheduler meets either
     * SESSION_SHEDULER_ATTEMPTS or
     * SESSION_SYNC_ATTEMPTS configuration criteria. */
    UWBD_SCHEDULER_STATUS_NTF,
    /** Session Data Notification */
    UWBD_SESSION_DATA,
    /** rf loopback test data notification */
    UWBD_RF_LOOPBACK_RCV,
    /** multicast list notification */
    UWBD_MULTICAST_LIST_NTF,
    /** Over Temperature reached notification */
    UWBD_OVER_TEMP_REACHED,
    /** Perform Application Clean Up & Restart upon receiving this notification */
    UWBD_ACTION_APP_CLEANUP,
    /** Loopback Status Data */
    UWBD_TEST_MODE_LOOP_BACK_NTF,
    /** PHY LOG NTF */
    UWB_TEST_PHY_LOG_NTF,
    /** TEST RX notification */
    UWBD_TEST_RX_RCV
} eNotificationType;
/* @} */
/**
 * \brief  Structure lists out the ranging measurement information.
 */
/* @{ */
typedef struct
{
    /** Mac address of the participating device, addr can be
     * of short 2 byte or extended 8 byte modes */
    UINT8 mac_addr[MAC_ADDR_LENGTH];
    /** Status */
    UINT8 status;
    /** Indicates if the ranging measurement was in Line of sight or
     * non-line of sight */
    UINT8 nLos;
    /** Distance in centimeters */
    UINT16 distance;
    /** Angle of Arrival preamble for no sts case, STS1 for STS
     * case */
    INT16 aoaFirst;
    /** Angle of Arrival of STS2 if Segmented STS enabled */
    INT16 aoaSecond;
    /** Estimated AoA phase difference  preamble for no sts
     * case, STS1 for STS case */
    INT16 pdoaFirst;
    /** Estimated AoA phase difference STS2 if Segmented STS
     * enabled */
    INT16 pdoaSecond;
    /** CIR sample index from which pdoaFirst is derived */
    INT16 pdoaFirstIndex;
    /** CIR sample index from which pdoaSecond is derived */
    INT16 pdoaSecondIndex;
    /** Destination's aka. Responder's AoA  preamble for no
     * sts case, STS1 for STS case */
    INT16 aoaDestFirst;
    /** Destination's aka. Responder's STS2 */
    INT16 aoaDestSecond;
    /** status to the slot number within the ranging round */
    UINT8 slot_index;
} phRangingMesr_t;
/* @} */

/**
 * \brief  Structure lists out the TDoA ranging measurement information.
 */
/* @{ */
typedef struct
{
    /** Mac address of the participating device,  addr can be
     * of short 2 byte or extended 8 byte modes */
    UINT8 mac_addr[MAC_ADDR_LENGTH];
    /** frame type */
    UINT8 frame_type;
    /** Indicates if the ranging measurement was in Line of sight or
     * non-line of sight */
    UINT8 nLos;
    /** Angle of Arrival preamble for no sts case, STS1 for STS
     * case */
    INT16 aoaFirst;
    /** Angle of Arrival of STS2 if Segmented STS enabled */
    INT16 aoaSecond;
    /** Estimated AoA phase difference  preamble for no sts
     * case, STS1 for STS case */
    INT16 pdoaFirst;
    /** Estimated AoA phase difference STS2 if Segmented STS
     * enabled */
    INT16 pdoaSecond;
    /** CIR sample index from which pdoaFirst is derived */
    INT16 pdoaFirstIndex;
    /** CIR sample index from which pdoaSecond is derived */
    INT16 pdoaSecondIndex;
    /** Timestamp of UWB RFRAME in 15.65ps ticks */
    UINT64 timestamp;
    /** blink frame number received from tag/master anchor */
    UINT32 blink_frame_number;
    /** RSSI field for Antenna 1 */
    UINT16 rssiRX1;
    /** RSSI field for Antenna 2 */
    UINT16 rssiRX2;
    /** Size of Device Specific Information */
    UINT8 device_info_size;
    /** Device Specific Information */
    UINT8 device_info[UCI_MAX_BPRF_PAYLOAD_SIZE];
    /** Size of Blink Payload Data */
    UINT8 blink_payload_size;
    /** Blink Payload Data */
    UINT8 blink_payload_data[UCI_MAX_BPRF_PAYLOAD_SIZE];
} phRangingMesrTdoa_t;
/* @} */

/**
 * \brief  Structure lists out the ranging notification information.
 */
/* @{ */
typedef struct phRangingData
{
    /** keep track of the notifications */
    UINT32 seq_ctr;
    /** Session ID of the ranging round */
    UINT32 sessionId;
    /** RCR Indication */
    UINT8 rcr_indication;
    /** Current ranging interval setting in milli
     * seconds */
    UINT32 curr_range_interval;
    /** Ranging Measurement Type */
    UINT8 ranging_measure_type;
    /** Antenna Pair Info */
    UINT8 antenna_pair_sel;
    /** Mac addr mode indicator, 0: short 2 byte,
     * 1: extended 8 byte mode */
    UINT8 mac_addr_mode_indicator;
    /** Number of ranging measurements */
    UINT8 no_of_measurements;
    /** Ranging measurements array */
    phRangingMesr_t range_meas[MAX_NUM_RESPONDERS];
    /** Ranging measurements TDoA array */
    phRangingMesrTdoa_t range_meas_tdoa;
} phRangingData_t;
/* @} */

/**
 * \brief  Structure lists out the mandatory configurations to be set for
 * ranging.
 */
/* @{ */
typedef struct phRangingParams
{
    /** Device Role
     *
     * - 0x00: Responder
     * - 0x01: Initiator
     * - 0x02: Master Anchor */
    UINT8 deviceRole;
    /** Multi Node Mode,
     *
     * - 0x00: Single device to Single device (Unicast),
     * - 0x01: One to Many,
     * - 0x02: Many to Many,
     * - 0x03: Reserved */
    UINT8 multiNodeMode;
    /** Device Mac Address mode 0:2bytes,1:8 bytes mac addr with
     * 2 byte in header, 2: 8 byte in mac addr and header */
    UINT8 macAddrMode;
    /** No Of Controlees, [1,16] */
    UINT8 noOfControlees;
    /** Device Mac Address, 2byte or 8 byte
     * addr is supported. */
    UINT8 deviceMacAddr[MAC_EXT_ADD_LEN];
    /** Destination MAC address List */
    UINT8 dstMacAddr[MAX_NUM_RESPONDERS * MAC_EXT_ADD_LEN];
    /** Device Type, 0x00: Controlee, 0x01: Controller */
    UINT8 deviceType;

} phRangingParams_t;
/* @} */

typedef enum
{
    kUWB_ChannelNumber_5 = 5,
    kUWB_ChannelNumber_6 = 6,
    kUWB_ChannelNumber_8 = 8,
    kUWB_ChannelNumber_9 = 9,

} UWB_ChannelNumber_t;

typedef enum
{
    kUWB_MacFcsType_CRC16 = 0,
    kUWB_MacFcsType_CRC32 = 1,
} UWB_MacFcsType_t;

typedef enum
{
    kUWB_SfdId_BPRF_0 = 0, //BPRF
    kUWB_SfdId_BPRF_2 = 2, //BPRF
    kUWB_SfdId_HPRF_1 = 1, //HPRF
    kUWB_SfdId_HPRF_2 = 2, //HPRF
    kUWB_SfdId_HPRF_3 = 3, //HPRF
} UWB_SfdId_t;

typedef enum
{
    kUWB_PreambleIndxCode_BPRF_09 = 9, // [9.. 24] BPRF
    kUWB_PreambleIndxCode_BPRF_10 = 10,
    kUWB_PreambleIndxCode_BPRF_11 = 11,
    kUWB_PreambleIndxCode_BPRF_12 = 12,
    kUWB_PreambleIndxCode_BPRF_13 = 13,
    kUWB_PreambleIndxCode_BPRF_14 = 14,
    kUWB_PreambleIndxCode_BPRF_15 = 15,
    kUWB_PreambleIndxCode_BPRF_16 = 16,
    kUWB_PreambleIndxCode_BPRF_17 = 17,
    kUWB_PreambleIndxCode_BPRF_18 = 18,
    kUWB_PreambleIndxCode_BPRF_19 = 19,
    kUWB_PreambleIndxCode_BPRF_20 = 20,
    kUWB_PreambleIndxCode_BPRF_21 = 21,
    kUWB_PreambleIndxCode_BPRF_22 = 22,
    kUWB_PreambleIndxCode_BPRF_23 = 23,
    kUWB_PreambleIndxCode_BPRF_24 = 24,
    kUWB_PreambleIndxCode_HPRF_25 = 25, // [25.. 32] HPRF
    kUWB_PreambleIndxCode_HPRF_26 = 26,
    kUWB_PreambleIndxCode_HPRF_27 = 27,
    kUWB_PreambleIndxCode_HPRF_28 = 28,
    kUWB_PreambleIndxCode_HPRF_29 = 29,
    kUWB_PreambleIndxCode_HPRF_30 = 30,
    kUWB_PreambleIndxCode_HPRF_31 = 31,
    kUWB_PreambleIndxCode_HPRF_32 = 32,
} UWB_PreambleIndxCode_t;

typedef enum
{
    kUWB_PreambleDuration_32Symbols = 0, //Both BPRF and HPRF
    kUWB_PreambleDuration_64Symbols = 1, //BPRF Only
} UWB_PreambleDuration_t;

typedef enum
{
    kUWB_DisableRange_Data_Ntf   = 0x00,
    kUWB_EnableRange_Data_Ntf    = 0x01,
    kUWB_RangeData_Ntf_Proximity = 0x02,
} UWB_Range_DataNtf_t;

typedef enum
{
    kUWB_PrfMode_62_4MHz  = 0, //BPRF
    kUWB_PrfMode_124_8MHz = 1, //HPRF
} UWB_PrfMode_t;

typedef enum
{
    kUWB_PsduDataRate_6_81Mbps = 0,
    kUWB_PsduDataRate_7_80Mbps = 1,
} UWB_PsduDataRate_t;

typedef enum
{
    kUWB_RfFrameConfig_No_Sts               = 0,
    kUWB_RfFrameConfig_Sfd_Sts              = 1,
    kUWB_RfFrameConfig_Psdu_Sts             = 2,
    kUWB_RfFrameConfig_Sfd_Sts_NoPhr_NoPsdu = 3,
} UWB_RfFrameConfig_t;

typedef enum
{
    kUWB_DeviceRole_Responder     = 0,
    kUWB_DeviceRole_Initiator     = 1,
    kUWB_DeviceRole_Master_Anchor = 2,
} UWB_DeviceRole_t;

typedef enum
{
    kUWB_MultiNodeMode_UniCast    = 0,
    kUWB_MultiNodeMode_OnetoMany  = 1,
    kUWB_MultiNodeMode_ManytoMany = 2,
} UWB_MultiNodeMode_t;

typedef enum
{
    kUWB_DeviceType_Controlee  = 0,
    kUWB_DeviceType_Controller = 1,
} UWB_DeviceType_t;

typedef enum
{
    kUWB_RangingMethod_TDoA   = 0,
    kUWB_RangingMethod_SS_TWR = 1,
    kUWB_RangingMethod_DS_TWR = 2,
} UWB_RangingMethod_t;

typedef enum
{
    kUWB_StsConfig_StaticSts             = 0,
    kUWB_StsConfig_DynamicSts            = 1,
    kUWB_StsConfig_DynamicSts_Ctrlee_key = 2,
    kUWB_StsConfig_StatisSts_Tdoa        = 3,
} UWB_StsConfig_t;

/**
 * \brief  Generic callback function registered by Application to receive data
 * from Rhodes SDK. This is a common callback for All session types
 *             1. Ranging
 *             2. App Data Transfer
 *             3. PER Exchange
 *             4. Generic Error Notification
 *             5. Device Reset Notification
 *             6. RFrame Data Notification
 *             7. Recovery Notification
 *             8. Scheduler status Notification
 *
 *             This data is provided by the stack, to the user.  The user should
 *             implement this callback if they want to receive the NTF
 *             information.
 *
 *             - If the callback function pointer
 *             is NULL, the user will not receive the notifications.
 *
 * \param opType - Type of Notification
 * \param pData  - Data received
 */
typedef void(tUwbApi_AppCallback)(eNotificationType opType, void *pData);

/**
 * \brief  Structure lists out the Generic Error notification
 */
/* @{ */
typedef struct phGenericError
{
    /** Status */
    UINT8 status;
} phGenericError_t;
/* @}*/

/**
 * \brief  Structure lists out session information.
 */
/* @{ */
typedef struct phUwbSessionInfo
{
    /** Session Id */
    UINT32 session_id;
    /** Session state */
    UINT8 state;
    /** Reason code */
    UINT8 reason_code;
} phUwbSessionInfo_t;
/* @}*/

/**
 * \brief Structure for storing  Session Data.
 */
/* @{ */
typedef struct phUwbSessionData
{
    /** Session Id */
    UINT32 session_id;
    /** Session Type */
    UINT8 session_type;
    /** Session State */
    UINT8 session_state;
} phUwbSessionData_t;
/* @}*/

/**
 * \brief Structure for storing  Session Context.
 */
/* @{ */
typedef struct phUwbSessionsContext
{
    /** Status */
    UINT8 status;
    /** Session Count */
    UINT8 sessioncnt;
    /** Pointer to Session Data */
    phUwbSessionData_t *pUwbSessionData;
} phUwbSessionsContext_t;
/* @}*/
typedef enum configType
{
    EXTENDED_APP_CONFIG_ID = 0xE3,
} eConfigType;

/**
 * \brief Structure for storing Multicast Controlee List Context.
 */
/* @{ */
typedef struct phMulticastControleeListContext
{
    UINT32 session_id;
    /** Action */
    UINT8 action;
    /** Controlee Count */
    UINT8 no_of_controlees;
    /** Short address list */
    UINT16 short_address_list[MAX_NUM_PHYSICAL_ACCESS_CONTROLEES];
    /** Sub Session ID list */
    UINT32 subsession_id_list[MAX_NUM_PHYSICAL_ACCESS_CONTROLEES];
} phMulticastControleeListContext_t;
/* @}*/

/**
 * \brief Structure for storing Multicast Controlee List Ntf Context.
 */
/* @{ */
typedef struct phMulticastControleeListNtfContext
{
    UINT32 session_id;
    UINT8 remaining_list;
    UINT8 no_of_controlees;
    UINT32 subsession_id_list[MAX_NUM_PHYSICAL_ACCESS_CONTROLEES];
    UINT8 status_list[MAX_NUM_PHYSICAL_ACCESS_CONTROLEES];
} phMulticastControleeListNtfContext_t;
/* @}*/

/** @} */ /* @addtogroup Uwb_Types */

#endif
