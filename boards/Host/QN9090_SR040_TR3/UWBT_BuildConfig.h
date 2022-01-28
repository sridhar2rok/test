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

#ifndef UWBT_TAG_CONFIG_H
#define UWBT_TAG_CONFIG_H

#define DEFAULT_BUILD              0
#define MANUF_TEST_BUILD           1
#define STANDALONE_INITATOR_BUILD  2
#define STANDALONE_RESPONDER_BUILD 3
#define VALIDATION_V3_BUILD        4

#define TAG_BUILD_CFG DEFAULT_BUILD

// TODO: remove qn9090_dev_board support
#define QN9090_DEV_BOARD 1
#define TAG_PROTO        2
// #define TAG_PROTO                   1
#define TAG_V2        3
#define TAG_V3        4
#define BOARD_VERSION TAG_V3

#define PIN_LEVEL_HIGH 1
#define PIN_LEVEL_LOW  0

#define CHIP_VERSION 0x01
#define MW_VERSION   0x02

typedef enum
{
    HELIOS_IRQ,
#if BOARD_VERSION == TAG_V3
    RST_N,
    RDY_N_IRQ,
#endif
    HELIOS_ENABLE,
    HELIOS_SYNC,
    HELIOS_RTC_SYNC,

#if BOARD_VERSION == TAG_PROTO || BOARD_VERSION == TAG_V2
    UWB_ENABLE_1V8,
    MCU_ENABLE_1V8,
    UWB_RF_ENABLE,
#endif

#if BOARD_VERSION == TAG_PROTO
    DIG_ENABLE_1V8,
#endif

#if BOARD_VERSION == TAG_V3
    MCU_ENABLE_1V8,
    LED_BLUE,
    LED_GREEN,
    HELIOS_CS,
#endif

} UWB_GpioComponent_t;

#endif
