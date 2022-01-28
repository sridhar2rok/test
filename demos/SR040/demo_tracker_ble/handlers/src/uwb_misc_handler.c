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
#include "UwbApi.h"

#if 1 //(UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_STANDALONE_DEFAULT)

#include "handler.h"
#include "TLV_Defs.h"
#include "Utilities.h"
#include "board.h"

#define MAX_SESSIONS 5

void uwb_misc_handler(
    UINT8 subType, UINT8 *valueBuffer, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    *pRespTagType = UWB_MISC;

    switch (subType) {
    case GET_STACK_CAPABILITIES: {
        phUwbDevInfo_t stackInfo;

        status = UwbApi_GetStackCapabilities(&stackInfo);
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        if (status == UWBAPI_STATUS_OK) {
            serializeStackInfo(&stackInfo, pRespBuf, pRespSize);
            *pRespSize = (UINT16)(*pRespSize + sizeof(status));
        }
        else {
            *pRespSize = sizeof(status);
        }
    } break;
    case GET_UWB_DEVICE_STATE: {
        UINT8 deviceState = 0;
        status            = UwbApi_GetUwbDevState(&deviceState);
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        UWB_UINT8_TO_STREAM(pRespBuf, deviceState);
        *pRespSize = sizeof(status) + sizeof(deviceState);
    } break;
    case SEND_RAW_COMMAND: {
        *pRespSize = (TLV_MAX_VALUE_SIZE - TLV_VALUE_OFFSET - 1); // 3 byte hdr + 1 byte subtype
        status     = UwbApi_SendRawCommand(valueBuffer, length, &pRespBuf[1], pRespSize);
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = (UINT16)(*pRespSize + sizeof(status));
    } break;
    case GET_ALL_UWB_SESSIONS: {
#if UWBIOT_UWBD_SR100T
        phUwbSessionData_t UwbSessionData[MAX_SESSIONS];
        phUwbSessionsContext_t uwbSessionContext;
        uwbSessionContext.pUwbSessionData            = UwbSessionData;
        phUwbSessionsContext_t *phUwbSessionsContext = &uwbSessionContext;
        status                                       = UwbApi_GetAllUwbSessions(phUwbSessionsContext);

        UWB_UINT8_TO_STREAM(pRespBuf, phUwbSessionsContext->status);
        UWB_UINT8_TO_STREAM(pRespBuf, phUwbSessionsContext->sessioncnt);

        *pRespSize = sizeof(phUwbSessionsContext->status);
        *pRespSize = (UINT16)(*pRespSize + sizeof(phUwbSessionsContext->sessioncnt));

        for (UINT8 i = 0; i < phUwbSessionsContext->sessioncnt; i++) {
            UWB_UINT32_TO_STREAM(pRespBuf, phUwbSessionsContext->pUwbSessionData[i].session_id);
            UWB_UINT8_TO_STREAM(pRespBuf, phUwbSessionsContext->pUwbSessionData[i].session_type);
            UWB_UINT8_TO_STREAM(pRespBuf, phUwbSessionsContext->pUwbSessionData[i].session_state);

            *pRespSize = (UINT16)(*pRespSize + sizeof(phUwbSessionsContext->pUwbSessionData->session_id));
            *pRespSize = (UINT16)(*pRespSize + sizeof(phUwbSessionsContext->pUwbSessionData->session_type));
            *pRespSize = (UINT16)(*pRespSize + sizeof(phUwbSessionsContext->pUwbSessionData->session_state));
        }
#elif UWBIOT_UWBD_SR040
        status = UWBAPI_STATUS_INVALID_PARAM; // UWBAPI not supported on SR040
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#endif
    } break;
    case SET_CALIBRATION: {
#if UWBIOT_UWBD_SR100T
        UINT8 channel    = 9;
        UINT8 calibParam = 0xFF;
        UINT8 len        = (UINT8)(length - (sizeof(channel) + sizeof(calibParam)));

        UWB_STREAM_TO_UINT8(channel, valueBuffer);
        UWB_STREAM_TO_UINT8(calibParam, valueBuffer);

        status = UwbApi_SetCalibration(channel, calibParam, valueBuffer, len);
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#elif UWBIOT_UWBD_SR040
        status     = UWBAPI_STATUS_INVALID_PARAM; // UWBAPI not supported on SR040
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#endif
        break;
    }
    case GET_CALIBRATION: {
#if UWBIOT_UWBD_SR100T
        UINT8 channel    = 9;
        UINT8 calibParam = 0xFF;
        phCalibRespStatus_t calibresp;

        UWB_STREAM_TO_UINT8(channel, valueBuffer);
        UWB_STREAM_TO_UINT8(calibParam, valueBuffer);

        status = UwbApi_GetCalibration(channel, calibParam, &calibresp);
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        UWB_ARRAY_TO_STREAM(pRespBuf, calibresp.calibValueOut, calibresp.length);
        *pRespSize = (UINT16)(calibresp.length + sizeof(status));

#elif UWBIOT_UWBD_SR040
        status     = UWBAPI_STATUS_INVALID_PARAM; // UWBAPI not supported on SR040
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#endif
        break;
    }
    case DO_CALIBRATION: {
#if UWBIOT_UWBD_SR100T

        UINT8 channel    = 9;
        UINT8 calibParam = 0xFF;
        phCalibRespStatus_t calibresp;

        UWB_STREAM_TO_UINT8(channel, valueBuffer);
        UWB_STREAM_TO_UINT8(calibParam, valueBuffer);

        status = UwbApi_DoCalibration(channel, calibParam, &calibresp);
        serializeDataFromDoCalibrationNtf(&calibresp, pRespBuf, pRespSize);

#elif UWBIOT_UWBD_SR040
        status     = UWBAPI_STATUS_INVALID_PARAM; // UWBAPI not supported on SR040
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#endif
        break;
    }
    case QUERY_TEMPERATURE: {
#if UWBIOT_UWBD_SR100T

        UINT8 tempValue;

        status = UwbApi_QueryTemperature(&tempValue);
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        UWB_UINT8_TO_STREAM(pRespBuf, tempValue);
        *pRespSize = (UINT16)(sizeof(status) + sizeof(tempValue));
#elif UWBIOT_UWBD_SR040
        status     = UWBAPI_STATUS_INVALID_PARAM; // UWBAPI not supported on SR040
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
#endif
        break;
    }
    case GET_SOFTWARE_VERSION: {
        UWB_UINT8_TO_STREAM(pRespBuf, RHODES_MW_MAJOR_VERSION);
        UWB_UINT8_TO_STREAM(pRespBuf, RHODES_MW_MINOR_VERSION);

        *pRespSize = (UINT16)(2 * sizeof(uint8_t));
        break;
    }
    case GET_BOARD_ID: {
        uint8_t len = 0;
        uint8_t boardId[16];

        BOARD_GetMCUUid(boardId, &len);

        UWB_ARRAY16_TO_STREAM(pRespBuf, boardId);
        *pRespSize = (UINT16)(sizeof(boardId));
        break;
    }
    default:
        *pRespSize = sizeof(status);
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        PRINTF("%s: Unknown Command\n", __func__);
        break;
    }
}

#endif
