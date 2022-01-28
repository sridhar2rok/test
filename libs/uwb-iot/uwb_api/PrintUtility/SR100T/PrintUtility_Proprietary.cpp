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

EXTERNC void printStackCapabilities(const phUwbDevInfo_t *pdevInfo)
{
    if (pdevInfo != NULL) {
        PRINT_UTILITY_TRACES("UCI Version                        : %02X.%02X\n",
            (UINT8)(pdevInfo->uciVersion >> 8),
            (UINT8)pdevInfo->uciVersion);
        PRINT_UTILITY_TRACES("Device Name Length                 : %d\n", pdevInfo->devNameLen);
        PRINT_UTILITY_TRACES("Device Name                        :");
        for (UINT8 i = 0; i < pdevInfo->devNameLen; i++) {
            PRINT_UTILITY_TRACES("%c", pdevInfo->devName[i]);
        }
        PRINT_UTILITY_TRACES("\n");
        PRINT_UTILITY_TRACES("Firmware Version                   : %02X.%02X.%02x\n",
            pdevInfo->fwMajor,
            pdevInfo->fwMinor,
            pdevInfo->fwRc);
        PRINT_UTILITY_TRACES("NXP UCI Version                    : %02X.%02X.%02x\n",
            pdevInfo->nxpUciMajor,
            pdevInfo->nxpUciMinor,
            pdevInfo->nxpUciPatch);
        PRINT_UTILITY_TRACES("NXP Chip ID                        :");
        for (UINT8 i = 0; i < sizeof(pdevInfo->nxpChipId); i++) {
            PRINT_UTILITY_TRACES("%x", pdevInfo->nxpChipId[i]);
        }
        PRINT_UTILITY_TRACES("\n");
        PRINT_UTILITY_TRACES("Middleware Version                 : %02X.%02X\n", pdevInfo->mwMajor, pdevInfo->mwMinor);
    }
}

EXTERNC void printRcvData(const phRcvData_t *pRcvData)
{
    if (pRcvData != NULL) {
        PRINT_UTILITY_TRACES("pRcvData->session_id                : %" PRIu32 " \n", pRcvData->session_id);
        PRINT_UTILITY_TRACES("pRcvData->status                    : %" PRIu8 " \n", pRcvData->status);
        PRINT_UTILITY_TRACES("pRcvData->data_len                  : %" PRIu8 " \n", pRcvData->data_len);
        for (UINT8 i = 0; i < pRcvData->data_len; i++) {
            PRINT_UTILITY_TRACES("pRcvData->data                      : %" PRIu8 " \n", pRcvData->data[i]);
        }
    }
    else {
        PRINT_UTILITY_TRACES("pRcvData is NULL");
    }
}

EXTERNC void printDoBindStatus(const phSeDoBindStatus_t *pDoBindStatus)
{
    if (pDoBindStatus != NULL) {
        PRINT_UTILITY_TRACES("pDoBindStatus->status                : %" PRIu8 " \n", pDoBindStatus->status);
        PRINT_UTILITY_TRACES("pDoBindStatus->count_remaining       : %" PRIu8 " \n", pDoBindStatus->count_remaining);
        PRINT_UTILITY_TRACES("pDoBindStatus->binding_state         : %" PRIu8 " \n", pDoBindStatus->binding_state);
    }
    else {
        PRINT_UTILITY_TRACES("pDoBindStatus is NULL");
    }
}

EXTERNC void printGetBindingStatus(const phSeGetBindingStatus_t *pGetBindingStatus)
{
    if (pGetBindingStatus != NULL) {
        PRINT_UTILITY_TRACES("pGetBindingStatus->status                 : %" PRIu8 " \n", pGetBindingStatus->status);
        PRINT_UTILITY_TRACES(
            "pGetBindingStatus->se_binding_count       : %" PRIu8 " \n", pGetBindingStatus->se_binding_count);
        PRINT_UTILITY_TRACES(
            "pGetBindingStatus->uwbd_binding_count     : %" PRIu8 " \n", pGetBindingStatus->uwbd_binding_count);
    }
    else {
        PRINT_UTILITY_TRACES("pGetBindingStatus is NULL");
    }
}

EXTERNC void printDistance_Aoa(const phRangingData_t *pRangingData)
{
    if (pRangingData != NULL) {
        PRINT_UTILITY_TRACES("--------------Received Range Data--------------\n");
        PRINT_UTILITY_TRACES("pRangingData->sessionId                     : %" PRIu32 " \n", pRangingData->sessionId);
        for (UINT8 i = 0; i < pRangingData->no_of_measurements; i++) {
            PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu8 "].status          : %" PRIu8 " \n",
                i,
                pRangingData->range_meas[i].status);
            if (pRangingData->range_meas[i].status == UWBAPI_STATUS_OK) {
                PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu16 "].distance        : %" PRIu16 " \n",
                    i,
                    pRangingData->range_meas[i].distance);
                PRINT_UTILITY_TRACES("pRangingData->range_meas[%" PRIu16 "].aoaFirst             : %" PRIu16 " \n",
                    i,
                    pRangingData->range_meas[i].aoaFirst);
            }
        }
    }
    else {
        PRINT_UTILITY_TRACES("pRangingData is NULL");
    }
}

EXTERNC void printDebugParams(const phDebugParams_t *pDebugParams)
{
    if (pDebugParams != NULL) {
        PRINT_UTILITY_TRACES("pDebugParams->secureThread          : %" PRIu16 " \n", pDebugParams->secureThread);
        PRINT_UTILITY_TRACES("pDebugParams->secureIsrThread       : %" PRIu16 " \n", pDebugParams->secureIsrThread);
        PRINT_UTILITY_TRACES("pDebugParams->nonSecureIsrThread    : %" PRIu16 " \n", pDebugParams->nonSecureIsrThread);
        PRINT_UTILITY_TRACES("pDebugParams->shellThread           : %" PRIu16 " \n", pDebugParams->shellThread);
        PRINT_UTILITY_TRACES("pDebugParams->phyThread             : %" PRIu16 " \n", pDebugParams->phyThread);
        PRINT_UTILITY_TRACES("pDebugParams->rangingThread         : %" PRIu16 " \n", pDebugParams->rangingThread);
        PRINT_UTILITY_TRACES("pDebugParams->dataLoggerNtf         : %" PRIu8 " \n", pDebugParams->dataLoggerNtf);
        PRINT_UTILITY_TRACES("pDebugParams->cirLogNtf             : %" PRIu8 " \n", pDebugParams->cirLogNtf);
        PRINT_UTILITY_TRACES("pDebugParams->psduLogNtf            : %" PRIu8 " \n", pDebugParams->psduLogNtf);
        PRINT_UTILITY_TRACES("pDebugParams->rframeLogNtf          : %" PRIu8 " \n\n", pDebugParams->rframeLogNtf);
    }
    else {
        PRINT_UTILITY_TRACES("pDebugParams is NULL");
    }
}

EXTERNC void printTestLoopNtfData(const phTestLoopData_t *pTestLoopData)
{
    if (pTestLoopData != NULL) {
        PRINT_UTILITY_TRACES("Status                        : %" PRIu8 " \n", pTestLoopData->status);
        PRINT_UTILITY_TRACES("Loop Count                    : %" PRIu8 " \n", pTestLoopData->loop_cnt);
        PRINT_UTILITY_TRACES("Loop Pass Count               : %" PRIu8 " \n", pTestLoopData->loop_pass_count);
    }
    else {
        PRINT_UTILITY_TRACES("pTestLoopData is NULL");
    }
}
