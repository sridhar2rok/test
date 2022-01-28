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

void ranging_handler(
    UINT8 subType, UINT8 *valueBuffer, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    *pRespTagType = RANGE_MANAGEMENT;
    switch (subType) {
    case START_RANGING_SESSION: {
        UINT32 sessionId = 0;

        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);

        status = UwbApi_StartRangingSession(sessionId);

        if (status == UWBAPI_STATUS_OK)
            mState = ranging;

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
    } break;
    case STOP_RANGING_SESSION: {
        UINT32 sessionId = 0;

        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);

        status = UwbApi_StopRangingSession(sessionId);

        if (status == UWBAPI_STATUS_OK)
            mState = sessionCreated;

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
    } break;
    case ENABLE_RANGING_DATA_NOTIFICATION: {
        UINT32 sessionId = 0;
        UINT8 state      = 0;
        UINT16 proximityNear, proximityFar;

        UWB_STREAM_TO_UINT32(sessionId, valueBuffer);
        UWB_STREAM_TO_UINT8(state, valueBuffer);
        UWB_STREAM_TO_UINT16(proximityNear, valueBuffer);
        UWB_STREAM_TO_UINT16(proximityFar, valueBuffer);
        status = UwbApi_EnableRangingDataNtf(sessionId, state, proximityNear, proximityFar);
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
    } break;
    case SESSION_UPDATE_MULTICAST_LIST: {
        phMulticastControleeListContext_t multicastControleeListContext;

        UWB_STREAM_TO_UINT32(multicastControleeListContext.session_id, valueBuffer);
        UWB_STREAM_TO_UINT8(multicastControleeListContext.action, valueBuffer);
        UWB_STREAM_TO_UINT8(multicastControleeListContext.no_of_controlees, valueBuffer);
        for (UINT8 i = 0; i < multicastControleeListContext.no_of_controlees; i++) {
            UWB_STREAM_TO_UINT16(multicastControleeListContext.short_address_list[i], valueBuffer);
        }

        for (UINT8 i = 0; i < multicastControleeListContext.no_of_controlees; i++) {
            UWB_STREAM_TO_UINT32(multicastControleeListContext.subsession_id_list[i], valueBuffer);
        }

        status = UwbApi_UpdateControllerMulticastList(&multicastControleeListContext);

        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
    } break;
    default:
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
        PRINTF("%s: Unknown Command\n", __func__);
        break;
    }
}

#endif
