/* Copyright 2019,2020 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */

#include "phUwb_BuildConfig.h"

#if 1 //(UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_STANDALONE_DEFAULT)

#ifndef _UWB_TLV_HANDLERS_H_
#define _UWB_TLV_HANDLERS_H_

#include "PrintUtility.h"
#include "UwbApi.h"
#include "uwb_types.h"
#include "TLV_Defs.h"

typedef enum
{
    uninit,
    init,
    sessionCreated,
    ranging
} UwbHandlerState;

// MK: Left for compatibility
#define SWITCH_HELIOS(on)

void session_handler(
    UINT8 subType, UINT8 *value, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType);
void config_handler(
    UINT8 subType, UINT8 *value, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType);
void ranging_handler(
    UINT8 subType, UINT8 *value, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType);
#if UWBIOT_UWBD_SR100T
void rf_test_handler(
    UINT8 subType, UINT8 *value, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType);
#endif
void uwb_misc_handler(
    UINT8 subType, UINT8 *value, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType);
void se_handler(
    UINT8 subType, UINT8 *valueBuffer, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType);
void ntf_handler(
    UINT8 subType, UINT8 *valueBuffer, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType);
void setup_handler(
    UINT8 subType, UINT8 *valueBuffer, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType);

#endif

#endif
