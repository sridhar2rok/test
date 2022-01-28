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

#if 1 //(UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_STANDALONE_DEFAULT)

#include "handler.h"

extern UwbHandlerState mState;
extern uint32_t mSessionId;

void session_handler(
    UINT8 subType, UINT8 *valueBuffer, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    *pRespTagType = SESSION_MANAGEMENT;
    switch (subType) {
    case SESSION_INIT: {
        UINT32 sessionId  = 0;
        UINT8 sessionType = 0xFF;

        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);
        UWB_STREAM_TO_UINT8(sessionType, valueBuffer);

        status = UwbApi_SessionInit(sessionId, sessionType);

        if (status == UWBAPI_STATUS_OK) {
            mState     = sessionCreated;
            mSessionId = sessionId;
        }

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
    } break;
    case SESSION_DE_INIT: {
        UINT32 sessionId = 0;

        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);

        status = UwbApi_SessionDeinit(sessionId);

        if (status == UWBAPI_STATUS_OK) {
            UwbApi_SuspendDevice();
            mState = init;
        }

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
    } break;
    case GET_SESSION_STATUS: {
        UINT32 sessionId    = 0;
        UINT8 sessionStatus = 0;

        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);

        status = UwbApi_GetSessionState(sessionId, &sessionStatus);
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
        UWB_UINT8_TO_STREAM(pRespBuf, sessionStatus);
        *pRespSize = (UINT16)(*pRespSize + sizeof(sessionStatus));
    } break;
    default:
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
        PRINTF("%s: Unknown Command\n", __func__);
        break;
    }
}

#endif
