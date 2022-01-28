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

#define DDFS_TONE_CONFIG_ENABLE_VALUE 1

/**
 * DDFS_TONE_CONFIG - 0xFFu
 *
 * Direct Digital Frequency Synthesizer(DDFS) Tone configuration.
 * 18 bytes value description: To be repeated for 4 blocks
 * Octet[0]: channel number. default - 0x05
 * Octet[1]: Tx antenna selection. Possible values are 1 or 2. default - 0x01
 * Octet[5:2]: Content of register TX_DDFS_TONE_0. default - 0x00010E
 * Octet[9:6]: Content of register TX_DDFS_TONE_1. default - 0x00013E
 * Octet[13:10]: Duration of the spur, in 124.8 MHz resolution (~ 8 ns). default - 0x01F4
 * Octet[14]: Content of register GAINVAL_SET. default - 0x26
 * Octet[15]: Content of register DDFSGAINBYPASS_ENBL. default - 0x00
 * Octet[17:16]: Periodicity of spur in terms of gap interval in the PER command. default - 0x0064
 */

/* clang-format off */
#define DDFS_TONE_CONFIG_VALUE_CHANNEL5  \
    0x05,                        \
    0x01,                        \
    0x0E, 0x01, 0x00, 0x00,      \
    0x3E, 0x01, 0x00, 0x00,      \
    0xF4, 0x01, 0x00, 0x00,      \
    0x26,                        \
    0x00,                        \
    0x64, 0x00                   \

#define DDFS_TONE_CONFIG_VALUE_CHANNEL6  \
    0x06,                        \
    0x01,                        \
    0x0E, 0x01, 0x00, 0x00,      \
    0x3E, 0x01, 0x00, 0x00,      \
    0xF4, 0x01, 0x00, 0x00,      \
    0x26,                        \
    0x00,                        \
    0x64, 0x00                   \

#define DDFS_TONE_CONFIG_VALUE_CHANNEL8  \
    0x08,                        \
    0x01,                        \
    0x0E, 0x01, 0x00, 0x00,      \
    0x3E, 0x01, 0x00, 0x00,      \
    0xF4, 0x01, 0x00, 0x00,      \
    0x26,                        \
    0x00,                        \
    0x64, 0x00                   \

#define DDFS_TONE_CONFIG_VALUE_CHANNEL9  \
    0x09,                        \
    0x01,                        \
    0x0E, 0x01, 0x00, 0x00,      \
    0x3E, 0x01, 0x00, 0x00,      \
    0xF4, 0x01, 0x00, 0x00,      \
    0x26,                        \
    0x00,                        \
    0x64, 0x00                   \

/* very basic compile time check of invalid combinations */

/* clang-format on */

tUWBAPI_STATUS do_EnableDDFSConfigs(void)
{
    tUWBAPI_STATUS status;
    LOG_I("DO:%s", __FUNCTION__);

    UINT8 cmd[255] = {0x20, 0x04, 0x00};
    UINT8 rsp[100];
    UINT16 respLen = sizeof(rsp);
    UINT16 cmdLen;
    status = UWBAPI_STATUS_FAILED;

    const uint8_t ENABLE_DDFS = 1;

    const uint8_t enableDDFS[] = {
        1 /* No of Params */,
        UCI_EXT_PARAM_ID_DDFS_TONE_CONFIG_ENABLE,
        1 /* len : DEBUG_LOG_LEVEL */,
        ENABLE_DDFS /* Enable : DEBUG_LOG_LEVEL */,
    };
    cmd[3] = sizeof(enableDDFS);
    cmdLen = cmd[3] + 4; /* payload + UCI Header length*/
    phOsalUwb_MemCopy(&cmd[4], enableDDFS, sizeof(enableDDFS));

    LOG_I("ENABLE_DDFS=%d", ENABLE_DDFS);
    status = UwbApi_SendRawCommand(cmd, cmdLen, rsp, &respLen);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UWB_SetCoreConfig(Enable enableDDFS) Failed");
        goto exit;
    }

    cmd[0] = 0x2E;
    cmd[1] = 0x26;
    cmd[2] = 0;

    const UINT8 ddfs_tone_config_values[] = {
        DDFS_TONE_CONFIG_VALUE_CHANNEL5,
        DDFS_TONE_CONFIG_VALUE_CHANNEL6,
        DDFS_TONE_CONFIG_VALUE_CHANNEL8,
        DDFS_TONE_CONFIG_VALUE_CHANNEL9,
    };
    cmd[3] = 3 + sizeof(ddfs_tone_config_values);
    cmd[4] = 1;                               /* N Parameters */
    cmd[5] = DDFS_TONE_VALUES;                /* N Parameters */
    cmd[6] = sizeof(ddfs_tone_config_values); /* N Parameters */
    phOsalUwb_MemCopy(&cmd[7], ddfs_tone_config_values, sizeof(ddfs_tone_config_values));
    cmdLen  = cmd[3] + 4; /* payload + UCI Header length*/
    respLen = sizeof(rsp);

    LOG_I("SET_DDFS");
    status = UwbApi_SendRawCommand(cmd, cmdLen, rsp, &respLen);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UWB_ProperitoryCommand(SET_DDFS) Failed");
        goto exit;
    }

exit:
    return status;
}
