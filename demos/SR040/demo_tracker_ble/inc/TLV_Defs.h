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

#include "stdio.h"

#ifndef STARTUP_INC_TLV_DEFS_H_
#define STARTUP_INC_TLV_DEFS_H_

#define TLV_MAX_VALUE_SIZE 2049
#define TLV_VALUE_OFFSET   3
// TLV header + 1 byte for TLV subtype + payload
#define TLV_MAX_TOTAL_SIZE (TLV_MAX_VALUE_SIZE + TLV_VALUE_OFFSET + 1)

/* TLV TYPES */
#define CMD_TYPE_OFFSET     0x00
#define CMD_SUB_TYPE_OFFSET 0x03

#define LSB_LENGTH_OFFSET 0x02
#define MSB_LENGTH_OFFSET 0x01
#define MSB_LENGTH_MASK   0x08

#define SESSION_MANAGEMENT   0x01
#define CONFIG_MANAGEMENT    0x02
#define RANGE_MANAGEMENT     0x03
#define DATA_MANAGEMENT      0x04
#define RF_TEST_MANAGEMENT   0x05
#define UWB_MISC             0x06
#define SE_MANAGEMENT        0x07
#define UWB_NTF_MANAGEMENT   0x08
#define UWB_SETUP_MANAGEMENT 0x09

#define RHODES_CMD 0x10

#define UI_CMD            0x40
#define UI_RSP            0x41
#define CONFIG_DEVICE_CMD 0x50
#define CONFIG_DEVICE_RSP 0x51
#define TLV_TYPE_END      0x52 // Always keep END as last type + 1

#define SESSION_INIT       0x10
#define SESSION_DE_INIT    0x11
#define GET_SESSION_STATUS 0x12
/* TLV_MW_CMD subtypes */

#define SET_RANGING_PARAMS 0x20
#define GET_RANGING_PARAMS 0x21
#define SET_APP_CONFIG     0x22
#define GET_APP_CONFIG     0x23
#define SET_DEBUG_PARAMS   0x24
#define GET_DEBUG_PARAMS   0x25
#define SET_RF_TEST_PARAM  0x26
#define GET_PER_PARAMS     0x27
#define GET_TEST_CONFIG    0x28
#define SET_TEST_CONFIG    0x29
#define SET_STATIC_STS     0x2A
/* TLV_MW_NTF subtypes */

#define START_RANGING_SESSION            0X30
#define STOP_RANGING_SESSION             0X31
#define ENABLE_RANGING_DATA_NOTIFICATION 0X33
#define SESSION_UPDATE_MULTICAST_LIST    0x35
/* TLV_RHODES_CMD subtypes */

#define START_PER_TX  0X50
#define START_PER_RX  0X51
#define STOP_RF_TEST  0X52
#define LOOPBACK_TEST 0x53
/* Raw TLV subtypes */

#define GET_STACK_CAPABILITIES 0X60
#define GET_UWB_DEVICE_STATE   0x61
#define SEND_RAW_COMMAND       0X63
#define GET_ALL_UWB_SESSIONS   0x64
#define SET_CALIBRATION        0x65
#define GET_CALIBRATION        0x66
#define DO_CALIBRATION         0x67
#define QUERY_TEMPERATURE      0x68
#define GET_SOFTWARE_VERSION   0x69
#define GET_BOARD_ID           0x6A
/* TLV_UI subtypes */

#define SE_TRANSRECEIVE       0x71
#define SE_OPEN_CHANNEL       0x72
#define SE_TEST_LOOP          0x73
#define SE_TEST_CONNECTIVITY  0x74
#define SE_DO_BIND            0x75
#define SE_GET_BINDING_STATUS 0x76
#define SE_GET_BINDING_COUNT  0x77
/* TLV_CONFIG subtypes */

#define RANGING_DATA_NTF   0x81
#define PER_TX_NTF         0x84
#define PER_RX_NTF         0x85
#define CIR0_NTF           0x86
#define CIR1_NTF           0x87
#define DATALOGGER_NTF     0x88
#define DEBUG_LOG_NTF      0x89
#define RFRAME_DATA_NTF    0x8A
#define SKD_STATUS_NTF     0x8B //MK: Changed name because it conflicts with another defined variable
#define SESSION_STATUS_NTF 0x8C
#define RF_LOOPBACK_NTF    0x8D
#define MULTICAST_LIST_NTF 0x8E
#define NTF_HPD_WAKEUP     0x8F
/* TLV_BOOTLOADER subtypes */

#define UWB_INIT          0x91
#define UWB_SHUTDOWN      0x92
#define UWB_FACTORY_TEST  0x93
#define UWB_MAINLINE_TEST 0x94
#define MCU_RESET         0x95 // MCU nvReset will be called.
/* TLV_LOOP_BACK subtypes */

#define RHODES_PLATFORM_INFO 0x04

/* UI commands */
#define SET_GPIO_STATE   0x01
#define UI_BUZZER_TONE   0x02
#define GET_GPIO_STATE   0x04
#define UI_BUZZER_MELODY 0x05
#define SET_GPIO_PERIOD  0x07

/* Device configurations commands */
#define SET_CONFIG  0x01
#define GET_CONFIG  0x02
#define DEVICE_PING 0x03

/* Bootloader commands */

typedef struct
{
    uint8_t type;
    uint16_t size;
    uint8_t *value;
} tlv_t;

#endif /* STARTUP_INC_TLV_DEFS_H_ */
