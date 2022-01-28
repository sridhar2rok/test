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

#ifndef UWBAPI_PROPRIETARY_INTERNAL_H
#define UWBAPI_PROPRIETARY_INTERNAL_H

#include "UwbCore_Types.h"
#include "UwbApi_Types.h"
#include "UwbApi_Types_Proprietary.h"

#if defined(ENABLE_NFC) && (ENABLE_NFC == TRUE)
#include "UwbSeApi.h"
void reset_se_on_error(void);
#endif

UINT8 getExtAppConfigTLVBuffer(UINT8 paramId, void *paramValue, UINT8 *tlvBuffer);
UINT8 getExtDebugConfigTLVBuffer(UINT8 paramId, void *paramValue, UINT8 *tlvBuffer);
UINT8 getExtDeviceConfigTLVBuffer(UINT8 paramId, void *paramValue, UINT8 *tlvBuffer);
UINT8 getExtTestConfigTLVBuffer(UINT8 paramId, void *paramValue, UINT8 *tlvBuffer);
void parseDebugParams(UINT8 *rspPtr, UINT8 noOfParams, phDebugParams_t *pDebugParams);
void parseUwbSessionParams(UINT8 *rspPtr, phUwbSessionsContext_t *pUwbSessionsContext);
void extDeviceManagementCallback(UINT8 event, uint16_t paramLength, UINT8 *pResponseBuffer);
bool parseDeviceInfo(UINT8 *manfacturerData, UINT8 manufacturerLength);
tUWBAPI_STATUS setDefaultCoreConfigs(void);

#endif // UWBAPI_PROPRIETARY_INTERNAL_H
