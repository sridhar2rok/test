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

#include "PrintUtility.h"

EXTERNC void printGenericErrorStatus(const phGenericError_t *pGenericError)
{
    if (pGenericError != NULL) {
        PRINT_UTILITY_TRACES("Status                        : %" PRIu8 "\n", pGenericError->status);
    }
}

EXTERNC void printSessionStatusData(const phUwbSessionInfo_t *pSessionInfo)
{
    if (pSessionInfo != NULL) {
        PRINT_UTILITY_TRACES("pSessionInfo->session_id          : %" PRIu32 "\n", pSessionInfo->session_id);
        PRINT_UTILITY_TRACES("pSessionInfo->state               : %" PRIu8 "\n", pSessionInfo->state);
        PRINT_UTILITY_TRACES("pSessionInfo->reason_code         : %" PRIu8 "\n", pSessionInfo->reason_code);
    }
    else {
        PRINT_UTILITY_TRACES("pSessionInfo is NULL");
    }
}

EXTERNC void printUwbSessionData(const phUwbSessionsContext_t *pUwbSessionsContext)
{
    if (pUwbSessionsContext != NULL) {
        PRINT_UTILITY_TRACES("Status                        : %" PRIu32 " \n", pUwbSessionsContext->status);
        PRINT_UTILITY_TRACES("Session Counter               : %d \n", pUwbSessionsContext->sessioncnt);
        for (UINT8 i = 0; i < pUwbSessionsContext->sessioncnt; i++) {
            PRINT_UTILITY_TRACES(
                "Session %d ID             : %" PRIu32 " \n", i, pUwbSessionsContext->pUwbSessionData[i].session_id);
            PRINT_UTILITY_TRACES(
                "Session %d Type           : %" PRIu8 " \n", i, pUwbSessionsContext->pUwbSessionData[i].session_type);
            PRINT_UTILITY_TRACES(
                "Session %d State          : %" PRIu8 " \n", i, pUwbSessionsContext->pUwbSessionData[i].session_state);
        }
    }
    else {
        PRINT_UTILITY_TRACES("pUwbSessionsContext is NULL");
    }
}

EXTERNC void printMulticastListStatus(const phMulticastControleeListNtfContext_t *pControleeNtfContext)
{
    if (pControleeNtfContext != NULL) {
        PRINT_UTILITY_TRACES(
            "pControleeNtfContext->session_id          : %" PRIu32 " \n", pControleeNtfContext->session_id);
        PRINT_UTILITY_TRACES(
            "pControleeNtfContext->remaining_list      : %" PRIu8 " \n", pControleeNtfContext->remaining_list);
        PRINT_UTILITY_TRACES(
            "pControleeNtfContext->no_of_controlees    : %" PRIu8 " \n", pControleeNtfContext->no_of_controlees);

        for (UINT8 i = 0; i < pControleeNtfContext->no_of_controlees; i++) {
            PRINT_UTILITY_TRACES("pControleeNtfContext->subsession_id[%" PRIu8 "]   : %" PRIu32 " \n",
                i,
                pControleeNtfContext->subsession_id_list[i]);
            PRINT_UTILITY_TRACES("pControleeNtfContext->status[%" PRIu8 "]          : %" PRIu8 " \n",
                i,
                pControleeNtfContext->status_list[i]);
        }
    }
    else {
        PRINT_UTILITY_TRACES("phControleeNtfContext_t is NULL");
    }
}

EXTERNC void printRangingParams(const phRangingParams_t *pRangingParams)
{
    if (pRangingParams != NULL) {
        PRINT_UTILITY_TRACES("pRangingParams->deviceRole               : %" PRIu8 " \n", pRangingParams->deviceRole);
        PRINT_UTILITY_TRACES("pRangingParams->multiNodeMode            : %" PRIu8 " \n", pRangingParams->multiNodeMode);
        PRINT_UTILITY_TRACES("pRangingParams->macAddrMode              : %" PRIu8 " \n", pRangingParams->macAddrMode);
        PRINT_UTILITY_TRACES(
            "pRangingParams->noOfControlees           : %" PRIu8 " \n", pRangingParams->noOfControlees);

        UINT8 addrLen = MAC_SHORT_ADD_LEN;
        if (pRangingParams->macAddrMode != SHORT_MAC_ADDRESS) { // mac addr is of 2 or 8 bytes.
            addrLen = MAC_EXT_ADD_LEN;
        }
        PRINT_UTILITY_TRACES("pRangingParams->deviceMacAddr: 0x");
        for (UINT8 j = 0; j < addrLen; j++) {
            PRINT_UTILITY_TRACES("%x", pRangingParams->deviceMacAddr[j]);
        }
        PRINT_UTILITY_TRACES("\n");

        for (UINT32 i = 0; i < pRangingParams->noOfControlees; i++) {
            PRINT_UTILITY_TRACES("pRangingParams->dstMacAddr[%" PRIu32 "]: 0x", (i + 1));
            for (UINT32 j = i * addrLen; j < (i + 1) * addrLen; j++) {
                PRINT_UTILITY_TRACES("%x", pRangingParams->dstMacAddr[j]);
            }
            PRINT_UTILITY_TRACES("\n");
        }
        PRINT_UTILITY_TRACES("pRangingParams->deviceType               : %" PRIu8 " \n", pRangingParams->deviceType);
    }
    else {
        PRINT_UTILITY_TRACES("pRangingParams is NULL");
    }
}

EXTERNC void printRangingData(const phRangingData_t *pRangingData)
{
    if (pRangingData != NULL) {
        PRINT_UTILITY_TRACES("--------------Received Range Data--------------\n");
        PRINT_UTILITY_TRACES("pRangingData->seq_ctr                    : %" PRIu32 " \n", pRangingData->seq_ctr);
        PRINT_UTILITY_TRACES("pRangingData->sessionId                     : %" PRIu32 " \n", pRangingData->sessionId);
        PRINT_UTILITY_TRACES(
            "pRangingData->rcr_indication                : %" PRIu8 " \n", pRangingData->rcr_indication);
        PRINT_UTILITY_TRACES(
            "pRangingData->curr_range_interval           : %" PRIu32 " \n", pRangingData->curr_range_interval);
        PRINT_UTILITY_TRACES(
            "pRangingData->ranging_measure_type          : %" PRIu8 " \n", pRangingData->ranging_measure_type);
        PRINT_UTILITY_TRACES(
            "pRangingData->antenna_pair_sel              : %" PRIu8 " \n", pRangingData->antenna_pair_sel);
        PRINT_UTILITY_TRACES(
            "pRangingData->mac_addr_mode_indicator       : %" PRIu8 " \n", pRangingData->mac_addr_mode_indicator);
        PRINT_UTILITY_TRACES(
            "pRangingData->no_of_measurements            : %" PRIu8 " \n", pRangingData->no_of_measurements);

        if (pRangingData->ranging_measure_type == MEASUREMENT_TYPE_TWOWAY) {
            for (UINT8 i = 0; i < pRangingData->no_of_measurements; i++) {
                PRINT_UTILITY_TRACES("pRangingData->range_meas[%d].mac_addr        : 0x", i);
                for (UINT8 j = 0; j < (pRangingData->mac_addr_mode_indicator * 6) + 2;
                     j++) { // mac addr is of 2 or 8 bytes.
                    PRINT_UTILITY_TRACES("%x", pRangingData->range_meas[i].mac_addr[j]);
                }
                PRINT_UTILITY_TRACES("\n");

                PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu8 "].status          : %" PRIu8 " \n",
                    i,
                    pRangingData->range_meas[i].status);
                if (pRangingData->range_meas[i].status == UWBAPI_STATUS_OK) {
                    PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu8 "].nLos            : %" PRIu8 " \n",
                        i,
                        pRangingData->range_meas[i].nLos);
                    PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu16 "].distance        : %" PRIu16 " \n",
                        i,
                        pRangingData->range_meas[i].distance);
                    PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu16 "].aoaFirst        : %" PRIu16 " \n",
                        i,
                        pRangingData->range_meas[i].aoaFirst);
                    PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu16 "].aoaSecond       : %" PRIu16 " \n",
                        i,
                        pRangingData->range_meas[i].aoaSecond);
                    PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu16 "].pdoaFirst       : %" PRIu16 " \n",
                        i,
                        pRangingData->range_meas[i].pdoaFirst);
                    PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu16 "].pdoaSecond      : %" PRIu16 " \n",
                        i,
                        pRangingData->range_meas[i].pdoaSecond);
                    PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu16 "].pdoaFirstIndex  : %" PRIu16 " \n",
                        i,
                        pRangingData->range_meas[i].pdoaFirstIndex);
                    PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu16 "].pdoaSecondIndex      : %" PRIu16 " \n",
                        i,
                        pRangingData->range_meas[i].pdoaSecondIndex);
                    PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu16 "].aoaDestFirst : %" PRIu16 " \n",
                        i,
                        pRangingData->range_meas[i].aoaDestFirst);
                    PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu16 "].aoaDestSecond   : %" PRIu16 " \n",
                        i,
                        pRangingData->range_meas[i].aoaDestSecond);
                    PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu16 "].slot_index      : %" PRIu8 " \n",
                        i,
                        pRangingData->range_meas[i].slot_index);
                }
            }
        }
        else if (pRangingData->ranging_measure_type == MEASUREMENT_TYPE_ONEWAY) {
            PRINT_UTILITY_TRACES("pRangingData->range_meas[0].mac_addr        : 0x");
            for (UINT8 j = 0; j < (pRangingData->mac_addr_mode_indicator * 6) + 2;
                 j++) {                                                                // mac addr is of 2 or 8 bytes.
                PRINT_UTILITY_TRACES("%x", pRangingData->range_meas_tdoa.mac_addr[j]); // mac_addr_mode_indicator:0 or 1
            }
            PRINT_UTILITY_TRACES("\n");

            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.frame_type        : %" PRIu8 " \n",
                pRangingData->range_meas_tdoa.frame_type);
            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.nLos                : %" PRIu8 " \n",
                pRangingData->range_meas_tdoa.nLos);
            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.aoaFirst            : %" PRIu16 " \n",
                pRangingData->range_meas_tdoa.aoaFirst);
            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.aoaSecond           : %" PRIu16 " \n",
                pRangingData->range_meas_tdoa.aoaSecond);
            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.pdoaFirst           : %" PRIu16 " \n",
                pRangingData->range_meas_tdoa.pdoaFirst);
            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.pdoaSecond          : %" PRIu16 " \n",
                pRangingData->range_meas_tdoa.pdoaSecond);
            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.pdoaFirstIndex      : %" PRIu16 " \n",
                pRangingData->range_meas_tdoa.pdoaFirstIndex);
            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.pdoaSecondIndex     : %" PRIu16 " \n",
                pRangingData->range_meas_tdoa.pdoaSecondIndex);
            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.timestamp           : %" PRIu64 " \n",
                pRangingData->range_meas_tdoa.timestamp);
            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.blink_frame_number  : %" PRIu32 " \n",
                pRangingData->range_meas_tdoa.blink_frame_number);
            PRINT_UTILITY_TRACES(
                "pRangingData->range_meas_tdoa.rssiRX1             : %d\n", pRangingData->range_meas_tdoa.rssiRX1);
            PRINT_UTILITY_TRACES(
                "pRangingData->range_meas_tdoa.rssiRX2             : %d\n", pRangingData->range_meas_tdoa.rssiRX2);

            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.device_info_size    : %" PRIu8 " \n",
                pRangingData->range_meas_tdoa.device_info_size);

            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.device_info        : 0x");
            for (UINT8 j = 0; j < pRangingData->range_meas_tdoa.device_info_size; j++) {
                PRINT_UTILITY_TRACES("%x", pRangingData->range_meas_tdoa.device_info[j]);
            }
            PRINT_UTILITY_TRACES("\n");

            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.blink_payload_size     : %" PRIu8 " \n",
                pRangingData->range_meas_tdoa.blink_payload_size);

            PRINT_UTILITY_TRACES("pRangingData->range_meas_tdoa.blink_payload        : 0x");
            for (UINT8 j = 0; j < pRangingData->range_meas_tdoa.blink_payload_size; j++) {
                PRINT_UTILITY_TRACES("%x", pRangingData->range_meas_tdoa.blink_payload_data[j]);
            }
            PRINT_UTILITY_TRACES("\n");
        }
    }
    else {
        PRINT_UTILITY_TRACES("pRangingData is NULL");
    }
}