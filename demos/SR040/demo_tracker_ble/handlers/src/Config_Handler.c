/*
 * Copyright 2019 NXP.
 *
 * All rights are reserved. Reproduction in whole or in part is
 * prohibited without the written consent of the copyright owner.
 *
 * NXP reserves the right to make changes without notice at any time.
 *
 * NXP makes no warranty, expressed, implied or statutory, including but
 * not limited to any implied warranty of merchantability or fitness for any
 * particular purpose, or that the use will not infringe any third party patent,
 * copyright or trademark. NXP must not be liable for any loss or damage
 * arising from its use.
 */

#include "phUwb_BuildConfig.h"
#include "PrintUtility_Proprietary.h"
#include "UwbApi.h"

#if 1 //(UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_STANDALONE_DEFAULT)

#include "handler.h"
#include "uci_defs.h"
#include "Utilities.h"
#include "PrintUtility_Proprietary.h"
#include "UwbApi.h"

void config_handler(
    UINT8 subType, UINT8 *valueBuffer, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    *pRespTagType = CONFIG_MANAGEMENT;
    switch (subType) {
    case GET_RANGING_PARAMS: {
        phRangingParams_t rangingParam;
        UINT32 sessionId = 0;

        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);

        status = UwbApi_GetRangingParams(sessionId, &rangingParam);

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        if (status == UWBAPI_STATUS_OK) {
            printRangingParams(&rangingParam);
            serializeDataFromRangingParams(&rangingParam, pRespBuf, pRespSize);
            *pRespSize = (UINT16)(*pRespSize + sizeof(status));
        }
        else {
            *pRespSize = sizeof(status);
        }
    } break;
    case GET_APP_CONFIG: {
        UINT32 sessionId = 0;
        UINT16 paramType = 0;
        UINT32 paramValue;
        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);
        UWB_STREAM_TO_UINT16(paramType, valueBuffer);

        status = UwbApi_GetAppConfig(sessionId, paramType, &paramValue);

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        UWB_UINT32_TO_STREAM(pRespBuf, paramValue);
        if (status == UWBAPI_STATUS_OK) {
            *pRespSize = (UINT16)(sizeof(paramValue) + sizeof(status));
        }
        else {
            *pRespSize = sizeof(status);
        }
    } break;
    case GET_DEBUG_PARAMS: {
#if UWBIOT_UWBD_SR100T
        phDebugParams_t debugParam;
        UINT32 sessionId = 0;
        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);

        status = UwbApi_GetDebugParams(sessionId, &debugParam);

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        if (status == UWBAPI_STATUS_OK) {
            printDebugParams(&debugParam);
            serializeDataFromDebugParams(&debugParam, pRespBuf, pRespSize);
            *pRespSize = (UINT16)(*pRespSize + sizeof(status));
        }
        else {
            *pRespSize = sizeof(status);
        }
#elif UWBIOT_UWBD_SR040
        status = UWBAPI_STATUS_INVALID_PARAM; // UWBAPI not supported on SR040
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#endif
    } break;
    case GET_PER_PARAMS: {
#if UWBIOT_UWBD_SR100T
        phRfTestParams_t rfTestParam;
        UINT32 sessionId = 0;

        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);

        status = UwbApi_GetRfTestParams(sessionId, &rfTestParam);

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        if (status == UWBAPI_STATUS_OK) {
            serializeDataFromPerParams(&rfTestParam, pRespBuf, pRespSize);
            *pRespSize = (UINT16)(*pRespSize + sizeof(status));
        }
        else {
            *pRespSize = sizeof(status);
        }
#elif UWBIOT_UWBD_SR040
        status     = UWBAPI_STATUS_INVALID_PARAM; // UWBAPI not supported on SR040
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#endif
    } break;
    case SET_RANGING_PARAMS: {
        phRangingParams_t rangingParam;
        UINT32 sessionId = 0;
        UINT8 addrLen    = (UINT8)SHORT_MAC_ADDRESS;

        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);
        UWB_STREAM_TO_UINT8(rangingParam.deviceRole, valueBuffer);
        UWB_STREAM_TO_UINT8(rangingParam.multiNodeMode, valueBuffer);
#if UWBIOT_UWBD_SR100T
        UWB_STREAM_TO_UINT8(rangingParam.macAddrMode, valueBuffer);
#endif
        UWB_STREAM_TO_UINT8(rangingParam.noOfControlees, valueBuffer);
#if UWBIOT_UWBD_SR100T
        if (rangingParam.macAddrMode == SHORT_MAC_ADDRESS) {
            addrLen = (UINT8)MAC_SHORT_ADD_LEN;
        }
        else if (rangingParam.macAddrMode == EXTENDED_MAC_ADDRESS ||
                 rangingParam.macAddrMode == EXTENDED_MAC_ADDRESS_AND_HEADER) {
            addrLen = (UINT8)MAC_EXT_ADD_LEN;
        }
#elif UWBIOT_UWBD_SR040
        addrLen    = (UINT8)MAC_SHORT_ADD_LEN;
#endif

        UWB_STREAM_TO_ARRAY(rangingParam.deviceMacAddr, valueBuffer, addrLen);
        UWB_STREAM_TO_ARRAY(rangingParam.dstMacAddr, valueBuffer, rangingParam.noOfControlees * addrLen);
        UWB_STREAM_TO_UINT8(rangingParam.deviceType, valueBuffer);

        status = UwbApi_SetRangingParams(sessionId, &rangingParam);

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
    } break;
    case SET_APP_CONFIG: {
        UINT32 sessionId  = 0;
        UINT16 paramType  = 0;
        UINT32 paramValue = 0;

        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);
        UWB_STREAM_TO_UINT16(paramType, valueBuffer);
        UWB_STREAM_TO_UINT32(paramValue, valueBuffer);

        status = UwbApi_SetAppConfig(sessionId, paramType, paramValue);

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
    } break;
    case GET_TEST_CONFIG: {
#if UWBIOT_UWBD_SR100T
        UINT16 paramType  = 0;
        UINT32 paramValue = 0xFF;
        UWB_STREAM_TO_UINT16(paramType, valueBuffer);

        status = UwbApi_GetTestConfig(paramType, &paramValue);

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        UWB_UINT32_TO_STREAM(pRespBuf, paramValue);
        *pRespSize = (UINT16)(sizeof(paramValue) + sizeof(status));

#elif UWBIOT_UWBD_SR040
        status     = UWBAPI_STATUS_INVALID_PARAM; // UWBAPI not supported on SR040
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#endif
    } break;
    case SET_TEST_CONFIG: {
#if UWBIOT_UWBD_SR100T
        UINT16 paramType  = 0;
        UINT32 paramValue = 0;

        UWB_STREAM_TO_UINT16(paramType, valueBuffer);
        UWB_STREAM_TO_UINT32(paramValue, valueBuffer);

        status = UwbApi_SetTestConfig(paramType, paramValue);

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#elif UWBIOT_UWBD_SR040
        status     = UWBAPI_STATUS_INVALID_PARAM; // UWBAPI not supported on SR040
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#endif
    } break;
    case SET_DEBUG_PARAMS: {
#if UWBIOT_UWBD_SR100T
        phDebugParams_t debugParam;
        UINT32 sessionId = 0;

        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);
        UWB_STREAM_TO_UINT16(debugParam.secureThread, valueBuffer);
        UWB_STREAM_TO_UINT16(debugParam.secureIsrThread, valueBuffer);
        UWB_STREAM_TO_UINT16(debugParam.nonSecureIsrThread, valueBuffer);
        UWB_STREAM_TO_UINT16(debugParam.shellThread, valueBuffer);
        UWB_STREAM_TO_UINT16(debugParam.phyThread, valueBuffer);
        UWB_STREAM_TO_UINT16(debugParam.rangingThread, valueBuffer);
        UWB_STREAM_TO_UINT8(debugParam.dataLoggerNtf, valueBuffer);
        UWB_STREAM_TO_UINT8(debugParam.cirLogNtf, valueBuffer);
        UWB_STREAM_TO_UINT8(debugParam.psduLogNtf, valueBuffer);
        UWB_STREAM_TO_UINT8(debugParam.rframeLogNtf, valueBuffer);
        status = UwbApi_SetDebugParams(sessionId, &debugParam);

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#elif UWBIOT_UWBD_SR040
        status     = UWBAPI_STATUS_INVALID_PARAM; // UWBAPI not supported on SR040
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#endif
    } break;
    case SET_RF_TEST_PARAM: {
#if UWBIOT_UWBD_SR100T
        phRfTestParams_t rfTestParam;
        UINT32 sessionId = 0;

        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);
        UWB_STREAM_TO_UINT32(rfTestParam.numOfPckts, valueBuffer);
        UWB_STREAM_TO_UINT32(rfTestParam.tGap, valueBuffer);
        UWB_STREAM_TO_UINT32(rfTestParam.tStart, valueBuffer);
        UWB_STREAM_TO_UINT32(rfTestParam.tWin, valueBuffer);
        UWB_STREAM_TO_UINT8(rfTestParam.randomizedSize, valueBuffer);
        UWB_STREAM_TO_UINT16(rfTestParam.rawPhr, valueBuffer);
        UWB_STREAM_TO_UINT32(rfTestParam.rmarkerTxStart, valueBuffer);
        UWB_STREAM_TO_UINT32(rfTestParam.rmarkerRxStart, valueBuffer);
        UWB_STREAM_TO_UINT8(rfTestParam.stsIndexAutoIncr, valueBuffer);
        UWB_STREAM_TO_UINT8(rfTestParam.macCfg, valueBuffer);

        status = UwbApi_SetRfTestParams(sessionId, &rfTestParam);

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
    } break;

    case SET_STATIC_STS: {
        UINT32 sessionId = 0;
        UINT16 vendorId  = 0;
        UINT8 staticStsIv[UCI_PARAM_LEN_STATIC_STS_IV];

        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);
        UWB_STREAM_TO_UINT16(vendorId, valueBuffer);
        UWB_STREAM_TO_ARRAY(staticStsIv, valueBuffer, UCI_PARAM_LEN_STATIC_STS_IV);

        status = UwbApi_SetStaticSts(sessionId, vendorId, staticStsIv);

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#elif UWBIOT_UWBD_SR040
        status     = UWBAPI_STATUS_INVALID_PARAM; // UWBAPI not supported on SR040
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#endif
    } break;

    default:
        *pRespSize = sizeof(status);
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        PRINTF("%s: Unknown Command\n", __func__);
        break;
    }
}

#endif
