/*
*
* Copyright 2020 NXP.
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

#include "SR040_RadioCfg_CRCTable.h"
#include "phNxpLogApis_App.h"
#include "UwbApi_Proprietary.h"

/**
 * \brief  Structure indicates the Radio config crc elements.
 */
typedef struct RadioConfigCrcTable
{
    UINT8 index;
    UINT16 crc;
} RadioConfigCrcTable_t;

/**
 * brief Checks Crc's of all indexes.
 *
 * \retval #UWBAPI_STATUS_OK       - Returns SUCCESS if values are "SAME" as
 *                                   expected as per the current release.
 * \retval #UWBAPI_STATUS_FAILED   - otherwise
 */
EXTERNC tUWBAPI_STATUS RadioConfig_CheckCrc(void)
{
    const RadioConfigCrcTable_t radioConfigCrcTable[] = SR040_RADIO_CONFIG_CRC_tdoaAndTracker;
    int loopIndex;
    const int noOfIndexes = sizeof(radioConfigCrcTable) / sizeof(radioConfigCrcTable[0]);
    tUWBAPI_STATUS status;
    UINT16 expectedCrc;

    /* Check the crc's of all the blocks are correct or not */
    for (loopIndex = 0; loopIndex < noOfIndexes; ++loopIndex) {
        status = UwbApi_RadioConfigGetCrc(radioConfigCrcTable[loopIndex].index, &expectedCrc);
        if (status != UWBAPI_STATUS_OK) {
            NXPLOG_APP_E("UwbApi_RadioConfigGetCrc Failed");
            goto exit;
        }

        if (expectedCrc != radioConfigCrcTable[loopIndex].crc) {
            NXPLOG_APP_E("CRC Check Failed for Index %d", radioConfigCrcTable[loopIndex].index);
            status = UWBAPI_STATUS_FAILED;
            goto exit;
        }
    }
exit:
    return status;
}
