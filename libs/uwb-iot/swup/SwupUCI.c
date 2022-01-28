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

//#include <AppInternal.h>
#include <SwupApi.h>
#include "SwupUCI.h"
#include <Swup_update.h>
#include "phNxpLogApis_Swup.h"

#include <phNxpUwb_SpiTransport.h>

#define LOG_TLV(TAG, PTR)                         \
    for (uint8_t i = 0; i < *(PTR + 1); i++) {    \
        ch += sprintf(ch, "%d.", *(PTR + 2 + i)); \
    }                                             \
    if (*(PTR + 1) > 0x00) {                      \
        *(ch - 1) = 0x00;                         \
    }                                             \
    LOG_I("%s: %s", TAG, (char *)&buffer[0]);     \
    memset(buffer, 0, sizeof(buffer));            \
    ch    = &buffer[0];                           \
    (PTR) = (PTR) + (*(PTR + 1)) + 2;

tUWBAPI_STATUS Uci_PrintDeviceInfo(uint32_t *fwVersion, uint32_t *dspVersion)
{
    uint8_t resp[256]                      = {0};
    uint16_t resp_len                      = 0;
    uint8_t uci_command_get_dev_info[]     = {0x20, 0x02, 0x00, 0x00};
    UINT16 uci_command_get_dev_info_length = sizeof(uci_command_get_dev_info);
    int retry_count                        = 0;
    tUWBAPI_STATUS retStatus               = UWBAPI_STATUS_FAILED;
    *fwVersion                             = 0;
    *dspVersion                            = 0;

resend_command:
    if (uci_command_get_dev_info_length !=
        phNxpUwb_UciWrite(uci_command_get_dev_info, uci_command_get_dev_info_length)) {
        goto cleanup;
    }

retry:
    if (retry_count > 4) {
        goto cleanup;
    }
    resp_len = phNxpUwb_UciRead(resp);
    if ((*resp == 0x60) && (*(resp + 1) == 0x01) && (*(resp + 4) == 0xFC)) {
        /* Resend previous frame */
        goto resend_command;
    }
    else if ((*resp == 0x40) && (*(resp + 1) == 0x02) && resp_len > 0) {
        /* Good */
    }
    else {
        phOsalUwb_Delay(100);
        retry_count++;
        goto retry;
    }

    retStatus = UWBAPI_STATUS_OK;
    LOG_I("--------------Device Info--------------");
    char buffer[256]   = {0};
    char *ch           = &buffer[0];
    uint8_t *payload   = resp;
    char deviceName[8] = {0};
    payload            = payload + 4;
    if (*payload != 0x00) {
        LOG_E("Device Status               : 0x%02X", *payload);
        goto cleanup;
    }

    payload++;
    LOG_I("UciVersion             : %d.%d", (*payload), (*(payload + 1)));
    payload += 3;

    LOG_TLV("Middleware Version     ", payload); /* Tag A0 */
    LOG_TLV("A1                     ", payload); /* Tag A0 */
    memcpy(deviceName, payload, *(payload + 1));
    LOG_I("DeviceName             : %s", deviceName); /* Tag E3 */
    payload += *(payload + 1) + 2;
    *fwVersion = (uint32_t)(*(payload + 2) << (8 * 2)) | (*(payload + 3) << (8 * 1)) | (*(payload + 4) << (8 * 0));
    LOG_TLV("FirmwareVersion        ", payload); /* Tag E4 */
    LOG_TLV("DeviceVersion          ", payload); /* Tag E5 */
    LOG_TLV("SerialNumber           ", payload); /* Tag E6 */
    *dspVersion = (uint32_t)(*(payload + 2) << (8 * 2)) | (*(payload + 3) << (8 * 1)) | (*(payload + 4) << (8 * 0));
    LOG_TLV("dspVersion             ", payload); /* Tag E7 */
    LOG_TLV("Ranger4Version         ", payload); /* Tag E8 */
    LOG_TLV("CCCVersion             ", payload); /* Tag E9 */

cleanup:
    return retStatus;
}

tUWBAPI_STATUS Uci_EnableSwup()
{
    uint8_t resp[256] = {0};

    uint16_t resp_len                     = 0;
    uint8_t uci_command_enable_swup[]     = {0x2E, 0x12, 0x00, 0x01, 0x00};
    UINT16 uci_command_enable_swup_length = sizeof(uci_command_enable_swup);
    int retry_count                       = 0;
    tUWBAPI_STATUS retStatus              = UWBAPI_STATUS_FAILED;

resend_command:
    if (uci_command_enable_swup_length != phNxpUwb_UciWrite(uci_command_enable_swup, uci_command_enable_swup_length)) {
        goto cleanup;
    }

retry:
    if (retry_count > 4) {
        goto cleanup;
    }
    resp_len = phNxpUwb_UciRead(resp);
    if (resp_len == 4) {
        goto resend_command;
    }
    if ((*resp == 0x60) && (*(resp + 1) == 0x01) && (*(resp + 4) == 0xFC)) {
        /* Device is in HPD Resend previous frame */
        goto resend_command;
    }
    else if (resp_len == 0x05 && (*resp == 0x4E) && (*(resp + 1) == 0x12)) {
        /* Good. Response from 0.2.0 FW Onwards */
    }
    else if (resp_len == 0x05 && (*resp == 0x40) && (*(resp + 1) == 0x00)) {
        /* Good. Response from 0.1.0 FW */
    }
    else {
        phOsalUwb_Delay(100);
        retry_count++;
        goto retry;
    }

    if (*(resp + 3) == 0x01 && *(resp + 4) == 0x00) {
        /* Good. Success. */
        retStatus = UWBAPI_STATUS_OK;
    }
    else {
        LOG_E("SWUP Activate failed");
        retStatus = UWBAPI_STATUS_FAILED;
    }

cleanup:
    return retStatus;
}

tUWBAPI_STATUS Uci_PrintCoreCapabilities()
{
    uint8_t resp[256] = {0};

    uint16_t resp_len                  = 0;
    uint8_t uci_command_get_caps[]     = {0x20, 0x03, 0x00, 0x00};
    UINT16 uci_command_get_caps_length = sizeof(uci_command_get_caps);
    int retry_count                    = 0;
    tUWBAPI_STATUS retStatus           = UWBAPI_STATUS_FAILED;

resend_cmd:
    if (uci_command_get_caps_length != phNxpUwb_UciWrite(uci_command_get_caps, uci_command_get_caps_length)) {
        goto cleanup;
    }

retry:
    if (retry_count > 4) {
        goto cleanup;
    }
    resp_len = phNxpUwb_UciRead(resp);
    if (resp_len <= 4) {
        goto resend_cmd;
    }
    else if ((*resp == 0x60) && (*(resp + 1) == 0x01) && (*(resp + 4) == 0xFC)) {
        /* Device is in HPD Resend previous frame */
        goto resend_cmd;
    }
    else if (resp_len == 46 && (*resp == 0x40) && (*(resp + 1) == 0x03)) {
        /*Good*/
    }
    else {
        phOsalUwb_Delay(100);
        retry_count++;
        goto retry;
    }

    char buffer[256] = {0};
    char *ch         = &buffer[0];
    uint8_t *payload = resp;
    payload          = payload + 4 + 1;

    LOG_I("--------------Device Core Capabilities--------------");
    LOG_TLV("A0                     ", payload); /* Tag A1 */
    LOG_TLV("A1                     ", payload); /* Tag A1 */
    LOG_TLV("A2                     ", payload); /* Tag A2 */
    LOG_TLV("MAX_PAYLOAD_LEN        ", payload); /* Tag E3 */
    LOG_TLV("MIN_SLOT_LEN           ", payload); /* Tag E4 */
    LOG_TLV("MAX_SESSION_NUM        ", payload); /* Tag E5 */
    LOG_TLV("MAX_ANCHOR_NUM         ", payload); /* Tag E6 */
    LOG_TLV("MIN_UWB_FREQ           ", payload); /* Tag E7 */
    LOG_TLV("MAX_UWB_FREQ           ", payload); /* Tag E8 */

cleanup:
    return retStatus;
}
