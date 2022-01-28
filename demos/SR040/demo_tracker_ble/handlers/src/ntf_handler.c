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

void ntf_handler(
    UINT8 subType, UINT8 *valueBuffer, UINT16 length, UINT16 *pRespSize, UINT8 *pRespBuf, UINT8 *pRespTagType)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    *pRespTagType = UWB_NTF_MANAGEMENT;

    switch (subType) {
    case PER_TX_NTF:
    case PER_RX_NTF:
    case RF_LOOPBACK_NTF:
    case RANGING_DATA_NTF:
    case RFRAME_DATA_NTF:
    case SKD_STATUS_NTF:
    case SESSION_STATUS_NTF:
    case NTF_HPD_WAKEUP:
    case MULTICAST_LIST_NTF: {
        *pRespSize = length;
        memcpy(pRespBuf, valueBuffer, length);
    } break;
    default:
        UWB_UINT8_TO_STREAM(pRespBuf, status);
        *pRespSize = sizeof(status);
        PRINTF("%s: Unknown Command\n", __func__);
        break;
    }
}

#endif
